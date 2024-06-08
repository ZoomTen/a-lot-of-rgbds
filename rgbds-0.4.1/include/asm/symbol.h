/*
 * This file is part of RGBDS.
 *
 * Copyright (c) 1997-2020, Carsten Sorensen and RGBDS contributors.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RGBDS_SYMBOL_H
#define RGBDS_SYMBOL_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "asm/section.h"

#include "types.h"

#define HASHSIZE	(1 << 16)
#define MAXSYMLEN	256

enum SymbolType {
	SYM_LABEL,
	SYM_EQU,
	SYM_SET,
	SYM_MACRO,
	SYM_EQUS,
	SYM_REF // Forward reference to a label
};

struct Symbol {
	char name[MAXSYMLEN + 1];
	enum SymbolType type;
	bool isExported; /* Whether the symbol is to be exported */
	bool isBuiltin;  /* Whether the symbol is a built-in */
	struct Symbol const *scope;
	struct Section *section;
	char fileName[_MAX_PATH + 1]; /* File where the symbol was defined. */
	uint32_t fileLine; /* Line where the symbol was defined. */

	union {
		struct { /* If sym_IsNumeric */
			int32_t value;
			int32_t (*callback)(void);
		};
		struct { /* For SYM_MACRO */
			uint32_t macroSize;
			char *macro;
		};
	};

	uint32_t ID; /* ID of the symbol in the object file (-1 if none) */
	struct Symbol *next; /* Next object to output in the object file */
};

bool sym_IsPC(struct Symbol const *sym);

static inline bool sym_IsDefined(struct Symbol const *sym)
{
	return sym->type != SYM_REF;
}

static inline struct Section *sym_GetSection(struct Symbol const *sym)
{
	return sym_IsPC(sym) ? sect_GetSymbolSection() : sym->section;
}

static inline bool sym_IsConstant(struct Symbol const *sym)
{
	if (sym->type == SYM_LABEL) {
		struct Section const *sect = sym_GetSection(sym);

		return sect && sect->nOrg != -1;
	}
	return sym->type == SYM_EQU || sym->type == SYM_SET;
}

static inline bool sym_IsNumeric(struct Symbol const *sym)
{
	return sym->type == SYM_LABEL || sym->type == SYM_EQU
	    || sym->type == SYM_SET;
}

static inline bool sym_IsLabel(struct Symbol const *sym)
{
	return sym->type == SYM_LABEL || sym->type == SYM_REF;
}

static inline bool sym_IsLocal(struct Symbol const *sym)
{
	return sym_IsLabel(sym) && strchr(sym->name, '.');
}

static inline bool sym_IsExported(struct Symbol const *sym)
{
	return sym->isExported;
}

/*
 * Get a string equate's value
 */
static inline char const *sym_GetStringValue(struct Symbol const *sym)
{
	return sym->macro;
}

void sym_ForEach(void (*func)(struct Symbol *, void *), void *arg);

int32_t sym_GetValue(struct Symbol const *sym);
void sym_SetExportAll(bool set);
struct Symbol *sym_AddLocalReloc(char const *symName);
struct Symbol *sym_AddReloc(char const *symName);
void sym_Export(char const *symName);
struct Symbol *sym_AddEqu(char const *symName, int32_t value);
struct Symbol *sym_AddSet(char const *symName, int32_t value);
uint32_t sym_GetPCValue(void);
uint32_t sym_GetConstantValue(char const *s);
struct Symbol *sym_FindSymbol(char const *symName);
struct Symbol *sym_AddMacro(char const *symName, int32_t defLineNo);
struct Symbol *sym_Ref(char const *symName);
struct Symbol *sym_AddString(char const *symName, char const *value);
uint32_t sym_GetDefinedValue(char const *s);
void sym_Purge(char const *symName);
void sym_Init(void);

/* Functions to save and restore the current symbol scope. */
struct Symbol *sym_GetCurrentSymbolScope(void);
void sym_SetCurrentSymbolScope(struct Symbol *newScope);

#endif /* RGBDS_SYMBOL_H */
