/*
 * This file is part of RGBDS.
 *
 * Copyright (c) 1997-2018, Carsten Sorensen and RGBDS contributors.
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Symboltable and macroargs stuff
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "asm/fixpoint.h"
#include "asm/fstack.h"
#include "asm/macro.h"
#include "asm/main.h"
#include "asm/output.h"
#include "asm/section.h"
#include "asm/symbol.h"
#include "asm/util.h"
#include "asm/warning.h"

#include "extern/err.h"

#include "hashmap.h"
#include "helpers.h"
#include "version.h"

HashMap symbols;

static char const *labelScope; /* Current section's label scope */
static struct Symbol *PCSymbol;
static char savedTIME[256];
static char savedDATE[256];
static char savedTIMESTAMP_ISO8601_LOCAL[256];
static char savedTIMESTAMP_ISO8601_UTC[256];
static char savedDAY[3];
static char savedMONTH[3];
static char savedYEAR[20];
static char savedHOUR[3];
static char savedMINUTE[3];
static char savedSECOND[3];
static bool exportall;

bool sym_IsPC(struct Symbol const *sym)
{
	return sym == PCSymbol;
}

struct ForEachArgs {
	void (*func)(struct Symbol *symbol, void *arg);
	void *arg;
};

static void forEachWrapper(void *_symbol, void *_argWrapper)
{
	struct ForEachArgs *argWrapper = _argWrapper;
	struct Symbol *symbol = _symbol;

	argWrapper->func(symbol, argWrapper->arg);
}

void sym_ForEach(void (*func)(struct Symbol *, void *), void *arg)
{
	struct ForEachArgs argWrapper = { .func = func, .arg = arg };

	hash_ForEach(symbols, forEachWrapper, &argWrapper);
}

static int32_t Callback_NARG(void)
{
	if (!macro_GetCurrentArgs()) {
		error("_NARG does not make sense outside of a macro\n");
		return 0;
	}
	return macro_NbArgs();
}

static int32_t Callback__LINE__(void)
{
	return lexer_GetLineNo();
}

static char const *Callback__FILE__(void)
{
	/*
	 * FIXME: this is dangerous, and here's why this is CURRENTLY okay. It's still bad, fix it.
	 * There are only two call sites for this; one copies the contents directly, the other is
	 * EQUS expansions, which cannot straddle file boundaries. So this should be fine.
	 */
	static char *buf = NULL;
	static size_t bufsize = 0;
	char const *fileName = fstk_GetFileName();
	size_t j = 1;

	/* TODO: is there a way for a file name to be empty? */
	assert(fileName[0]);
	/* The assertion above ensures the loop runs at least once */
	for (size_t i = 0; fileName[i]; i++, j++) {
		/* Account for the extra backslash inserted below */
		if (fileName[i] == '"')
			j++;
		/* Ensure there will be enough room; DO NOT PRINT ANYTHING ABOVE THIS!! */
		if (j + 2 >= bufsize) { /* Always keep room for 2 tail chars */
			bufsize = bufsize ? bufsize * 2 : 64;
			buf = realloc(buf, bufsize);
			if (!buf)
				fatalerror("Failed to grow buffer for file name: %s\n",
					   strerror(errno));
		}
		/* Escape quotes, since we're returning a string */
		if (fileName[i] == '"')
			buf[j - 1] = '\\';
		buf[j] = fileName[i];
	}
	/* Write everything after the loop, to ensure the buffer has been allocated */
	buf[0] = '"';
	buf[j++] = '"';
	buf[j] = '\0';
	return buf;
}

static int32_t CallbackPC(void)
{
	struct Section const *section = sect_GetSymbolSection();

	return section ? section->org + sect_GetSymbolOffset() : 0;
}

/*
 * Get the value field of a symbol
 */
int32_t sym_GetValue(struct Symbol const *sym)
{
	if (sym_IsNumeric(sym) && sym->hasCallback)
		return sym->numCallback();

	if (sym->type == SYM_LABEL)
		/* TODO: do not use section's org directly */
		return sym->value + sym_GetSection(sym)->org;

	return sym->value;
}

static void dumpFilename(struct Symbol const *sym)
{
	if (sym->src)
		fstk_Dump(sym->src, sym->fileLine);
	else if (sym->fileLine == 0)
		fputs("<command-line>", stderr);
	else
		fputs("<builtin>", stderr);
}

/*
 * Set a symbol's definition filename and line
 */
static void setSymbolFilename(struct Symbol *sym)
{
	sym->src = fstk_GetFileStack();
	sym->fileLine = sym->src ? lexer_GetLineNo() : 0; // This is (NULL, 1) for built-ins
}

/*
 * Update a symbol's definition filename and line
 */
static void updateSymbolFilename(struct Symbol *sym)
{
	struct FileStackNode *oldSrc = sym->src;

	setSymbolFilename(sym);
	/* If the old node was referenced, ensure the new one is */
	if (oldSrc && oldSrc->referenced && oldSrc->ID != -1)
		out_RegisterNode(sym->src);
	/* TODO: unref the old node, and use `out_ReplaceNode` instead if deleting it */
}

/*
 * Create a new symbol by name
 */
static struct Symbol *createsymbol(char const *s)
{
	struct Symbol *symbol = malloc(sizeof(*symbol));

	if (!symbol)
		fatalerror("Failed to create symbol '%s': %s\n", s, strerror(errno));

	if (snprintf(symbol->name, MAXSYMLEN + 1, "%s", s) > MAXSYMLEN)
		warning(WARNING_LONG_STR, "Symbol name is too long: '%s'\n", s);

	symbol->isExported = false;
	symbol->isBuiltin = false;
	symbol->hasCallback = false;
	symbol->section = NULL;
	setSymbolFilename(symbol);
	symbol->ID = -1;
	symbol->next = NULL;

	hash_AddElement(symbols, symbol->name, symbol);
	return symbol;
}

/*
 * Creates the full name of a local symbol in a given scope, by prepending
 * the name with the parent symbol's name.
 */
static void fullSymbolName(char *output, size_t outputSize,
			   char const *localName, char const *scopeName)
{
	int ret = snprintf(output, outputSize, "%s%s", scopeName, localName);

	if (ret < 0)
		fatalerror("snprintf error when expanding symbol name: %s", strerror(errno));
	else if ((size_t)ret >= outputSize)
		fatalerror("Symbol name is too long: '%s%s'\n", scopeName, localName);
}

static void assignStringSymbol(struct Symbol *sym, char const *value)
{
	char *string = strdup(value);

	if (string == NULL)
		fatalerror("No memory for string equate: %s\n", strerror(errno));

	sym->type = SYM_EQUS;
	/* TODO: use other fields */
	sym->macro = string;
	sym->macroSize = strlen(string);
}

struct Symbol *sym_FindExactSymbol(char const *name)
{
	return hash_GetElement(symbols, name);
}

struct Symbol *sym_FindUnscopedSymbol(char const *name)
{
	if (strchr(name, '.')) {
		error("Expected non-scoped symbol name, not \"%s\"\n", name);
		return NULL;
	}
	return sym_FindExactSymbol(name);
}

struct Symbol *sym_FindScopedSymbol(char const *name)
{
	char const *dotPtr = strchr(name, '.');

	if (dotPtr) {
		if (strchr(dotPtr + 1, '.'))
			fatalerror("'%s' is a nonsensical reference to a nested local symbol\n",
				   name);
		/* If auto-scoped local label, expand the name */
		if (dotPtr == name) { /* Meaning, the name begins with the dot */
			char fullname[MAXSYMLEN + 1];

			fullSymbolName(fullname, sizeof(fullname), name, labelScope);
			return sym_FindExactSymbol(fullname);
		}
	}
	return sym_FindExactSymbol(name);
}

struct Symbol const *sym_GetPC(void)
{
	return PCSymbol;
}

static inline bool isReferenced(struct Symbol const *sym)
{
	return sym->ID != -1;
}

/*
 * Purge a symbol
 */
void sym_Purge(char const *symName)
{
	struct Symbol *symbol = sym_FindScopedSymbol(symName);

	if (!symbol) {
		error("'%s' not defined\n", symName);
	} else if (symbol->isBuiltin) {
		error("Built-in symbol '%s' cannot be purged\n", symName);
	} else if (isReferenced(symbol)) {
		error("Symbol \"%s\" is referenced and thus cannot be purged\n", symName);
	} else {
		/* Do not keep a reference to the label's name after purging it */
		if (symbol->name == labelScope)
			labelScope = NULL;

		/*
		 * FIXME: this leaks symbol->macro for SYM_EQUS and SYM_MACRO, but this can't
		 * free(symbol->macro) because the expansion may be purging itself.
		 */

		hash_RemoveElement(symbols, symbol->name);
		/* TODO: ideally, also unref the file stack nodes */
		free(symbol);
	}
}

uint32_t sym_GetPCValue(void)
{
	struct Section const *sect = sect_GetSymbolSection();

	if (!sect)
		error("PC has no value outside a section\n");
	else if (sect->org == -1)
		error("Expected constant PC but section is not fixed\n");
	else
		return CallbackPC();
	return 0;
}

/*
 * Return a constant symbol's value, assuming it's defined
 */
uint32_t sym_GetConstantSymValue(struct Symbol const *sym)
{
	if (sym == PCSymbol)
		return sym_GetPCValue();
	else if (!sym_IsConstant(sym))
		error("\"%s\" does not have a constant value\n", sym->name);
	else
		return sym_GetValue(sym);

	return 0;
}

/*
 * Return a constant symbol's value
 */
uint32_t sym_GetConstantValue(char const *s)
{
	struct Symbol const *sym = sym_FindScopedSymbol(s);

	if (sym == NULL)
		error("'%s' not defined\n", s);
	else
		return sym_GetConstantSymValue(sym);

	return 0;
}

char const *sym_GetCurrentSymbolScope(void)
{
	return labelScope;
}

void sym_SetCurrentSymbolScope(char const *newScope)
{
	labelScope = newScope;
}

/*
 * Create a symbol that will be non-relocatable and ensure that it
 * hasn't already been defined or referenced in a context that would
 * require that it be relocatable
 * @param symbolName The name of the symbol to create
 * @param numeric If false, the symbol may not have been referenced earlier
 */
static struct Symbol *createNonrelocSymbol(char const *symbolName, bool numeric)
{
	struct Symbol *symbol = sym_FindExactSymbol(symbolName);

	if (!symbol) {
		symbol = createsymbol(symbolName);
	} else if (sym_IsDefined(symbol)) {
		error("'%s' already defined at ", symbolName);
		dumpFilename(symbol);
		putc('\n', stderr);
	} else if (!numeric) {
		// The symbol has already been referenced, but it's not allowed
		error("'%s' already referenced at ", symbolName);
		dumpFilename(symbol);
		putc('\n', stderr);
		return NULL; // Don't allow overriding the symbol, that'd be bad!
	}

	return symbol;
}

/*
 * Add an equated symbol
 */
struct Symbol *sym_AddEqu(char const *symName, int32_t value)
{
	struct Symbol *sym = createNonrelocSymbol(symName, true);

	if (!sym)
		return NULL;

	sym->type = SYM_EQU;
	sym->value = value;

	return sym;
}

/*
 * Add a string equated symbol.
 *
 * If the desired symbol is a string it needs to be passed to this function with
 * quotes inside the string, like sym_AddString("name", "\"test\"), or the
 * assembler won't be able to use it with DB and similar. This is equivalent to
 * ``` name EQUS "\"test\"" ```
 *
 * If the desired symbol is a register or a number, just the terminator quotes
 * of the string are enough: sym_AddString("M_PI", "3.1415"). This is the same
 * as ``` M_PI EQUS "3.1415" ```
 */
struct Symbol *sym_AddString(char const *symName, char const *value)
{
	struct Symbol *sym = createNonrelocSymbol(symName, false);

	if (!sym)
		return NULL;

	assignStringSymbol(sym, value);
	return sym;
}

struct Symbol *sym_RedefString(char const *symName, char const *value)
{
	struct Symbol *sym = sym_FindExactSymbol(symName);

	if (!sym) {
		sym = createsymbol(symName);
	} else if (sym->type != SYM_EQUS) {
		error("'%s' already defined as non-EQUS at ", symName);
		dumpFilename(sym);
		putc('\n', stderr);
	}

	/*
	 * FIXME: this leaks the previous sym->macro value, but this can't
	 * free(sym->macro) because the expansion may be redefining itself.
	 */

	assignStringSymbol(sym, value);

	return sym;
}

/*
 * Alter a SET symbols value
 */
struct Symbol *sym_AddSet(char const *symName, int32_t value)
{
	struct Symbol *sym = sym_FindExactSymbol(symName);

	if (sym == NULL) {
		sym = createsymbol(symName);
	} else if (sym_IsDefined(sym) && sym->type != SYM_SET) {
		error("'%s' already defined as %s at ",
		      symName, sym->type == SYM_LABEL ? "label" : "constant");
		dumpFilename(sym);
		putc('\n', stderr);
		return sym;
	} else {
		updateSymbolFilename(sym);
	}

	sym->type = SYM_SET;
	sym->value = value;

	return sym;
}

/*
 * Add a label (aka "relocatable symbol")
 * @param name The label's full name (so `.name` is invalid)
 * @return The created symbol
 */
static struct Symbol *addLabel(char const *name)
{
	assert(name[0] != '.'); /* The symbol name must have been expanded prior */
	struct Symbol *sym = sym_FindExactSymbol(name);

	if (!sym) {
		sym = createsymbol(name);
	} else if (sym_IsDefined(sym)) {
		error("'%s' already defined at ", name);
		dumpFilename(sym);
		putc('\n', stderr);
		return NULL;
	} else {
		updateSymbolFilename(sym);
	}
	/* If the symbol already exists as a ref, just "take over" it */
	sym->type = SYM_LABEL;
	sym->value = sect_GetSymbolOffset();
	if (exportall)
		sym->isExported = true;
	sym->section = sect_GetSymbolSection();

	if (sym && !sym->section)
		error("Label \"%s\" created outside of a SECTION\n", name);
	return sym;
}

/*
 * Add a local (.name or Parent.name) relocatable symbol
 */
struct Symbol *sym_AddLocalLabel(char const *name)
{
	if (!labelScope) {
		error("Local label '%s' in main scope\n", name);
		return NULL;
	}

	char fullname[MAXSYMLEN + 1];

	if (name[0] == '.') {
		/* If symbol is of the form `.name`, expand to the full `Parent.name` name */
		fullSymbolName(fullname, sizeof(fullname), name, labelScope);
		name = fullname; /* Use the expanded name instead */
	} else {
		size_t i = 0;

		/* Otherwise, check that `Parent` is in fact the current scope */
		while (labelScope[i] && name[i] == labelScope[i])
			i++;
		/* Assuming no dots in `labelScope` */
		assert(strchr(&name[i], '.')); /* There should be at least one dot, though */
		size_t parentLen = i + (strchr(&name[i], '.') - name);

		/*
		 * Check that `labelScope[i]` ended the check, guaranteeing that `name` is at least
		 * as long, and then that this was the entirety of the `Parent` part of `name`.
		 */
		if (labelScope[i] != '\0' || name[i] != '.') {
			assert(parentLen <= INT_MAX);
			error("Not currently in the scope of '%.*s'\n", (int)parentLen, name);
		}
		if (strchr(&name[parentLen + 1], '.')) /* There will at least be a terminator */
			fatalerror("'%s' is a nonsensical reference to a nested local label\n",
				   name);
	}

	return addLabel(name);
}

/*
 * Add a relocatable symbol
 */
struct Symbol *sym_AddLabel(char const *name)
{
	struct Symbol *sym = addLabel(name);

	/* Set the symbol as the new scope */
	if (sym)
		labelScope = sym->name;
	return sym;
}

static uint32_t anonLabelID;

/*
 * Add an anonymous label
 */
struct Symbol *sym_AddAnonLabel(void)
{
	if (anonLabelID == UINT32_MAX) {
		error("Only %" PRIu32 " anonymous labels can be created!", anonLabelID);
		return NULL;
	}
	char name[MAXSYMLEN + 1];

	sym_WriteAnonLabelName(name, 0, true); // The direction is important!!
	anonLabelID++;
	return addLabel(name);
}

/*
 * Write an anonymous label's name to a buffer
 */
void sym_WriteAnonLabelName(char buf[MIN_NB_ELMS(MAXSYMLEN + 1)], uint32_t ofs, bool neg)
{
	uint32_t id = 0;

	if (neg) {
		if (ofs > anonLabelID)
			error("Reference to anonymous label %" PRIu32 " before, when only %" PRIu32
			      " ha%s been created so far\n",
			      ofs, anonLabelID, anonLabelID == 1 ? "s" : "ve");
		else
			id = anonLabelID - ofs;
	} else {
		ofs--; // We're referencing symbols that haven't been created yet...
		if (ofs > UINT32_MAX - anonLabelID)
			error("Reference to anonymous label %" PRIu32 " after, when only %" PRIu32
			      " may still be created\n", ofs + 1, UINT32_MAX - anonLabelID);
		else
			id = anonLabelID + ofs;
	}

	sprintf(buf, "!%u", id);
}

/*
 * Export a symbol
 */
void sym_Export(char const *symName)
{
	if (symName[0] == '!') {
		error("Anonymous labels cannot be exported\n");
		return;
	}

	struct Symbol *sym = sym_FindScopedSymbol(symName);

	/* If the symbol doesn't exist, create a ref that can be purged */
	if (!sym)
		sym = sym_Ref(symName);
	sym->isExported = true;
}

/*
 * Add a macro definition
 */
struct Symbol *sym_AddMacro(char const *symName, int32_t defLineNo, char *body, size_t size)
{
	struct Symbol *sym = createNonrelocSymbol(symName, false);

	if (!sym)
		return NULL;

	sym->type = SYM_MACRO;
	sym->macroSize = size;
	sym->macro = body;
	setSymbolFilename(sym); /* TODO: is this really necessary? */
	/*
	 * The symbol is created at the line after the `endm`,
	 * override this with the actual definition line
	 */
	sym->fileLine = defLineNo;

	return sym;
}

/*
 * Flag that a symbol is referenced in an RPN expression
 * and create it if it doesn't exist yet
 */
struct Symbol *sym_Ref(char const *symName)
{
	struct Symbol *nsym = sym_FindScopedSymbol(symName);

	if (nsym == NULL) {
		char fullname[MAXSYMLEN + 1];

		if (symName[0] == '.') {
			if (!labelScope)
				fatalerror("Local label reference '%s' in main scope\n", symName);
			fullSymbolName(fullname, sizeof(fullname), symName, labelScope);
			symName = fullname;
		}

		nsym = createsymbol(symName);
		nsym->type = SYM_REF;
	}

	return nsym;
}

/*
 * Set whether to export all relocatable symbols by default
 */
void sym_SetExportAll(bool set)
{
	exportall = set;
}

/**
 * Returns a pointer to the first non-zero character in a string
 * Non-'0', not non-'\0'.
 */
static inline char const *removeLeadingZeros(char const *ptr)
{
	while (*ptr == '0')
		ptr++;
	return ptr;
}

static inline struct Symbol *createBuiltinSymbol(char const *name)
{
	struct Symbol *sym = createsymbol(name);

	sym->isBuiltin = true;
	sym->hasCallback = true;
	sym->src = NULL;
	sym->fileLine = 1; // This is 0 for CLI-defined symbols
	return sym;
}

/*
 * Initialize the symboltable
 */
void sym_Init(time_t now)
{
	PCSymbol = createBuiltinSymbol("@");
	struct Symbol *_NARGSymbol = createBuiltinSymbol("_NARG");
	struct Symbol *__LINE__Symbol = createBuiltinSymbol("__LINE__");
	struct Symbol *__FILE__Symbol = createBuiltinSymbol("__FILE__");

	PCSymbol->type = SYM_LABEL;
	PCSymbol->section = NULL;
	PCSymbol->numCallback = CallbackPC;
	_NARGSymbol->type = SYM_EQU;
	_NARGSymbol->numCallback = Callback_NARG;
	__LINE__Symbol->type = SYM_EQU;
	__LINE__Symbol->numCallback = Callback__LINE__;
	__FILE__Symbol->type = SYM_EQUS;
	__FILE__Symbol->strCallback = Callback__FILE__;
	sym_AddSet("_RS", 0)->isBuiltin = true;

	sym_AddEqu("__RGBDS_MAJOR__", PACKAGE_VERSION_MAJOR)->isBuiltin = true;
	sym_AddEqu("__RGBDS_MINOR__", PACKAGE_VERSION_MINOR)->isBuiltin = true;
	sym_AddEqu("__RGBDS_PATCH__", PACKAGE_VERSION_PATCH)->isBuiltin = true;
#ifdef PACKAGE_VERSION_RC
	sym_AddEqu("__RGBDS_RC__", PACKAGE_VERSION_RC)->isBuiltin = true;
#endif

	if (now == (time_t)-1) {
		warn("Couldn't determine current time");
		/* Fall back by pretending we are at the Epoch */
		now = 0;
	}

	const struct tm *time_utc = gmtime(&now);
	const struct tm *time_local = localtime(&now);

	strftime(savedTIME, sizeof(savedTIME), "\"%H:%M:%S\"", time_local);
	strftime(savedDATE, sizeof(savedDATE), "\"%d %B %Y\"", time_local);
	strftime(savedTIMESTAMP_ISO8601_LOCAL,
		 sizeof(savedTIMESTAMP_ISO8601_LOCAL), "\"%Y-%m-%dT%H:%M:%S%z\"",
		 time_local);

	strftime(savedTIMESTAMP_ISO8601_UTC,
		 sizeof(savedTIMESTAMP_ISO8601_UTC), "\"%Y-%m-%dT%H:%M:%SZ\"",
		 time_utc);

	strftime(savedYEAR, sizeof(savedYEAR), "%Y", time_utc);
	strftime(savedMONTH, sizeof(savedMONTH), "%m", time_utc);
	strftime(savedDAY, sizeof(savedDAY), "%d", time_utc);
	strftime(savedHOUR, sizeof(savedHOUR), "%H", time_utc);
	strftime(savedMINUTE, sizeof(savedMINUTE), "%M", time_utc);
	strftime(savedSECOND, sizeof(savedSECOND), "%S", time_utc);

#define addString(name, val) sym_AddString(name, val)->isBuiltin = true
	addString("__TIME__", savedTIME);
	addString("__DATE__", savedDATE);
	addString("__ISO_8601_LOCAL__", savedTIMESTAMP_ISO8601_LOCAL);
	addString("__ISO_8601_UTC__", savedTIMESTAMP_ISO8601_UTC);
	/* This cannot start with zeros */
	addString("__UTC_YEAR__", savedYEAR);
	addString("__UTC_MONTH__", removeLeadingZeros(savedMONTH));
	addString("__UTC_DAY__", removeLeadingZeros(savedDAY));
	addString("__UTC_HOUR__", removeLeadingZeros(savedHOUR));
	addString("__UTC_MINUTE__", removeLeadingZeros(savedMINUTE));
	addString("__UTC_SECOND__", removeLeadingZeros(savedSECOND));
#undef addString

	labelScope = NULL;
	anonLabelID = 0;

	/* _PI is deprecated */
	struct Symbol *_PISymbol = createBuiltinSymbol("_PI");

	_PISymbol->type = SYM_EQU;
	_PISymbol->src = NULL;
	_PISymbol->fileLine = 0;
	_PISymbol->hasCallback = true;
	_PISymbol->numCallback = fix_Callback_PI;
}
