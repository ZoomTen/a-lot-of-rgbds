/*
 * This file is part of RGBDS.
 *
 * Copyright (c) 2019, Eldred Habert and RGBDS contributors.
 *
 * SPDX-License-Identifier: MIT
 */

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "link/patch.h"
#include "link/section.h"
#include "link/symbol.h"

#include "linkdefs.h"

#include "extern/err.h"

static int32_t asl(int32_t value, int32_t shiftamt); // Forward decl for below
static int32_t asr(int32_t value, int32_t shiftamt)
{
	uint32_t uvalue = value;

	// Get the easy cases out of the way
	if (shiftamt == 0)
		return value;
	if (value == 0 || shiftamt <= -32)
		return 0;
	if (shiftamt > 31)
		return (value < 0) ? -1 : 0;
	if (shiftamt < 0)
		return asl(value, -shiftamt);
	if (value > 0)
		return uvalue >> shiftamt;

	{
		// Calculate an OR mask for sign extension
		// 1->0x80000000, 2->0xC0000000, ..., 31->0xFFFFFFFE
		uint32_t shiftamt_high_bits = -((uint32_t)1 << (32 - shiftamt));

		return (uvalue >> shiftamt) | shiftamt_high_bits;
	}
}

static int32_t asl(int32_t value, int32_t shiftamt)
{
	// Repeat the easy cases here to avoid INT_MIN funny business
	if (shiftamt == 0)
		return value;
	if (value == 0 || shiftamt >= 32)
		return 0;
	if (shiftamt < -31)
		return (value < 0) ? -1 : 0;
	if (shiftamt < 0)
		return asr(value, -shiftamt);

	return (uint32_t)value << shiftamt;
}

/* This is an "empty"-type stack */
struct RPNStack {
	int32_t *buf;
	size_t size;
	size_t capacity;
} stack;

static inline void initRPNStack(void)
{
	stack.capacity = 64;
	stack.buf = malloc(sizeof(*stack.buf) * stack.capacity);
	if (!stack.buf)
		err(1, "Failed to init RPN stack");
}

static inline void clearRPNStack(void)
{
	stack.size = 0;
}

static void pushRPN(int32_t value)
{
	if (stack.size >= stack.capacity) {
		static const size_t increase_factor = 2;

		if (stack.capacity > SIZE_MAX / increase_factor)
			errx(1, "Overflow in RPN stack resize");

		stack.capacity *= increase_factor;
		stack.buf =
			realloc(stack.buf, sizeof(*stack.buf) * stack.capacity);
		/*
		 * Static analysis tools complain that the capacity might become
		 * zero due to overflow, but fail to realize that it's caught by
		 * the overflow check above. Hence the stringent check below.
		 */
		if (!stack.buf || !stack.capacity)
			err(1, "Failed to resize RPN stack");
	}

	stack.buf[stack.size] = value;
	stack.size++;
}

static int32_t popRPN(char const *fileName)
{
	if (stack.size == 0)
		errx(1, "%s: Internal error, RPN stack empty", fileName);

	stack.size--;
	return stack.buf[stack.size];
}

static inline void freeRPNStack(void)
{
	free(stack.buf);
}

/* RPN operators */

static uint32_t getRPNByte(uint8_t const **expression, int32_t *size,
			   char const *fileName)
{
	if (!(*size)--)
		errx(1, "%s: RPN expression overread", fileName);
	return *(*expression)++;
}

static struct Symbol const *getSymbol(struct Symbol const * const *symbolList,
				      uint32_t index)
{
	struct Symbol const *symbol = symbolList[index];

	/* If the symbol is defined elsewhere... */
	if (symbol->type == SYMTYPE_IMPORT)
		return sym_GetSymbol(symbol->name);

	return symbol;
}

/**
 * Compute a patch's value from its RPN string.
 * @param patch The patch to compute the value of
 * @param section The section the patch is contained in
 * @return The patch's value
 */
static int32_t computeRPNExpr(struct Patch const *patch,
			      struct Symbol const * const *fileSymbols)
{
/* Small shortcut to avoid a lot of repetition */
#define popRPN() popRPN(patch->fileName)

	uint8_t const *expression = patch->rpnExpression;
	int32_t size = patch->rpnSize;

	clearRPNStack();

	while (size > 0) {
		enum RPNCommand command = getRPNByte(&expression, &size,
						     patch->fileName);
		int32_t value;

		/*
		 * Friendly reminder:
		 * Be VERY careful with two `popRPN` in the same expression.
		 * C does not guarantee order of evaluation of operands!!
		 * So, if there are two `popRPN` in the same expression, make
		 * sure the operation is commutative.
		 */
		switch (command) {
			struct Symbol const *symbol;
			char const *name;
			struct Section const *sect;

		case RPN_ADD:
			value = popRPN() + popRPN();
			break;
		case RPN_SUB:
			value = popRPN();
			value = popRPN() - value;
			break;
		case RPN_MUL:
			value = popRPN() * popRPN();
			break;
		case RPN_DIV:
			value = popRPN();
			if (value == 0) {
				error("%s: Division by 0", patch->fileName);
				popRPN();
				value = INT32_MAX;
			} else {
				value = popRPN() / value;
			}
			break;
		case RPN_MOD:
			value = popRPN();
			if (value == 0) {
				error("%s: Modulo by 0", patch->fileName);
				popRPN();
				value = 0;
			} else {
				value = popRPN() % value;
			}
			break;
		case RPN_UNSUB:
			value = -popRPN();
			break;

		case RPN_OR:
			value = popRPN() | popRPN();
			break;
		case RPN_AND:
			value = popRPN() & popRPN();
			break;
		case RPN_XOR:
			value = popRPN() ^ popRPN();
			break;
		case RPN_UNNOT:
			value = ~popRPN();
			break;

		case RPN_LOGAND:
			value = popRPN();
			value = popRPN() && value;
			break;
		case RPN_LOGOR:
			value = popRPN();
			value = popRPN() || value;
			break;
		case RPN_LOGUNNOT:
			value = !popRPN();
			break;

		case RPN_LOGEQ:
			value = popRPN() == popRPN();
			break;
		case RPN_LOGNE:
			value = popRPN() != popRPN();
			break;
		case RPN_LOGGT:
			value = popRPN();
			value = popRPN() > value;
			break;
		case RPN_LOGLT:
			value = popRPN();
			value = popRPN() < value;
			break;
		case RPN_LOGGE:
			value = popRPN();
			value = popRPN() >= value;
			break;
		case RPN_LOGLE:
			value = popRPN();
			value = popRPN() <= value;
			break;

		case RPN_SHL:
			value = popRPN();
			value = asl(popRPN(), value);
			break;
		case RPN_SHR:
			value = popRPN();
			value = asr(popRPN(), value);
			break;

		case RPN_BANK_SYM:
			value = 0;
			for (uint8_t shift = 0; shift < 32; shift += 8)
				value |= getRPNByte(&expression, &size,
						    patch->fileName) << shift;
			symbol = getSymbol(fileSymbols, value);

			if (!symbol) {
				error("%s: Requested BANK() of symbol \"%s\", which was not found",
				      patch->fileName,
				      fileSymbols[value]->name);
				value = 1;
			} else if (!symbol->section) {
				error("%s: Requested BANK() of non-label symbol \"%s\"",
				      patch->fileName,
				      fileSymbols[value]->name);
				value = 1;
			} else {
				value = symbol->section->bank;
			}
			break;

		case RPN_BANK_SECT:
			name = (char const *)expression;
			while (getRPNByte(&expression, &size, patch->fileName))
				;

			sect = sect_GetSection(name);

			if (!sect) {
				error("%s: Requested BANK() of section \"%s\", which was not found",
				      patch->fileName, name);
				value = 1;
			} else {
				value = sect->bank;
			}
			break;

		case RPN_BANK_SELF:
			if (!patch->pcSection) {
				error("%s: PC has no bank outside a section");
				value = 1;
			} else {
				value = patch->pcSection->bank;
			}
			break;

		case RPN_HRAM:
			value = popRPN();
			if (value < 0
			 || (value > 0xFF && value < 0xFF00)
			 || value > 0xFFFF)
				error("%s: Value %" PRId32 " is not in HRAM range",
				      patch->fileName, value);
			value &= 0xFF;
			break;

		case RPN_RST:
			value = popRPN();
			/* Acceptable values are 0x00, 0x08, 0x10, ..., 0x38
			 * They can be easily checked with a bitmask
			 */
			if (value & ~0x38)
				error("%s: Value %" PRId32 " is not a RST vector",
				      patch->fileName, value);
			value |= 0xC7;
			break;

		case RPN_CONST:
			value = 0;
			for (uint8_t shift = 0; shift < 32; shift += 8)
				value |= getRPNByte(&expression, &size,
						    patch->fileName) << shift;
			break;

		case RPN_SYM:
			value = 0;
			for (uint8_t shift = 0; shift < 32; shift += 8)
				value |= getRPNByte(&expression, &size,
						    patch->fileName) << shift;

			symbol = getSymbol(fileSymbols, value);

			if (!symbol) {
				error("%s: Unknown symbol \"%s\"",
				      patch->fileName,
				      fileSymbols[value]->name);
			} else if (strcmp(symbol->name, "@")) {
				value = symbol->value;
				/* Symbols attached to sections have offsets */
				if (symbol->section)
					value += symbol->section->org;
			} else if (!patch->pcSection) {
				error("%s: PC has no value outside a section",
				      patch->fileName);
				value = 0;
			} else {
				value = patch->pcOffset + patch->pcSection->org;
			}
			break;
		}

		pushRPN(value);
	}

	if (stack.size > 1)
		error("%s: RPN stack has %zu entries on exit, not 1",
		      patch->fileName, stack.size);

	return popRPN();

#undef popRPN
}

void patch_CheckAssertions(struct Assertion *assert)
{
	verbosePrint("Checking assertions...");
	initRPNStack();

	while (assert) {
		if (!computeRPNExpr(&assert->patch,
				    (struct Symbol const * const *)
							assert->fileSymbols)) {
			switch ((enum AssertionType)assert->patch.type) {
			case ASSERT_FATAL:
				fatal("%s: %s", assert->patch.fileName,
				      assert->message[0] ? assert->message
							 : "assert failure");
				/* Not reached */
				break; /* Here so checkpatch doesn't complain */
			case ASSERT_ERROR:
				error("%s: %s", assert->patch.fileName,
				      assert->message[0] ? assert->message
							 : "assert failure");
				break;
			case ASSERT_WARN:
				warnx("%s: %s", assert->patch.fileName,
				      assert->message[0] ? assert->message
							 : "assert failure");
				break;
			}
		}
		struct Assertion *next = assert->next;

		free(assert);
		assert = next;
	}

	freeRPNStack();
}

/**
 * Applies all of a section's patches
 * @param section The section to patch
 * @param arg Ignored callback arg
 */
static void applyFilePatches(struct Section *section, struct Section *dataSection)
{
	if (!sect_HasData(section->type))
		return;

	verbosePrint("Patching section \"%s\"...\n", section->name);
	for (uint32_t patchID = 0; patchID < section->nbPatches; patchID++) {
		struct Patch *patch = &section->patches[patchID];
		int32_t value = computeRPNExpr(patch,
					       (struct Symbol const * const *)
							section->fileSymbols);
		uint16_t offset = patch->offset + section->offset;

		/* `jr` is quite unlike the others... */
		if (patch->type == PATCHTYPE_JR) {
			/* Target is relative to the byte *after* the operand */
			uint16_t address = patch->pcSection->org
							+ patch->pcOffset + 1;
			int16_t jumpOffset = value - address;

			if (jumpOffset < -128 || jumpOffset > 127)
				error("%s: jr target out of reach (expected -129 < %" PRId16 " < 128)",
				      patch->fileName, jumpOffset);
			dataSection->data[offset] = jumpOffset & 0xFF;
		} else {
			/* Patch a certain number of bytes */
			struct {
				uint8_t size;
				int32_t min;
				int32_t max;
			} const types[] = {
				[PATCHTYPE_BYTE] = {1,      -128,       255},
				[PATCHTYPE_WORD] = {2,    -32768,     65536},
				[PATCHTYPE_LONG] = {4, INT32_MIN, INT32_MAX}
			};

			if (value < types[patch->type].min
			 || value > types[patch->type].max)
				error("%s: Value %#" PRIx32 "%s is not %u-bit",
				      patch->fileName, value,
				      value < 0 ? " (maybe negative?)" : "",
				      types[patch->type].size * 8U);
			for (uint8_t i = 0; i < types[patch->type].size; i++) {
				dataSection->data[offset + i] = value & 0xFF;
				value >>= 8;
			}
		}
	}
}

/**
 * Applies all of a section's patches, iterating over "components" of
 * unionized sections
 * @param section The section to patch
 * @param arg Ignored callback arg
 */
static void applyPatches(struct Section *section, void *arg)
{
	(void)arg;
	struct Section *dataSection = section;

	do {
		applyFilePatches(section, dataSection);
		section = section->nextu;
	} while (section);
}

void patch_ApplyPatches(void)
{
	initRPNStack();
	sect_ForEach(applyPatches, NULL);
	freeRPNStack();
}

