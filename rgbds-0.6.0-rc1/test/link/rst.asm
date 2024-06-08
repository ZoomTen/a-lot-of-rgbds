
SECTION "calls", ROM0[0]

; The values are not known at this point, forcing the assembler to emit an
; expression
	rst rst00
	rst rst08
	rst rst10
	rst rst18
	rst rst20
	rst rst28
	rst rst30
	rst rst38


defRST: MACRO
	SECTION "rst\1", ROM0[$\1]
	rst\1:
ENDM
	defRST 00
	defRST 08
	defRST 10
	defRST 18
	defRST 20
	defRST 28
	defRST 30
	defRST 38
