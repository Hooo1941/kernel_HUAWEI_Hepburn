// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>



#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & (CONFIG_SYS_POST_MEMORY | CONFIG_SYS_POST_MEM_REGIONS)

DECLARE_GLOBAL_DATA_PTR;

/*
 * Define INJECT_*_ERRORS for testing error detection in the presence of
 * _good_ hardware.
 */
#undef  INJECT_DATA_ERRORS
#undef  INJECT_ADDRESS_ERRORS

#ifdef INJECT_DATA_ERRORS
#warning "Injecting data line errors for testing purposes"
#endif

#ifdef INJECT_ADDRESS_ERRORS
#warning "Injecting address line errors for testing purposes"
#endif


/*
 * This function performs a double word move from the data at
 * the source pointer to the location at the destination pointer.
 * This is helpful for testing memory on processors which have a 64 bit
 * wide data bus.
 *
 * On those PowerPC with FPU, use assembly and a floating point move:
 * this does a 64 bit move.
 *
 * For other processors, let the compiler generate the best code it can.
 */
static void move64(const unsigned long long *src, unsigned long long *dest)
{
	*dest = *src;
}

/*
 * This is 64 bit wide test patterns.  Note that they reside in ROM
 * (which presumably works) and the tests write them to RAM which may
 * not work.
 *
 * The "otherpattern" is written to drive the data bus to values other
 * than the test pattern.  This is for detecting floating bus lines.
 *
 */
const static unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,
	0xccccccccccccccccULL,
	0xf0f0f0f0f0f0f0f0ULL,
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,
	0x00000000ffffffffULL,
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};
const unsigned long long otherpattern = 0x0123456789abcdefULL;


static int memory_post_dataline(unsigned long long * pmem)
{
	unsigned long long temp64 = 0;
	int num_patterns = ARRAY_SIZE(pattern);
	int i;
	unsigned int hi, lo, pathi, patlo;
	int ret = 0;

	for ( i = 0; i < num_patterns; i++) {
		move64(&(pattern[i]), pmem++);
		/*
		 * Put a different pattern on the data lines: otherwise they
		 * may float long enough to read back what we wrote.
		 */
		move64(&otherpattern, pmem--);
		move64(pmem, &temp64);

#ifdef INJECT_DATA_ERRORS
		temp64 ^= 0x00008000;
#endif

		if (temp64 != pattern[i]){
			pathi = (pattern[i]>>32) & 0xffffffff;
			patlo = pattern[i] & 0xffffffff;

			hi = (temp64>>32) & 0xffffffff;
			lo = temp64 & 0xffffffff;

			post_log("Memory (data line) error at %08x, "
				  "wrote %08x%08x, read %08x%08x !\n",
					  pmem, pathi, patlo, hi, lo);
			ret = -1;
		}
	}
	return ret;
}

static int memory_post_addrline(ulong *testaddr, ulong *base, ulong size)
{
	ulong *target;
	ulong *end;
	ulong readback;
	ulong xor;
	int   ret = 0;

	end = (ulong *)((ulong)base + size);	/* pointer arith! */
	xor = 0;
	for(xor = sizeof(ulong); xor > 0; xor <<= 1) {
		target = (ulong *)((ulong)testaddr ^ xor);
		if((target >= base) && (target < end)) {
			*testaddr = ~*target;
			readback  = *target;

#ifdef INJECT_ADDRESS_ERRORS
			if(xor == 0x00008000) {
				readback = *testaddr;
			}
#endif
			if(readback == *testaddr) {
				post_log("Memory (address line) error at %08x<->%08x, "
					"XOR value %08x !\n",
					testaddr, target, xor);
				ret = -1;
			}
		}
	}
	return ret;
}

static int memory_post_test1(unsigned long start,
			      unsigned long size,
			      unsigned long val)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = val;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != val) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, val, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test2(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = 1 << (i % 32);
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != (1 << (i % 32))) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, 1 << (i % 32), readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test3(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test4(unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = ~i;
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	for (i = 0; i < size / sizeof (ulong) && !ret; i++) {
		readback = mem[i];
		if (readback != ~i) {
			post_log("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, ~i, readback);

			ret = -1;
			break;
		}
		if (i % 1024 == 0)
			WATCHDOG_RESET();
	}

	return ret;
}

static int memory_post_test_lines(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_dataline((unsigned long long *)start);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)start, (ulong *)start,
				size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_addrline((ulong *)(start+size-8),
				(ulong *)start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_patterns(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test1(start, size, 0x00000000);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xffffffff);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0x55555555);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test1(start, size, 0xaaaaaaaa);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test2(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test3(start, size);
	WATCHDOG_RESET();
	if (!ret)
		ret = memory_post_test4(start, size);
	WATCHDOG_RESET();

	return ret;
}

static int memory_post_test_regions(unsigned long start, unsigned long size)
{
	unsigned long i;
	int ret = 0;

	for (i = 0; i < (size >> 20) && (!ret); i++) {
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20),
				0x800);
		if (!ret)
			ret = memory_post_test_patterns(start + (i << 20) +
				0xff800, 0x800);
	}

	return ret;
}

static int memory_post_tests(unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_test_lines(start, size);
	if (!ret)
		ret = memory_post_test_patterns(start, size);

	return ret;
}

/*
 * !! this is only valid, if you have contiguous memory banks !!
 */
__attribute__((weak))
int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	bd_t *bd = gd->bd;

	*vstart = CONFIG_SYS_SDRAM_BASE;
	*size = (gd->ram_size >= 256 << 20 ?
			256 << 20 : gd->ram_size) - (1 << 20);

	/* Limit area to be tested with the board info struct */
	if ((*vstart) + (*size) > (ulong)bd)
		*size = (ulong)bd - *vstart;

	return 0;
}

__attribute__((weak))
int arch_memory_test_advance(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 1;
}

__attribute__((weak))
int arch_memory_test_cleanup(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	return 0;
}

__attribute__((weak))
void arch_memory_failure_handle(void)
{
	return;
}

int memory_regions_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	ret = memory_post_test_lines(vstart, memsize);
	if (!ret)
		ret = memory_post_test_regions(vstart, memsize);

	return ret;
}

int memory_post_test(int flags)
{
	int ret = 0;
	phys_addr_t phys_offset = 0;
	u32 memsize, vstart;

	arch_memory_test_prepare(&vstart, &memsize, &phys_offset);

	do {
		if (flags & POST_SLOWTEST) {
			ret = memory_post_tests(vstart, memsize);
		} else {			/* POST_NORMAL */
			ret = memory_post_test_regions(vstart, memsize);
		}
	} while (!ret &&
		!arch_memory_test_advance(&vstart, &memsize, &phys_offset));

	arch_memory_test_cleanup(&vstart, &memsize, &phys_offset);
	if (ret)
		arch_memory_failure_handle();

	return ret;
}

#endif /* CONFIG_POST&(CONFIG_SYS_POST_MEMORY|CONFIG_SYS_POST_MEM_REGIONS) */
