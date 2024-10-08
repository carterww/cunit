#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARG_RESULT_DEFAULT (0)
#define ARG_RESULT_JSON (1)

#define ARG_COLOR_DISABLED (1 << 0)
#define ARG_INTERACTIVE_MODE (1 << 1)

struct suite_to_tests {
	const char *suite;
	const char *tests[];
};

struct arguments {
	uint8_t output;
	uint8_t boolean_flags;
	uint16_t concurrent_suites;
	const char **suites_full;
	const struct suite_to_tests *suites_partial;
};

static struct arguments args = { 0 };

static char *__skip_whitespace(const char *a);
static int __parse_next_arg(int argc, char *argv[], int curr);
static int __parse_concurrent_suites(const char *a);
static const char **__parse_suites_full(char *a);
static void __print_help(int status);

struct arguments *arguments_get(int argc, char *argv[])
{
	if (argc <= 1) {
		return &args;
	}
	for (int i = 1; i < argc;) {
		i = __parse_next_arg(argc, argv, i);
	}

	return &args;
}

static char *__skip_whitespace(const char *a)
{
	while (*a) {
		switch (*a) {
		case ' ':
		case '\n':
		case '\t':
			++a;
			continue;
		default:
			break;
		}
	}
	return (char *)a;
}

static int __parse_next_arg(int argc, char *argv[], int curr)
{
	char *a = argv[curr];
	a = __skip_whitespace(a);
	if (strcmp(a, "j") > 0) {
		args.concurrent_suites = __parse_concurrent_suites(a);
		++curr;
	} else if (!strcmp(a, "s")) {
		if (curr + 2 <= argc) {
			args.suites_full = __parse_suites_full(argv[curr + 1]);
			curr += 2;
		} else {
			__print_help(1);
		}
	} else if (!strcmp(a, "f")) {
	} else if (!strcmp(a, "h")) {
		__print_help(0);
	} else if (!strcmp(a, "no-color")) {
		args.boolean_flags |= ARG_COLOR_DISABLED;
		++curr;
	} else if (!strcmp(a, "interactive")) {
		args.boolean_flags |= ARG_INTERACTIVE_MODE;
		++curr;
	} else if (strcmp(a, "suite=") > 0) {
	} else {
		__print_help(1);
	}
	return curr;
}

static int __parse_concurrent_suites(const char *a)
{
	++a;
	int j = atoi(a);
	if (!j) {
		__print_help(1);
	}
	return j;
}

static const char **__parse_suites_full(char *a)
{
	size_t suites_num = 1;
	a = __skip_whitespace(a);
	for (char *tmp = a; *tmp; ++tmp) {
		if (',' == *tmp) {
			++suites_num;
		}
	}
	const char **suites = malloc(sizeof(*suites) * suites_num);
	for (size_t i = 0; i < suites_num; ++i) {
		char *beg = a;
		while (*a) {
			if (',' != *a) {
				++a;
				continue;
			}
			*a = '\0';
			suites[i] = beg;
			beg = ++a;
			break;
		}
	}
	return suites;
}

static void __print_help(int status)
{
	exit(status);
}
