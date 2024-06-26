/*
 * Copyright (C) 2017 Antonio Nino Diaz <antonio_nd@outlook.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef RGBDS_LINK_SCRIPT_H
#define RGBDS_LINK_SCRIPT_H

#include <stdint.h>

#include "extern/stdnoreturn.h"

noreturn void script_fatalerror(const char *fmt, ...);

void script_Parse(const char *path);

void script_IncludeFile(const char *path);
int32_t script_IncludeDepthGet(void);
void script_IncludePop(void);

void script_InitSections(void);
void script_SetCurrentSectionType(const char *type, uint32_t bank);
void script_SetAddress(uint32_t addr);
void script_SetAlignment(uint32_t alignment);
void script_OutputSection(const char *section_name);

#endif /* RGBDS_LINK_SCRIPT_H */
