MACRO N
FOR R,1,_NARG+1
PRINT STRSUB("\n\"\\ PR1NT,ABCDEFGHIMnOSU2_+-()<>",\<R>+1,1)
ENDR
REPT R-2
PRINT"\1,"
SHIFT
ENDR
PRINT"\1\n"
ENDM
 N 19,10,12,5,21,3,7,0,15,21,5,3,5,9,6,9,25,7,10,5,16,26,6,0,4,5,18,7,8,3,22,8,5,22,23,11,28,1,2,20,2,1,2,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,1,9,2,30,5,31,26,6,9,6,29,0,14,7,13,5,0,5,14,4,8,3,5,27,24,0,4,5,18,7,8,1,2,6,9,1,0,22,17,18,15,8,0,14,7,13,5,0,4,5,18,7,8,1,2,6,2,20,1,0,14,7,13,19,0,3,7,3
