#include "perf_buffered_log_sender.h"
#include "securec.h"

#include <linux/time64.h>
#include <linux/timekeeping.h>
#include <linux/net.h>
#include <linux/un.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#define PERFD_SOCKET_NAME "PERF_BUFFERED_LOG"
#define PERF_BUFFERED_LOG_TYPE "perf_driver_auth"

static DEFINE_MUTEX(sock_lock);
static struct socket *perfd_socket = NULL;
static int do_send_perf_buffered_log(const struct perf_buffered_log_entry *entry);
static int send_perf_buffered_log_init(const struct perf_buffered_log_entry *entry);
int (*send_perf_buffered_log)(const struct perf_buffered_log_entry *entry) = send_perf_buffered_log_init;

void perf_sock_lock_init(void)
{
	mutex_init(&sock_lock);
}

static unsigned long long perf_get_time()
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	struct timespec64 now;
	ktime_get_coarse_real_ts64(&now);
#else
	struct timespec now = current_kernel_time();
#endif
	return (unsigned long long)(now.tv_sec * S_TO_MS + now.tv_nsec / NS_TO_MS);
}

static bool perf_log_entry_is_valid_length(int len)
{
	if ((len > 0) && (len <= MAX_PERF_LOG_LENGTH))
		return true;
	return false;
}

static int write_to_perf_daemon(const struct perf_buffered_log_entry *entry)
{
	size_t payload_size = 0;
	struct msghdr msg;
	struct kvec vec[1];

	if (perfd_socket == NULL || entry == NULL) {
		pr_err("PerfD %s: invalid parameter\n", __func__);
		return -EINVAL;
	}

	if (!perf_log_entry_is_valid_length(entry->length)) {
		pr_err("PerfD %s: invalid log entry length: %d, magic: %d\n", __func__,
			entry->length, entry->magic);
		return -EINVAL;
	}

	vec[0].iov_base = (void *) entry;
	vec[0].iov_len = (size_t) entry->length;
	payload_size += vec[0].iov_len;
	memset_s(&msg, sizeof(msg), 0, sizeof(msg));

	return kernel_sendmsg(perfd_socket, &msg, vec, 1, payload_size);
}

static int make_sockaddr_un(const char *name, struct sockaddr_un *un, int *alen)
{
	size_t name_len;
	int ret;

	memset_s(un, sizeof(*un), 0, sizeof(*un));
	name_len = strlen(name);
	if ((name_len + 1) > sizeof(un->sun_path)) {
		pr_err("PerfD %s: invalid name length\n", __func__);
		return -EINVAL;
	}
	// The path in this case is not supposed to be '\0'-terminated
	un->sun_path[0] = 0;
	ret = memcpy_s(un->sun_path + 1, name_len, name, name_len);
	if (ret != EOK) {
		pr_err("PerfD %s: memcpy_s failed\n", __func__);
		return ret;
	}
	un->sun_family = AF_LOCAL;
	*alen = name_len + offsetof(struct sockaddr_un, sun_path) + 1;

	return 0;
}

/*
 * Try to connect perfd local socket server and authenticate data type
 * Connect success if both operations were done
 */
static int connect_perfd_server(int magic)
{
	int ret;
	int alen = 0;
	struct sockaddr_un un;

	if (perfd_socket != NULL)
		return 0;

	ret = sock_create(AF_LOCAL, SOCK_STREAM, 0, &perfd_socket);
	if (ret < 0 || perfd_socket == NULL) {
		pr_err("PerfD %s: failed to create socket\n", __func__);
		goto error;
	}
	ret = make_sockaddr_un(PERFD_SOCKET_NAME, &un, &alen);
	if (ret < 0) {
		pr_err("PerfD %s: failed to make socket address\n", __func__);
		goto error;
	}
	ret = kernel_connect(perfd_socket, (struct sockaddr *)&un, alen, O_CLOEXEC);
	if (ret < 0) {
		pr_err("PerfD %s: failed to connect socket\n", __func__);
		goto error;
	}
	pr_info("PerfD %s: Succeed to connect perf local socket server\n", __func__);

	return 0;

error:
	if (perfd_socket != NULL) {
		sock_release(perfd_socket);
		perfd_socket = NULL;
	}

	return -ECONNREFUSED;
}

// Initilize socket to connect perfd local socket server
static int send_perf_buffered_log_init(const struct perf_buffered_log_entry *entry)
{
	int ret;

	if (entry == NULL) {
		pr_err("PerfD %s: invalid parameter\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sock_lock);
	if (send_perf_buffered_log == send_perf_buffered_log_init) {
		// initialize perf socket for the first time
		ret = connect_perfd_server(entry->magic);
		if (ret < 0) {
			// Retry to connect to perf local socket server
			ret = connect_perfd_server(entry->magic);
			if (ret < 0) {
				mutex_unlock(&sock_lock);
				return -ECONNREFUSED;
			}
		}
		send_perf_buffered_log = do_send_perf_buffered_log;
	}
	mutex_unlock(&sock_lock);

	return do_send_perf_buffered_log(entry);
}

static int do_send_perf_buffered_log(const struct perf_buffered_log_entry *entry)
{
	int ret;

	if (entry == NULL) {
		pr_err("PerfD %s: invalid parameter\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&sock_lock);
	ret = write_to_perf_daemon(entry);
	if (ret < 0) {
		if (perfd_socket != NULL) {
			sock_release(perfd_socket);
			perfd_socket = NULL;
		} else {
			pr_err("PerfD %s: unexpectedly! perfd_socket is null!\n", __func__);
		}
		pr_err("PerfD %s: failed to write to daemon: %d\n", __func__, ret);
		send_perf_buffered_log = send_perf_buffered_log_init;
	}
	mutex_unlock(&sock_lock);

	return ret;
}

int send_perf_buffered_log_entry(const struct perf_buffered_log_entry *entry)
{
	return send_perf_buffered_log(entry);
}

// Warnning: Using vmalloc to allocate memory, so this fuction might sleep
struct perf_buffered_log_entry *perf_create_log_entry(unsigned int size, int magic, int count)
{
	struct perf_buffered_log_entry *entry = NULL;

	might_sleep();

	if (size <= 0 || size > MAX_PERF_LOG_LENGTH) {
		pr_err("PerfD %s: Invalid log size: %lld, magic: %d\n", __func__, size, magic);
		return NULL;
	}
	entry = vmalloc(size);
	if (entry == NULL) {
		pr_err("PerfD %s: Failed to allocate memory, magic: %d\n", __func__, magic);
		return NULL;
	}
	memset_s(entry, size, 0, size);
	entry->length = size;
	entry->magic = magic;
	entry->count = count;
	entry->time = perf_get_time();

	return entry;
}

void free_perf_buffered_log_entry(struct perf_buffered_log_entry *entry)
{
	if (entry)
		vfree(entry);
}

void perf_buffered_log_release(void)
{
	mutex_lock(&sock_lock);
	if (perfd_socket != NULL)
		sock_release(perfd_socket);
	mutex_unlock(&sock_lock);
}

unsigned int cal_log_entry_len(size_t head, size_t info, int count)
{
	return (unsigned int)head + count * (unsigned int)info;
}

