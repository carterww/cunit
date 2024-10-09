#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARG_OUT_FORMAT_DEFAULT (0)
#define ARG_OUT_FORMAT_JSON (1)

#define ARG_COLOR_DISABLED (1 << 0)
#define ARG_INTERACTIVE_MODE (1 << 1)

struct suite_to_tests {
	const char *suite;
	const char **tests;
};

struct arguments {
	uint8_t output_format;
	uint8_t boolean_flags;
	uint16_t concurrent_suites;
	uint16_t suites_partial_length;
	struct suite_to_tests *suites_partial;
	const char **suites_full;
};

static struct arguments args = { 0 };

static char *__skip_leading_chars(const char *a);
static int __parse_next_arg(int argc, char *argv[], int curr);
static int __parse_concurrent_suites(const char *a);
static const char **__parse_suites_full(char *a);
static uint8_t __parse_output_format(char *a);
static void __parse_suite_partial(char *suite, char *tests);
static const char **__char_array_from_csv(char *a);
static void __print_help(int status);
static void __to_lower(char *a);

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

static char *__skip_leading_chars(const char *a)
{
	while (*a) {
		switch (*a) {
		case ' ':
		case '\n':
		case '\t':
		case '-':
			++a;
			continue;
		default:
			return (char *)a;
		}
	}
	return (char *)a;
}

static int __parse_next_arg(int argc, char *argv[], int curr)
{
	char *a = argv[curr];
	a = __skip_leading_chars(a);
	if (!strcmp(a, "s")) {
		if (curr + 1 <= argc) {
			args.suites_full = __parse_suites_full(argv[curr + 1]);
			curr += 2;
		} else {
			__print_help(1);
		}
	} else if (!strcmp(a, "f")) {
		if (curr + 2 <= argc) {
			args.output_format =
				__parse_output_format(argv[curr + 1]);
			curr += 2;
		} else {
			__print_help(1);
		}
	} else if (!strcmp(a, "h")) {
		__print_help(0);
	} else if (!strcmp(a, "no-color")) {
		args.boolean_flags |= ARG_COLOR_DISABLED;
		++curr;
	} else if (!strcmp(a, "interactive")) {
		args.boolean_flags |= ARG_INTERACTIVE_MODE;
		++curr;
	} else if ('j' == *a) {
		args.concurrent_suites = __parse_concurrent_suites(a);
		++curr;
	} else if (strcmp(a, "suite=") > 0) {
		if (curr + 2 <= argc) {
			__parse_suite_partial(argv[curr], argv[curr + 1]);
			curr += 2;
		} else {
			__print_help(1);
		}
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
	return __char_array_from_csv(a);
}

static uint8_t __parse_output_format(char *a)
{
	__to_lower(a);
	if (!strcmp(a, "json")) {
		return ARG_OUT_FORMAT_JSON;
	} else {
		return ARG_OUT_FORMAT_DEFAULT;
	}
}

static void __parse_suite_partial(char *suite, char *tests)
{
	while (*suite) {
		char c = *suite;
		++suite;
		if ('=' == c) {
			break;
		}
	}
	struct suite_to_tests val = { 0 };
	val.suite = suite;
	val.tests = __char_array_from_csv(tests);
	if (NULL == args.suites_partial) {
		args.suites_partial = malloc(sizeof(*args.suites_partial));
		if (NULL == args.suites_partial) {
			exit(1);
		}
	} else {
		args.suites_partial =
			realloc(args.suites_partial,
				sizeof(*args.suites_partial) *
					(args.suites_partial_length + 1));
	}
	args.suites_partial[args.suites_partial_length++] = val;
}

static const char **__char_array_from_csv(char *a)
{
	size_t val_num = 1;
	a = __skip_leading_chars(a);
	for (char *tmp = a; *tmp; ++tmp) {
		if (',' == *tmp) {
			++val_num;
		}
	}
	const char **arr = malloc(sizeof(*arr) * (val_num + 1));
	char *beg = a;
	for (size_t i = 0; i < val_num; ++i) {
		while (*a) {
			if (',' != *a) {
				++a;
				continue;
			}
			*a = '\0';
			arr[i] = beg;
			beg = ++a;
			break;
		}
	}
	arr[val_num - 1] = beg;
	arr[val_num] = NULL;
	return arr;
}

static void __print_help(int status)
{
	exit(status);
}

static void __to_lower(char *a)
{
	while (*a) {
		if (*a >= 'A' && *a <= 'Z') {
			*a += 32;
		}
		++a;
	}
}

#undef ARG_OUT_FORMAT_DEFAULT
#undef ARG_OUT_FORMAT_JSON

#undef ARG_COLOR_DISABLED
#undef ARG_INTERACTIVE_MODE
