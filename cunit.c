/*
 * Author: Carter Williams
 * Email: carterww@hotmail.com
 * Date: 2024-10-06
 * License: MIT
 */

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "cassert.h"
#include "cunit.h"

#define __CUNIT_TEST_OUT_BLOCK_SIZE 512

#define RESET_COLOR_OUT "\x1b[0m"

#define BOLD_OUT "\x1b[1m"

#define RED_BOLD_OUT "\x1b[1;31m"
#define GREEN_BOLD_OUT "\x1b[1;32m"
#define YELLOW_BOLD_OUT "\x1b[1;33m"

#define RED_OUT "\x1b[0;31m"
#define GREEN_OUT "\x1b[0;32m"

struct __cunit_test_out {
	char *buff;
	size_t bytes;
	struct __cunit_test_out *next;
};

struct __cunit_test_result {
	const char *name;
	const struct __cunit_test_out *test_stderr;
	int test_status;
	struct __cunit_test_result *next;
};

struct __cunit_suite_result {
	const char *name;
	size_t tests_total;
	size_t tests_passed;
	struct __cunit_test_result *results;
	struct __cunit_suite_result *next;
};

static struct cunit_suite *suite_first = NULL;
static struct cunit_suite *suite_curr = NULL;

static void __nullable_func_call(void (*func)(void))
{
	if (NULL == func) {
		return;
	}
	func();
}

static struct __cunit_suite_result *
__run_suite(const struct cunit_suite *suite);
static struct __cunit_test_result *__run_test(const struct cunit_suite *suite,
					      const struct cunit_test *test);
static void __run_test_child(int fds[2], const struct cunit_suite *suite,
			     const struct cunit_test *test);
static struct __cunit_test_result *
__run_test_parent(pid_t child_pid, int fds[2], const struct cunit_test *test);
static struct __cunit_test_out *__get_child_stderr(int fd);

static void __print_suite_results(struct __cunit_suite_result *result);
static void __print_result_tests(struct __cunit_suite_result *result,
				 size_t suite_num);
static int __did_test_fail(int status);
static char *__get_failure_reason(int status);
static const char *__strsignal(int sig_num);

int main(void)
{
	// TODO: Free malloc'd stuff
	// TODO: Make suites run concurrently
	// TODO: Add command line args for
	// 1. Interactive mode: After tests run, keep results so user can
	//     print suite results again and/or print single test results.
	// 2. Run subset of suites and subset of tests.
	// 3. Print formatted results (json or something easily parsable).
	// 4. Maximum number of suites that can run concurrently.
	struct __cunit_suite_result *suite_result_first = NULL;
	struct cunit_suite *suite = suite_first;
	while (NULL != suite) {
		struct __cunit_suite_result *res = __run_suite(suite);
		res->next = suite_result_first;
		suite_result_first = res;
		suite = suite->next;
	}
	__print_suite_results(suite_result_first);
	return 0;
}

static void __print_suite_results(struct __cunit_suite_result *result)
{
	size_t suite_num = 1;
	while (NULL != result) {
		char *color = RED_BOLD_OUT;
		if (result->tests_passed == result->tests_total) {
			color = GREEN_BOLD_OUT;
		}
		// color X) suite_name (passed/total) color_reset
		printf("\t%s%ld) %s (%lu/%lu)%s\n", color, suite_num,
		       result->name, result->tests_passed, result->tests_total,
		       RESET_COLOR_OUT);
		__print_result_tests(result, suite_num);
		++suite_num;
		result = result->next;
	}
}

static void __print_result_tests(struct __cunit_suite_result *result,
				 size_t suite_num)
{
	size_t test_num = 1;
	struct __cunit_test_result *single_result = result->results;
	while (NULL != single_result) {
		int failed = __did_test_fail(single_result->test_status);
		char *failure_reason = NULL;
		char *color;
		char *result_string;
		if (failed) {
			color = RED_OUT;
			result_string = "FAILED";
			failure_reason = __get_failure_reason(
				single_result->test_status);
		} else {
			color = GREEN_OUT;
			result_string = "PASSED";
		}
		// "color X.X) test_name -> result color_reset
		printf("\t\t%s%ld.%ld) %s -> %s%s\n", color, suite_num,
		       test_num, single_result->name, result_string,
		       RESET_COLOR_OUT);
		if (failed && NULL != failure_reason) {
			printf("\t\t\t%s\n", failure_reason);
			free(failure_reason);
		}
		if (NULL != single_result->test_stderr) {
			printf("\t\t\t%sSTDERR:%s\n", YELLOW_BOLD_OUT,
			       RESET_COLOR_OUT);
			const struct __cunit_test_out *out =
				single_result->test_stderr;
			while (NULL != out) {
				printf("\t\t\t%s", out->buff);
				out = out->next;
			}
			printf("\n");
		}
		++test_num;
		single_result = single_result->next;
	}
}

void __cunit_add_suite(struct cunit_suite *suite)
{
	if (NULL == suite_first) {
		suite_first = suite;
	} else {
		suite_curr->next = suite;
	}
	suite_curr = suite;
}

static struct __cunit_suite_result *__run_suite(const struct cunit_suite *suite)
{
	c_assert_fail(NULL != suite);
	struct __cunit_suite_result *res = malloc(sizeof(*res));
	c_assert_fail(NULL != res); // If malloc returns NULL, we'll just fail
	memset(res, 0, sizeof(*res));
	res->name = suite->name;

	struct cunit_test *curr = suite->tests;
	while (NULL != curr) {
		++res->tests_total;
		struct __cunit_test_result *test_res = __run_test(suite, curr);
		c_assert_fail(NULL != test_res);
		test_res->next = res->results;
		res->results = test_res;
		if (!__did_test_fail(test_res->test_status)) {
			++res->tests_passed;
		}
		curr = curr->next;
	}

	return res;
}

static struct __cunit_test_result *__run_test(const struct cunit_suite *suite,
					      const struct cunit_test *test)
{
	c_assert_fail(NULL != suite);
	c_assert_fail(NULL != test);
	pid_t pid;
	int fd[2];

	int pipe_res = pipe(fd);
	pid = fork();

	c_assert_fail(pipe_res == 0);
	c_assert_fail(pid >= 0);
	if (0 == pid) {
		__run_test_child(fd, suite, test);
		// This should be unreachable
		c_assert_fail(1 == 0);
		return NULL;
	} else {
		return __run_test_parent(pid, fd, test);
	}
}

static void __run_test_child(int fds[2], const struct cunit_suite *suite,
			     const struct cunit_test *test)
{
	close(fds[0]);
	dup2(fds[1], STDERR_FILENO);
	__nullable_func_call(suite->setup);
	test->test();
	__nullable_func_call(suite->teardown);
	close(fds[1]);
	exit(0);
}

static struct __cunit_test_result *
__run_test_parent(pid_t child_pid, int fds[2], const struct cunit_test *test)
{
	close(fds[1]);
	int status = -1;
	// TODO: add timeout mechanism to avoid getting hung
	waitpid(child_pid, &status, 0);
	struct __cunit_test_result *res = malloc(sizeof(*res));
	c_assert_fail(NULL != res);
	res->name = test->name;
	res->test_status = status;
	res->next = NULL;
	res->test_stderr = __get_child_stderr(fds[0]);

	close(fds[0]);
	return res;
}

static struct __cunit_test_out *__get_child_stderr(int fd)
{
	struct __cunit_test_out *out = NULL;
	struct __cunit_test_out *curr = NULL;
	do {
		char *buff = malloc(__CUNIT_TEST_OUT_BLOCK_SIZE);
		if (NULL == buff) {
			return out;
		}
		ssize_t bytes = read(fd, buff, __CUNIT_TEST_OUT_BLOCK_SIZE);
		if (bytes <= 0) {
			free(buff);
			return out;
		}
		struct __cunit_test_out *new = malloc(sizeof(*new));
		if (NULL == new) {
			free(buff);
			return out;
		}
		new->buff = buff;
		new->bytes = bytes;
		new->next = NULL;
		if (curr == NULL) {
			out = new;
			curr = new;
		} else {
			curr->next = new;
			curr = new;
		}
	} while (1);
}

static int __did_test_fail(int status)
{
	return !WIFEXITED(status) || WEXITSTATUS(status) != 0;
}

static char *__get_failure_reason(int status)
{
	static const int rlen = 48;
	char *reason = malloc(rlen);
	if (NULL == reason) {
		return NULL;
	}
	if (WIFEXITED(status)) {
		int exit_code = WEXITSTATUS(status);
		snprintf(reason, rlen, "%sTest Exitted With Status %d%s",
			 BOLD_OUT, exit_code, RESET_COLOR_OUT);
	} else if (WIFSIGNALED(status)) {
		int sig_num = WTERMSIG(status);
		const char *sig_str = __strsignal(sig_num);
		if (NULL != sig_str) {
			snprintf(reason, rlen, "%sSignal %s Terminated Test%s",
				 BOLD_OUT, sig_str, RESET_COLOR_OUT);
		} else {
			snprintf(reason, rlen, "%sSignal: %d%s", BOLD_OUT,
				 sig_num, RESET_COLOR_OUT);
		}
	}
	return reason;
}

static const char *__strsignal(int sig_num)
{
	if (sig_num > 31 || sig_num < 0) {
		return NULL;
	}
	// These are for x86/ARM, missing SIGEMT, SIGINFO, and SIGLOST
	static const char *SIG_TO_STRING[] = {
		[SIGHUP] = "SIGHUP",   [SIGINT] = "SIGINT",
		[SIGQUIT] = "SIGQUIT", [SIGILL] = "SIGILL",
		[SIGTRAP] = "SIGTRAP", [SIGABRT] = "SIGABRT",
		[SIGBUS] = "SIGBUS",   [SIGFPE] = "SIGFPE",
		[SIGKILL] = "SIGKILL", [SIGUSR1] = "SIGUSR1",
		[SIGSEGV] = "SIGSEGV", [SIGUSR2] = "SIGUSR2",
		[SIGPIPE] = "SIGPIPE", [SIGALRM] = "SIGALRM",
		[SIGTERM] = "SIGTERM", [SIGSTKFLT] = "SIGSTKFLT",
		[SIGCHLD] = "SIGCHLD", [SIGCONT] = "SIGCONT",
		[SIGSTOP] = "SIGSTOP", [SIGTSTP] = "SIGTSTP",
		[SIGTTIN] = "SIGTTIN", [SIGTTOU] = "SIGTTOU",
		[SIGURG] = "SIGURG",   [SIGXCPU] = "SIGXCPU",
		[SIGXFSZ] = "SIGXFSZ", [SIGVTALRM] = "SIGVTALRM",
		[SIGPROF] = "SIGPROF", [SIGWINCH] = "SIGWINCH",
		[SIGIO] = "SIGIO",     [SIGPWR] = "SIGPWR",
		[SIGSYS] = "SIGSYS",
	};
	return SIG_TO_STRING[sig_num];
}
