/*
 * This file is part of RGBDS.
 *
 * Copyright (c) 2005-2018, Rich Felker and RGBDS contributors.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "extern/err.h"

void rgbds_vwarn(const char *fmt, va_list ap)
{
	fprintf(stderr, "warning");
	if (fmt) {
		fputs(": ", stderr);
		vfprintf(stderr, fmt, ap);
	}
	putc('\n', stderr);
	perror(0);
}

void rgbds_vwarnx(const char *fmt, va_list ap)
{
	fprintf(stderr, "warning");
	if (fmt) {
		fputs(": ", stderr);
		vfprintf(stderr, fmt, ap);
	}
	putc('\n', stderr);
}

noreturn void rgbds_verr(int status, const char *fmt, va_list ap)
{
	fprintf(stderr, "error");
	if (fmt) {
		fputs(": ", stderr);
		vfprintf(stderr, fmt, ap);
	}
	putc('\n', stderr);
	exit(status);
}

noreturn void rgbds_verrx(int status, const char *fmt, va_list ap)
{
	fprintf(stderr, "error");
	if (fmt) {
		fputs(": ", stderr);
		vfprintf(stderr, fmt, ap);
	}
	putc('\n', stderr);
	exit(status);
}

void rgbds_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarn(fmt, ap);
	va_end(ap);
}

void rgbds_warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

noreturn void rgbds_err(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(status, fmt, ap);
	va_end(ap);
}

noreturn void rgbds_errx(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verrx(status, fmt, ap);
	va_end(ap);
}
