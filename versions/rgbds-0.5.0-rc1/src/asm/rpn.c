/*
 * This file is part of RGBDS.
 *
 * Copyright (c) 1997-2018, Carsten Sorensen and RGBDS contributors.
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * Controls RPN expressions for objectfiles
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm/main.h"
#include "asm/output.h"
#include "asm/rpn.h"
#include "asm/section.h"
#include "asm/symbol.h"
#include "asm/warning.h"

#include "opmath.h"

/* Makes an expression "not known", also setting its error message */
#define makeUnknown(expr_, ...) do { \
	struct Expression *_expr = expr_; \
	_expr->isKnown = false; \
	/* If we had `asprintf` this would be great, but alas. */ \
	_expr->reason = malloc(128); /* Use an initial reasonable size */ \
	if (!_expr->reason) \
		fatalerror("Can't allocate err string: %s\n", strerror(errno)); \
	int size = snprintf(_expr->reason, 128, __VA_ARGS__); \
	if (size >= 128) { /* If this wasn't enough, try again */ \
		_expr->reason = realloc(_expr->reason, size + 1); \
		sprintf(_expr->reason, __VA_ARGS__); \
	} \
} while (0)

static uint8_t *reserveSpace(struct Expression *expr, uint32_t size)
{
	/* This assumes the RPN length is always less than the capacity */
	if (expr->nRPNCapacity - expr->nRPNLength < size) {
		/* If there isn't enough room to reserve the space, realloc */
		if (!expr->tRPN)
			expr->nRPNCapacity = 256; /* Initial size */
		while (expr->nRPNCapacity - expr->nRPNLength < size) {
			if (expr->nRPNCapacity >= MAXRPNLEN)
				/*
				 * To avoid generating humongous object files, cap the
				 * size of RPN expressions
				 */
				fatalerror("RPN expression cannot grow larger than "
					   EXPAND_AND_STR(MAXRPNLEN) " bytes\n");
			else if (expr->nRPNCapacity > MAXRPNLEN / 2)
				expr->nRPNCapacity = MAXRPNLEN;
			else
				expr->nRPNCapacity *= 2;
		}
		expr->tRPN = realloc(expr->tRPN, expr->nRPNCapacity);

		if (!expr->tRPN)
			fatalerror("Failed to grow RPN expression: %s\n", strerror(errno));
	}

	uint8_t *ptr = expr->tRPN + expr->nRPNLength;

	expr->nRPNLength += size;
	return ptr;
}

/*
 * Init the RPN expression
 */
static void rpn_Init(struct Expression *expr)
{
	expr->reason = NULL;
	expr->isKnown = true;
	expr->isSymbol = false;
	expr->tRPN = NULL;
	expr->nRPNCapacity = 0;
	expr->nRPNLength = 0;
	expr->nRPNPatchSize = 0;
}

/*
 * Free the RPN expression
 */
void rpn_Free(struct Expression *expr)
{
	free(expr->tRPN);
	free(expr->reason);
	rpn_Init(expr);
}

/*
 * Add symbols, constants and operators to expression
 */
void rpn_Number(struct Expression *expr, uint32_t i)
{
	rpn_Init(expr);
	expr->nVal = i;
}

void rpn_Symbol(struct Expression *expr, char const *tzSym)
{
	struct Symbol *sym = sym_FindScopedSymbol(tzSym);

	if (sym_IsPC(sym) && !sect_GetSymbolSection()) {
		error("PC has no value outside a section\n");
		rpn_Number(expr, 0);
	} else if (!sym || !sym_IsConstant(sym)) {
		rpn_Init(expr);
		expr->isSymbol = true;

		makeUnknown(expr, sym_IsPC(sym) ? "PC is not constant at assembly time"
						: "'%s' is not constant at assembly time", tzSym);
		sym = sym_Ref(tzSym);
		expr->nRPNPatchSize += 5; /* 1-byte opcode + 4-byte symbol ID */

		size_t nameLen = strlen(sym->name) + 1; /* Don't forget NUL! */
		uint8_t *ptr = reserveSpace(expr, nameLen + 1);
		*ptr++ = RPN_SYM;
		memcpy(ptr, sym->name, nameLen);
	} else {
		rpn_Number(expr, sym_GetConstantValue(tzSym));
	}
}

void rpn_BankSelf(struct Expression *expr)
{
	rpn_Init(expr);

	if (!pCurrentSection) {
		error("PC has no bank outside a section\n");
		expr->nVal = 1;
	} else if (pCurrentSection->bank == -1) {
		makeUnknown(expr, "Current section's bank is not known");
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_BANK_SELF;
	} else {
		expr->nVal = pCurrentSection->bank;
	}
}

void rpn_BankSymbol(struct Expression *expr, char const *tzSym)
{
	struct Symbol const *sym = sym_FindScopedSymbol(tzSym);

	/* The @ symbol is treated differently. */
	if (sym_IsPC(sym)) {
		rpn_BankSelf(expr);
		return;
	}

	rpn_Init(expr);
	if (sym && !sym_IsLabel(sym)) {
		error("BANK argument must be a label\n");
	} else {
		sym = sym_Ref(tzSym);
		assert(sym); // If the symbol didn't exist, it should have been created

		if (sym_GetSection(sym) && sym_GetSection(sym)->bank != -1) {
			/* Symbol's section is known and bank is fixed */
			expr->nVal = sym_GetSection(sym)->bank;
		} else {
			makeUnknown(expr, "\"%s\"'s bank is not known", tzSym);
			expr->nRPNPatchSize += 5; /* opcode + 4-byte sect ID */

			size_t nameLen = strlen(sym->name) + 1; /* Room for NUL! */
			uint8_t *ptr = reserveSpace(expr, nameLen + 1);
			*ptr++ = RPN_BANK_SYM;
			memcpy(ptr, sym->name, nameLen);
		}
	}
}

void rpn_BankSection(struct Expression *expr, char const *tzSectionName)
{
	rpn_Init(expr);

	struct Section *pSection = out_FindSectionByName(tzSectionName);

	if (pSection && pSection->bank != -1) {
		expr->nVal = pSection->bank;
	} else {
		makeUnknown(expr, "Section \"%s\"'s bank is not known",
			    tzSectionName);

		size_t nameLen = strlen(tzSectionName) + 1; /* Room for NUL! */
		uint8_t *ptr = reserveSpace(expr, nameLen + 1);

		expr->nRPNPatchSize += nameLen + 1;
		*ptr++ = RPN_BANK_SECT;
		memcpy(ptr, tzSectionName, nameLen);
	}
}

void rpn_CheckHRAM(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (!rpn_isKnown(expr)) {
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_HRAM;
	} else if (expr->nVal >= 0xFF00 && expr->nVal <= 0xFFFF) {
		/* That range is valid, but only keep the lower byte */
		expr->nVal &= 0xFF;
	} else if (expr->nVal < 0 || expr->nVal > 0xFF) {
		error("Source address $%" PRIx32 " not between $FF00 to $FFFF\n", expr->nVal);
	}
}

void rpn_CheckRST(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;

	if (rpn_isKnown(expr)) {
		/* A valid RST address must be masked with 0x38 */
		if (expr->nVal & ~0x38)
			error("Invalid address $%" PRIx32 " for RST\n", expr->nVal);
		/* The target is in the "0x38" bits, all other bits are set */
		expr->nVal |= 0xC7;
	} else {
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_RST;
	}
}

void rpn_LOGNOT(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (rpn_isKnown(expr)) {
		expr->nVal = !expr->nVal;
	} else {
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_LOGUNNOT;
	}
}

struct Symbol const *rpn_SymbolOf(struct Expression const *expr)
{
	if (!rpn_isSymbol(expr))
		return NULL;
	return sym_FindScopedSymbol((char *)expr->tRPN + 1);
}

bool rpn_IsDiffConstant(struct Expression const *src, struct Symbol const *sym)
{
	/* Check if both expressions only refer to a single symbol */
	struct Symbol const *sym1 = rpn_SymbolOf(src);

	if (!sym1 || !sym || sym1->type != SYM_LABEL || sym->type != SYM_LABEL)
		return false;

	struct Section const *section1 = sym_GetSection(sym1);
	struct Section const *section2 = sym_GetSection(sym);
	return section1 && (section1 == section2);
}

static bool isDiffConstant(struct Expression const *src1,
			   struct Expression const *src2)
{
	return rpn_IsDiffConstant(src1, rpn_SymbolOf(src2));
}

void rpn_BinaryOp(enum RPNCommand op, struct Expression *expr,
		  const struct Expression *src1, const struct Expression *src2)
{
	expr->isSymbol = false;

	/* First, check if the expression is known */
	expr->isKnown = src1->isKnown && src2->isKnown;
	if (expr->isKnown) {
		rpn_Init(expr); /* Init the expression to something sane */

		/* If both expressions are known, just compute the value */
		uint32_t uleft = src1->nVal, uright = src2->nVal;

		switch (op) {
		case RPN_LOGOR:
			expr->nVal = src1->nVal || src2->nVal;
			break;
		case RPN_LOGAND:
			expr->nVal = src1->nVal && src2->nVal;
			break;
		case RPN_LOGEQ:
			expr->nVal = src1->nVal == src2->nVal;
			break;
		case RPN_LOGGT:
			expr->nVal = src1->nVal > src2->nVal;
			break;
		case RPN_LOGLT:
			expr->nVal = src1->nVal < src2->nVal;
			break;
		case RPN_LOGGE:
			expr->nVal = src1->nVal >= src2->nVal;
			break;
		case RPN_LOGLE:
			expr->nVal = src1->nVal <= src2->nVal;
			break;
		case RPN_LOGNE:
			expr->nVal = src1->nVal != src2->nVal;
			break;
		case RPN_ADD:
			expr->nVal = uleft + uright;
			break;
		case RPN_SUB:
			expr->nVal = uleft - uright;
			break;
		case RPN_XOR:
			expr->nVal = src1->nVal ^ src2->nVal;
			break;
		case RPN_OR:
			expr->nVal = src1->nVal | src2->nVal;
			break;
		case RPN_AND:
			expr->nVal = src1->nVal & src2->nVal;
			break;
		case RPN_SHL:
			if (src2->nVal < 0)
				warning(WARNING_SHIFT_AMOUNT,
					"Shifting left by negative amount %" PRId32 "\n",
					src2->nVal);

			if (src2->nVal >= 32)
				warning(WARNING_SHIFT_AMOUNT,
					"Shifting left by large amount %" PRId32 "\n",
					src2->nVal);

			expr->nVal = op_shift_left(src1->nVal, src2->nVal);
			break;
		case RPN_SHR:
			if (src1->nVal < 0)
				warning(WARNING_SHIFT, "Shifting right negative value %"
					PRId32 "\n",
					src1->nVal);

			if (src2->nVal < 0)
				warning(WARNING_SHIFT_AMOUNT,
					"Shifting right by negative amount %" PRId32 "\n",
					src2->nVal);

			if (src2->nVal >= 32)
				warning(WARNING_SHIFT_AMOUNT,
					"Shifting right by large amount %" PRId32 "\n",
					src2->nVal);

			expr->nVal = op_shift_right(src1->nVal, src2->nVal);
			break;
		case RPN_MUL:
			expr->nVal = uleft * uright;
			break;
		case RPN_DIV:
			if (src2->nVal == 0)
				fatalerror("Division by zero\n");

			if (src1->nVal == INT32_MIN && src2->nVal == -1) {
				warning(WARNING_DIV, "Division of %" PRId32 " by -1 yields %"
					PRId32 "\n", INT32_MIN, INT32_MIN);
				expr->nVal = INT32_MIN;
			} else {
				expr->nVal = op_divide(src1->nVal, src2->nVal);
			}
			break;
		case RPN_MOD:
			if (src2->nVal == 0)
				fatalerror("Modulo by zero\n");

			if (src1->nVal == INT32_MIN && src2->nVal == -1)
				expr->nVal = 0;
			else
				expr->nVal = op_modulo(src1->nVal, src2->nVal);
			break;
		case RPN_EXP:
			if (src2->nVal < 0)
				fatalerror("Exponentiation by negative power\n");

			if (src1->nVal == INT32_MIN && src2->nVal == -1)
				expr->nVal = 0;
			else
				expr->nVal = op_exponent(src1->nVal, src2->nVal);
			break;

		case RPN_UNSUB:
		case RPN_UNNOT:
		case RPN_LOGUNNOT:
		case RPN_BANK_SYM:
		case RPN_BANK_SECT:
		case RPN_BANK_SELF:
		case RPN_HRAM:
		case RPN_RST:
		case RPN_CONST:
		case RPN_SYM:
			fatalerror("%d is not a binary operator\n", op);
		}

	} else if (op == RPN_SUB && isDiffConstant(src1, src2)) {
		struct Symbol const *symbol1 = rpn_SymbolOf(src1);
		struct Symbol const *symbol2 = rpn_SymbolOf(src2);

		expr->nVal = sym_GetValue(symbol1) - sym_GetValue(symbol2);
		expr->isKnown = true;
	} else {
		/* If it's not known, start computing the RPN expression */

		/* Convert the left-hand expression if it's constant */
		if (src1->isKnown) {
			uint32_t lval = src1->nVal;
			uint8_t bytes[] = {RPN_CONST, lval, lval >> 8,
					   lval >> 16, lval >> 24};
			expr->nRPNPatchSize = sizeof(bytes);
			expr->tRPN = NULL;
			expr->nRPNCapacity = 0;
			expr->nRPNLength = 0;
			memcpy(reserveSpace(expr, sizeof(bytes)), bytes,
			       sizeof(bytes));

			/* Use the other expression's un-const reason */
			expr->reason = src2->reason;
			free(src1->reason);
		} else {
			/* Otherwise just reuse its RPN buffer */
			expr->nRPNPatchSize = src1->nRPNPatchSize;
			expr->tRPN = src1->tRPN;
			expr->nRPNCapacity = src1->nRPNCapacity;
			expr->nRPNLength = src1->nRPNLength;
			expr->reason = src1->reason;
			free(src2->reason);
		}

		/* Now, merge the right expression into the left one */
		uint8_t *ptr = src2->tRPN; /* Pointer to the right RPN */
		uint32_t len = src2->nRPNLength; /* Size of the right RPN */
		uint32_t patchSize = src2->nRPNPatchSize;

		/* If the right expression is constant, merge a shim instead */
		uint32_t rval = src2->nVal;
		uint8_t bytes[] = {RPN_CONST, rval, rval >> 8, rval >> 16,
				   rval >> 24};
		if (src2->isKnown) {
			ptr = bytes;
			len = sizeof(bytes);
			patchSize = sizeof(bytes);
		}
		/* Copy the right RPN and append the operator */
		uint8_t *buf = reserveSpace(expr, len + 1);

		memcpy(buf, ptr, len);
		buf[len] = op;

		free(src2->tRPN); /* If there was none, this is `free(NULL)` */
		expr->nRPNPatchSize += patchSize + 1;
	}
}

void rpn_HIGH(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (rpn_isKnown(expr)) {
		expr->nVal = (uint32_t)expr->nVal >> 8 & 0xFF;
	} else {
		uint8_t bytes[] = {RPN_CONST,    8, 0, 0, 0, RPN_SHR,
				   RPN_CONST, 0xFF, 0, 0, 0, RPN_AND};
		expr->nRPNPatchSize += sizeof(bytes);
		memcpy(reserveSpace(expr, sizeof(bytes)), bytes, sizeof(bytes));
	}
}

void rpn_LOW(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (rpn_isKnown(expr)) {
		expr->nVal = expr->nVal & 0xFF;
	} else {
		uint8_t bytes[] = {RPN_CONST, 0xFF, 0, 0, 0, RPN_AND};

		expr->nRPNPatchSize += sizeof(bytes);
		memcpy(reserveSpace(expr, sizeof(bytes)), bytes, sizeof(bytes));
	}
}

void rpn_ISCONST(struct Expression *expr, const struct Expression *src)
{
	rpn_Init(expr);
	expr->nVal = rpn_isKnown(src);
	expr->isKnown = true;
	expr->isSymbol = false;
}

void rpn_UNNEG(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (rpn_isKnown(expr)) {
		expr->nVal = -(uint32_t)expr->nVal;
	} else {
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_UNSUB;
	}
}

void rpn_UNNOT(struct Expression *expr, const struct Expression *src)
{
	*expr = *src;
	expr->isSymbol = false;

	if (rpn_isKnown(expr)) {
		expr->nVal = ~expr->nVal;
	} else {
		expr->nRPNPatchSize++;
		*reserveSpace(expr, 1) = RPN_UNNOT;
	}
}
