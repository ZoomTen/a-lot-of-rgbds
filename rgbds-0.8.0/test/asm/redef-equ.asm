DEF n EQU 0
REDEF n EQU 1
; prints "$1"
PRINTLN n

MACRO list
DEF LIST_NAME EQUS "\1"
DEF LENGTH_{LIST_NAME} EQU 0
ENDM

MACRO item
REDEF LENGTH_{LIST_NAME} EQU LENGTH_{LIST_NAME} + 1
DEF {LIST_NAME}_{d:LENGTH_{LIST_NAME}} EQU \1
ENDM

	list SQUARES
	item 1
	item 4
	item 9
	println LENGTH_SQUARES, SQUARES_1, SQUARES_2, SQUARES_3

REDEF NEW EQU 7

DEF N EQUS "X"
REDEF N EQU 42