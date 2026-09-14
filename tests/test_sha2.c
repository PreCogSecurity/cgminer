/*
 * Unit tests for the SHA-256 implementation in sha2.c.
 *
 * Test vectors are the well-known FIPS 180-2 / NIST examples:
 *   - empty string
 *   - "abc"
 *   - the two-block message "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *   - one million 'a' characters
 *
 * Copyright 2011-2013 Con Kolivas
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.  See COPYING for more details.
 */

#include "config.h"
#include "sha2.h"
#include "test.h"

static void test_sha256_empty(void)
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	unsigned char expected[SHA256_DIGEST_SIZE] = {
		0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
		0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
		0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
		0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
	};

	sha256((const unsigned char *)"", 0, digest);
	CHECK_MEM_EQ(digest, expected, SHA256_DIGEST_SIZE);
}

static void test_sha256_abc(void)
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	unsigned char expected[SHA256_DIGEST_SIZE] = {
		0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
		0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
		0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
		0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
	};

	sha256((const unsigned char *)"abc", 3, digest);
	CHECK_MEM_EQ(digest, expected, SHA256_DIGEST_SIZE);
}

static void test_sha256_two_block(void)
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	unsigned char expected[SHA256_DIGEST_SIZE] = {
		0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
		0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
		0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
		0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
	};
	const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

	sha256((const unsigned char *)msg, strlen(msg), digest);
	CHECK_MEM_EQ(digest, expected, SHA256_DIGEST_SIZE);
}

static void test_sha256_million_a(void)
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	unsigned char expected[SHA256_DIGEST_SIZE] = {
		0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92,
		0x81, 0xa1, 0xc7, 0xe2, 0x84, 0xd7, 0x3e, 0x67,
		0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97, 0x20, 0x0e,
		0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0
	};
	unsigned char *msg = malloc(1000000);
	int i;

	CHECK(msg != NULL);
	if (!msg)
		return;
	for (i = 0; i < 1000000; i++)
		msg[i] = 'a';

	sha256(msg, 1000000, digest);
	free(msg);
	CHECK_MEM_EQ(digest, expected, SHA256_DIGEST_SIZE);
}

static void test_sha256_incremental(void)
{
	/* Feed "abc" one byte at a time through init/update/final and compare
	 * with the one-shot result. */
	sha256_ctx ctx;
	unsigned char digest[SHA256_DIGEST_SIZE];
	unsigned char one_shot[SHA256_DIGEST_SIZE];
	const unsigned char *msg = (const unsigned char *)"abc";
	int i;

	sha256(msg, 3, one_shot);

	sha256_init(&ctx);
	for (i = 0; i < 3; i++)
		sha256_update(&ctx, msg + i, 1);
	sha256_final(&ctx, digest);

	CHECK_MEM_EQ(digest, one_shot, SHA256_DIGEST_SIZE);
}

int main(void)
{
	test_sha256_empty();
	test_sha256_abc();
	test_sha256_two_block();
	test_sha256_million_a();
	test_sha256_incremental();

	TEST_RESULT("test_sha2");
}