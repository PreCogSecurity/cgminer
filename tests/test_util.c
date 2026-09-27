/*
 * Unit tests for the pure utility functions in util.c.
 *
 * Only self-contained helpers are exercised here (byte reversal, hex
 * encoding, timeval/timespec arithmetic and URL parsing); functions that
 * touch sockets, pools or the network are out of scope for this unit test.
 *
 * The test binary links util.c with -ffunction-sections and --gc-sections so
 * that unreferenced functions (and their external dependencies) are dropped
 * at link time.
 *
 * Copyright 2011-2013 Con Kolivas
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 */

#include "config.h"
#include "util.h"
#include "miner.h"
#include "test.h"

/* bin_reverse and __bin2hex are defined in util.c but not declared in util.h */
void bin_reverse(unsigned char *in, unsigned char *out, int length);
void __bin2hex(char *s, const unsigned char *p, size_t len);

static void test_bin_reverse(void)
{
	unsigned char in[4] = { 0x01, 0x02, 0x03, 0x04 };
	unsigned char out[4] = { 0, 0, 0, 0 };
	unsigned char expected[4] = { 0x04, 0x03, 0x02, 0x01 };

	bin_reverse(in, out, 4);
	CHECK_MEM_EQ(out, expected, 4);

	/* Odd length */
	bin_reverse(in, out, 3);
	CHECK(out[0] == 0x03 && out[1] == 0x02 && out[2] == 0x01);
}

static void test_bin2hex(void)
{
	unsigned char in[3] = { 0xde, 0xad, 0xbe };
	char out[7];

	__bin2hex(out, in, 3);
	out[6] = '\0';
	CHECK_STR_EQ(out, "deadbe");
}

static void test_timeval_helpers(void)
{
	struct timeval a = { 10, 500000 };
	struct timeval b = { 3, 750000 };
	struct timeval res;
	struct timespec spec;

	/* subtime: a - b stored in b */
	subtime(&a, &b);
	CHECK(b.tv_sec == 6 && b.tv_usec == 750000);

	/* addtime: a + b stored in b */
	addtime(&a, &b);
	CHECK(b.tv_sec == 17 && b.tv_usec == 250000);

	/* time_more / time_less */
	CHECK(time_more(&a, &b));
	CHECK(!time_less(&a, &b));
	CHECK(time_less(&b, &a));

	/* copy_time */
	copy_time(&res, &a);
	CHECK(res.tv_sec == a.tv_sec && res.tv_usec == a.tv_usec);
}

static void test_timespec_helpers(void)
{
	struct timespec spec = { 5, 123456789 };
	struct timeval val;

	timespec_to_val(&val, &spec);
	CHECK(val.tv_sec == 5 && val.tv_usec == 123456);

	timeval_to_spec(&spec, &val);
	CHECK(spec.tv_sec == 5 && spec.tv_nsec == 123456000);

	us_to_timeval(&val, 1234567);
	CHECK(val.tv_sec == 1 && val.tv_usec == 234567);

	us_to_timespec(&spec, 1234567);
	CHECK(spec.tv_sec == 1 && spec.tv_nsec == 234567000);

	ms_to_timespec(&spec, 1500);
	CHECK(spec.tv_sec == 1 && spec.tv_nsec == 500000000);

	ms_to_timeval(&val, 1500);
	CHECK(val.tv_sec == 1 && val.tv_usec == 500000);

	/* Negative values must round-trip through lldiv correctly */
	us_to_timeval(&val, -1);
	CHECK(val.tv_sec == -1 && val.tv_usec == 999999);
}

static void test_timespec_arith(void)
{
	struct timespec a = { 1, 900000000 };
	struct timespec b = { 0, 200000000 };
	struct timespec res;

	/* timeraddspec: a += b with nsec overflow */
	timeraddspec(&a, &b);
	CHECK(a.tv_sec == 2 && a.tv_nsec == 100000000);

	/* cgtimer_sub: a - b stored in res */
	a.tv_sec = 2;
	a.tv_nsec = 100000000;
	cgtimer_sub(&a, &b, &res);
	CHECK(res.tv_sec == 1 && res.tv_nsec == 900000000);

	/* cgtimer_to_ms */
	a.tv_sec = 1;
	a.tv_nsec = 500000000;
	CHECK(cgtimer_to_ms(&a) == 1500);
}

static void test_tdiff(void)
{
	struct timeval start = { 100, 0 };
	struct timeval end = { 105, 500000 };

	CHECK(us_tdiff(&end, &start) == 5500000);
	CHECK(ms_tdiff(&end, &start) == 5500);
	CHECK(tdiff(&end, &start) == 5.5);

	/* Sanity caps: >60s for us_tdiff, >1h for ms_tdiff */
	end.tv_sec = 200;
	CHECK(us_tdiff(&end, &start) == 60000000);
	end.tv_sec = 4000;
	CHECK(ms_tdiff(&end, &start) == 3600000);
}

static void test_extract_sockaddr(void)
{
	char url1[] = "http://pool.example.com:8337";
	char url2[] = "stratum+tcp://mining.example.org:3333";
	char url3[] = "http://pool.example.com";
	char url4[] = "[::1]:8080";
	char url5[] = "http://:8337";
	char *sockaddr_url = NULL;
	char *sockaddr_port = NULL;

	CHECK(extract_sockaddr(url1, &sockaddr_url, &sockaddr_port));
	CHECK_STR_EQ(sockaddr_url, "pool.example.com");
	CHECK_STR_EQ(sockaddr_port, "8337");
	free(sockaddr_url);
	free(sockaddr_port);

	CHECK(extract_sockaddr(url2, &sockaddr_url, &sockaddr_port));
	CHECK_STR_EQ(sockaddr_url, "mining.example.org");
	CHECK_STR_EQ(sockaddr_port, "3333");
	free(sockaddr_url);
	free(sockaddr_port);

	/* No port -> defaults to 80 */
	CHECK(extract_sockaddr(url3, &sockaddr_url, &sockaddr_port));
	CHECK_STR_EQ(sockaddr_url, "pool.example.com");
	CHECK_STR_EQ(sockaddr_port, "80");
	free(sockaddr_url);
	free(sockaddr_port);

	/* IPv6 literal */
	CHECK(extract_sockaddr(url4, &sockaddr_url, &sockaddr_port));
	CHECK_STR_EQ(sockaddr_url, "[::1]");
	CHECK_STR_EQ(sockaddr_port, "8080");
	free(sockaddr_url);
	free(sockaddr_port);

	/* Empty host is invalid */
	CHECK(!extract_sockaddr(url5, &sockaddr_url, &sockaddr_port));
}

int main(void)
{
	test_bin_reverse();
	test_bin2hex();
	test_timeval_helpers();
	test_timespec_helpers();
	test_timespec_arith();
	test_tdiff();
	test_extract_sockaddr();

	TEST_RESULT("test_util");
}