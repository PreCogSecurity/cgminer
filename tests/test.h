/*
 * Minimal self-contained test harness for the cgminer unit tests.
 *
 * Deliberately dependency-free: no external test framework is required so
 * that `make check` works on a fresh clone with only the build dependencies
 * installed (see DEPENDENCIES.md).
 *
 * Each test binary includes this header exactly once and finishes with a
 * TEST_RESULT(name) call which prints a summary and returns a non-zero exit
 * status if any check failed (which is what makes `make check` fail).
 */

#ifndef CGMINER_TEST_H
#define CGMINER_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond) do { \
	tests_run++; \
	if (!(cond)) { \
		tests_failed++; \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
	} \
} while (0)

#define CHECK_STR_EQ(a, b) do { \
	const char *test_a = (a); \
	const char *test_b = (b); \
	tests_run++; \
	if (strcmp(test_a, test_b) != 0) { \
		tests_failed++; \
		fprintf(stderr, "FAIL %s:%d: \"%s\" != \"%s\"\n", \
			__FILE__, __LINE__, test_a, test_b); \
	} \
} while (0)

#define CHECK_MEM_EQ(a, b, len) do { \
	tests_run++; \
	if (memcmp((a), (b), (len)) != 0) { \
		tests_failed++; \
		fprintf(stderr, "FAIL %s:%d: memory mismatch (%u bytes)\n", \
			__FILE__, __LINE__, (unsigned)(len)); \
	} \
} while (0)

#define TEST_RESULT(name) do { \
	if (tests_failed) { \
		fprintf(stderr, "%s: %d/%d checks FAILED\n", \
			(name), tests_failed, tests_run); \
		return 1; \
	} \
	printf("%s: all %d checks passed\n", (name), tests_run); \
	return 0; \
} while (0)

#endif /* CGMINER_TEST_H */