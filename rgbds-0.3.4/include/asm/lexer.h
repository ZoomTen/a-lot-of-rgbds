#ifndef RGBDS_ASM_LEXER_H
#define RGBDS_ASM_LEXER_H

#include <stdint.h>
#include <stdio.h>

#define LEXHASHSIZE	(1 << 11)
#define MAXSTRLEN	255

struct sLexInitString {
	char *tzName;
	uint32_t nToken;
};

struct sLexFloat {
	uint32_t (*Callback)(char *s, uint32_t size);
	uint32_t nToken;
};

struct yy_buffer_state {
	/* Actual starting address */
	char *pBufferRealStart;
	/* Address where the data is initially written after a safety margin */
	char *pBufferStart;
	char *pBuffer;
	uint32_t nBufferSize;
	uint32_t oAtLineStart;
};

enum eLexerState {
	LEX_STATE_NORMAL,
	LEX_STATE_MACROARGS
};

#define INITIAL		0
#define macroarg	3

typedef struct yy_buffer_state *YY_BUFFER_STATE;

void setup_lexer(void);

void yy_set_state(enum eLexerState i);
YY_BUFFER_STATE yy_create_buffer(FILE *f);
YY_BUFFER_STATE yy_scan_bytes(char *mem, uint32_t size);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void yy_switch_to_buffer(YY_BUFFER_STATE buf);
uint32_t lex_FloatAlloc(const struct sLexFloat *tok);
void lex_FloatAddRange(uint32_t id, uint16_t start, uint16_t end);
void lex_FloatDeleteRange(uint32_t id, uint16_t start, uint16_t end);
void lex_FloatAddFirstRange(uint32_t id, uint16_t start, uint16_t end);
void lex_FloatDeleteFirstRange(uint32_t id, uint16_t start, uint16_t end);
void lex_FloatAddSecondRange(uint32_t id, uint16_t start, uint16_t end);
void lex_FloatDeleteSecondRange(uint32_t id, uint16_t start, uint16_t end);
void lex_Init(void);
void lex_AddStrings(const struct sLexInitString *lex);
void lex_SetBuffer(char *buffer, uint32_t len);
int yywrap(void);
uint32_t yylex(void);
void yyunput(char c);
void yyunputstr(char *s);
void yyskipbytes(uint32_t count);
void yyunputbytes(uint32_t count);

extern YY_BUFFER_STATE pCurrentBuffer;

void upperstring(char *s);
void lowerstring(char *s);

#endif /* RGBDS_ASM_LEXER_H */
