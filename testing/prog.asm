nombre equ "VMX26"
abc equ "abc\b\n\r"
nose equ "Fund. de la Arq. de Computadoras"
SHL EL,1
MOV b[DS+0],AH
MOV CX,w[EDX+0]
MOV w[DS+2],l[DS+10]