

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <uapi/linux/pkt_sched.h>
#include <log/log.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... <aggrq> aggrlen LEN on TX_ON_DURATION off TX_OFF_DURATION\n");
}

static int aggrq_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok = 0;
	struct tc_aggrq_qopt opt = {};

	while (argc > 0) {
		if (strcmp(*argv, "aggrlen") == 0) {
			NEXT_ARG();
			if (get_size(&opt.aggrlen, *argv)) {
				syslog(LOG_ERR, "%s: Illegal value for \"aggrlen\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			ok |= 0x1;
		} else if (strcmp(*argv, "on") == 0) {
			NEXT_ARG();
			if (get_time(&opt.txonduration, *argv)) {
				syslog(LOG_ERR, "%s: Illegal value for \"on\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			ok |= 0x2;
		} else if (strcmp(*argv, "off") == 0) {
			NEXT_ARG();
			if (get_time(&opt.txoffduration, *argv)) {
				syslog(LOG_ERR, "%s: Illegal value for \"off\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			ok |= 0x4;
		} else {
			syslog(LOG_ERR, "%s: unknown parameter \"%s\"\n", qu->id, *argv);
			explain();
			return -1;
		}
		argc--;
		argv++;
	}

	if (ok == 0x7) {
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt)); // 1024 is maxlen
		syslog(LOG_ERR, "aggrlen = %d, on = %d, off = %d\n", opt.aggrlen, opt.txonduration, opt.txoffduration);
	} else {
		explain();
		return -1;
	}
	return 0;
}

static void aggrq_print(struct tc_aggrq_qopt *qopt, FILE *file)
{
	fprintf(file, "aggrlen %d, on %d , off %d \n", qopt->aggrlen, qopt->txonduration, qopt->txoffduration);
	fprintf(file, "pi [10,50) : %u, [50, 100): %u, [100, 200): %u, [200, 300): %u, [300,+oo) : %u\n", \
		qopt->pktnum[0], qopt->pktnum[1], qopt->pktnum[2], qopt->pktnum[3], qopt->pktnum[4]); // packt interval
}

static int aggrq_print_opt(struct qdisc_util *qu, FILE *file, struct rtattr *opt)
{
	struct tc_aggrq_qopt *qopt;

	if (opt == NULL)
		return 0;
	if (RTA_PAYLOAD(opt) < sizeof(*qopt))
		return -1;
	qopt = RTA_DATA(opt);
	if (strcmp(qu->id, "aggrq") == 0)
		aggrq_print(qopt, file);
	return 0;
}

struct qdisc_util aggrq_qdisc_util = {
	.id = "aggrq",
	.parse_qopt = aggrq_parse_opt,
	.print_qopt = aggrq_print_opt,
};