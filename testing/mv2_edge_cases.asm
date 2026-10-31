; EDGE CASES DE LA MAQUINA VIRTUAL - VERSION 2
; ================================================================
; Basado en ASM MV1/MV2 2026, TP MV1/MV2 2026 y en la implementacion
; actual de main.c y operaciones.c.
;
; Ensamblar:
;   ./testing/vmt testing/mv2_edge_cases.asm /tmp/mv2_edge_cases.vmx -o v=2
;
; Ejecutar un caso (el ID se lee por entrada estandar):
;   printf '3\n' | ./main /tmp/mv2_edge_cases.vmx
;
; Para el caso 2 hay que crear Param Segment:
;   printf '2\n' | ./main /tmp/mv2_edge_cases.vmx -p hola
;
; Convencion de salida:
;   100 = caso seguro aprobado.
;   1xx = fallo; las dos ultimas cifras identifican la asercion.
;   Los casos 9..12 deben abortar y mostrar el error indicado.
;
; Casos:
;    1  entry point, segmentos y marco inicial de main sin parametros
;    2  Param Segment, argc, argv y acceso al segundo argumento
;    3  escrituras parciales y extension de signo de AL/AH/AX
;    4  b[]/w[]/l[], endianess y extension de signo
;    5  PUSH/POP: extension, truncamiento, LIFO y restauracion de SP
;    6  CALL/RET anidados y restauracion de SP
;    7  lectura b[] en el ultimo byte de DS (debe ser valida)
;    8  lectura w[] en los ultimos dos bytes de DS (debe ser valida)
;    9  POP con pila vacia: STACK UNDERFLOW
;   10  RET con pila vacia: STACK UNDERFLOW
;   11  PUSH sin cuatro bytes libres: STACK OVERFLOW
;   12  CALL sin cuatro bytes libres: STACK OVERFLOW
;   13  Const Segment y SYS 4 sobre string terminado en cero
;   14  RET de main: debe finalizar mediante la direccion 0xFFFFFFFF
;
; Los casos 7 y 8 son especialmente utiles para la VM actual: segun la
; especificacion, el modificador determina cuantos bytes se leen. Leer cuatro
; bytes antes de aplicar b[] o w[] produce un fallo de segmento espurio.
;
; Los casos 9..12 son destructivos para esa ejecucion. Deben correrse de a uno.

\\DATA 16
\\EXTRA 16
\\STACK 64

mensaje_ok equ "MV2 string OK\n"

; Si la VM ignora el entry point y comienza en el offset cero, termina aca.
trampa_entry: stop

main:   mov     eax, 0x01
        mov     edx, DS
        ldh     ecx, 4
        ldl     ecx, 1
        sys     1
        mov     efx, [DS]

        cmp     efx, 1
        jz      caso01
        cmp     efx, 2
        jz      caso02
        cmp     efx, 3
        jz      caso03
        cmp     efx, 4
        jz      caso04
        cmp     efx, 5
        jz      caso05
        cmp     efx, 6
        jz      caso06
        cmp     efx, 7
        jz      caso07
        cmp     efx, 8
        jz      caso08
        cmp     efx, 9
        jz      caso09
        cmp     efx, 10
        jz      caso10
        cmp     efx, 11
        jz      caso11
        cmp     efx, 12
        jz      caso12
        cmp     efx, 13
        jz      caso13
        cmp     efx, 14
        jz      caso14

        mov     eax, 199
        call    reportar
        ret

; ----------------------------------------------------------------
; 1 - ENTRY POINT, SEGMENTOS Y MARCO INICIAL
; Ejecutar sin -p. Todos los segmentos declarados deben existir, PS debe ser
; -1 y la pila inicial debe contener RET=-1, argc=0 y argv=-1. Con STACK=64,
; despues de esos tres valores SP queda a 52 bytes del comienzo de SS.
; ----------------------------------------------------------------
caso01: cmp     CS, -1
        jz      fallo101
        cmp     DS, -1
        jz      fallo102
        cmp     ES, -1
        jz      fallo103
        cmp     SS, -1
        jz      fallo104
        cmp     KS, -1
        jz      fallo105
        cmp     PS, -1
        jnz     fallo106
        cmp     [SP], -1
        jnz     fallo107
        cmp     [SP+4], 0
        jnz     fallo108
        cmp     [SP+8], -1
        jnz     fallo109
        mov     eax, SP
        sub     eax, SS
        cmp     eax, 52
        jnz     fallo110
        mov     eax, 100
        call    reportar
        ret

fallo101: mov eax, 101
        jmp     informar_y_salir
fallo102: mov eax, 102
        jmp     informar_y_salir
fallo103: mov eax, 103
        jmp     informar_y_salir
fallo104: mov eax, 104
        jmp     informar_y_salir
fallo105: mov eax, 105
        jmp     informar_y_salir
fallo106: mov eax, 106
        jmp     informar_y_salir
fallo107: mov eax, 107
        jmp     informar_y_salir
fallo108: mov eax, 108
        jmp     informar_y_salir
fallo109: mov eax, 109
        jmp     informar_y_salir
fallo110: mov eax, 110
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 2 - PARAM SEGMENT
; Ejecutar con "-p hola". La especificacion incluye el nombre del VMX en
; argv, por eso argc debe ser 2 y argv[1] debe apuntar a "hola".
; ----------------------------------------------------------------
caso02: cmp     PS, -1
        jz      fallo111
        cmp     [SP+4], 2
        jnz     fallo112
        mov     eex, [SP+8]
        cmp     eex, -1
        jz      fallo113
        mov     ebx, [eex+4]
        mov     eax, b[ebx]
        cmp     eax, 104
        jnz     fallo114
        mov     eax, 100
        call    reportar
        ret

fallo111: mov eax, 111
        jmp     informar_y_salir
fallo112: mov eax, 112
        jmp     informar_y_salir
fallo113: mov eax, 113
        jmp     informar_y_salir
fallo114: mov eax, 114
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 3 - SUBREGISTROS
; Las escrituras parciales preservan el resto del registro. Las lecturas de
; AL, AH y AX deben extender el signo a 32 bits.
; ----------------------------------------------------------------
caso03: mov     eax, 0x5678
        ldh     eax, 0x1234
        mov     al, 0x80
        mov     ecx, al
        cmp     ecx, -128
        jnz     fallo115
        mov     ah, -1
        mov     ecx, ah
        cmp     ecx, -1
        jnz     fallo116
        mov     ax, 0x8001
        mov     ecx, ax
        mov     ebx, 0x8001
        ldh     ebx, -1
        cmp     ecx, ebx
        jnz     fallo117
        mov     ebx, 0x8001
        ldh     ebx, 0x1234
        cmp     eax, ebx
        jnz     fallo118
        mov     eax, 100
        call    reportar
        ret

fallo115: mov eax, 115
        jmp     informar_y_salir
fallo116: mov eax, 116
        jmp     informar_y_salir
fallo117: mov eax, 117
        jmp     informar_y_salir
fallo118: mov eax, 118
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 4 - MODIFICADORES DE MEMORIA, BIG ENDIAN Y SIGNO
; ----------------------------------------------------------------
caso04: mov     b[DS], 0x80
        mov     eax, b[DS]
        cmp     eax, -128
        jnz     fallo119
        mov     w[DS+2], 0x8001
        mov     eax, w[DS+2]
        mov     ebx, 0x8001
        ldh     ebx, -1
        cmp     eax, ebx
        jnz     fallo120
        mov     eax, 0x5678
        ldh     eax, 0x1234
        mov     l[DS+4], eax
        mov     ebx, b[DS+4]
        cmp     ebx, 0x12
        jnz     fallo121
        mov     ebx, b[DS+5]
        cmp     ebx, 0x34
        jnz     fallo122
        mov     ebx, w[DS+6]
        cmp     ebx, 0x5678
        jnz     fallo123
        mov     eax, 100
        call    reportar
        ret

fallo119: mov eax, 119
        jmp     informar_y_salir
fallo120: mov eax, 120
        jmp     informar_y_salir
fallo121: mov eax, 121
        jmp     informar_y_salir
fallo122: mov eax, 122
        jmp     informar_y_salir
fallo123: mov eax, 123
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 5 - PUSH/POP
; PUSH AX expande signo a cuatro bytes. POP hacia w[] y b[] trunca los bytes
; mas significativos. Al final SP debe recuperar exactamente su valor inicial.
; ----------------------------------------------------------------
caso05: mov     efx, SP
        mov     eax, 0x8001
        ldh     eax, 0x1234
        push    ax
        pop     ebx
        mov     ecx, 0x8001
        ldh     ecx, -1
        cmp     ebx, ecx
        jnz     fallo124
        push    eax
        pop     w[ES]
        mov     ebx, w[ES]
        cmp     ebx, ecx
        jnz     fallo125
        push    -128
        pop     b[ES+2]
        mov     ebx, b[ES+2]
        cmp     ebx, -128
        jnz     fallo126
        cmp     SP, efx
        jnz     fallo127
        mov     eax, 100
        call    reportar
        ret

fallo124: mov eax, 124
        jmp     informar_y_salir
fallo125: mov eax, 125
        jmp     informar_y_salir
fallo126: mov eax, 126
        jmp     informar_y_salir
fallo127: mov eax, 127
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 6 - CALL/RET ANIDADOS
; ----------------------------------------------------------------
caso06: mov     efx, SP
        mov     eex, 0
        call    subrutina_1
        cmp     eex, 3
        jnz     fallo128
        cmp     SP, efx
        jnz     fallo129
        mov     eax, 100
        call    reportar
        ret

subrutina_1: add     eex, 1
        call    subrutina_2
        add     eex, 1
        ret

subrutina_2: add     eex, 1
        ret

fallo128: mov eax, 128
        jmp     informar_y_salir
fallo129: mov eax, 129
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 7/8 - ACCESOS VALIDOS EN EL BORDE SUPERIOR DE DATA
; ----------------------------------------------------------------
caso07: mov     b[DS+15], 0x7f
        mov     eax, b[DS+15]
        cmp     eax, 0x7f
        jnz     fallo130
        mov     eax, 100
        call    reportar
        ret

fallo130: mov eax, 130
        jmp     informar_y_salir

caso08: mov     w[DS+14], 0x7f01
        mov     eax, w[DS+14]
        cmp     eax, 0x7f01
        jnz     fallo131
        mov     eax, 100
        call    reportar
        ret

fallo131: mov eax, 131
        jmp     informar_y_salir

; ----------------------------------------------------------------
; 9/10 - STACK UNDERFLOW
; Se descarta deliberadamente el marco inicial para dejar SP en SS+64.
; ----------------------------------------------------------------
caso09: mov     SP, SS
        add     SP, 64
        pop     eax
        mov     eax, 132
        call    reportar
        ret

caso10: mov     SP, SS
        add     SP, 64
        ret

; ----------------------------------------------------------------
; 11/12 - STACK OVERFLOW
; En el caso 11, con SP=SS+4 cabe un PUSH y el siguiente debe fallar. En el
; caso 12, SP=SS deja a CALL sin los cuatro bytes que necesita para el retorno.
; ----------------------------------------------------------------
caso11: mov     SP, SS
        add     SP, 4
        push    1
        push    2
        stop

caso12: mov     SP, SS
        call    subrutina_2
        stop

; ----------------------------------------------------------------
; 13 - CONST SEGMENT Y STRING WRITE
; Debe imprimir "MV2 string OK" y despues el codigo 100.
; ----------------------------------------------------------------
caso13: mov     edx, KS
        add     edx, mensaje_ok
        sys     4
        mov     eax, 100
        call    reportar
        ret

; ----------------------------------------------------------------
; 14 - RET DE MAIN
; No imprime nada: aprobar significa finalizar normalmente al cargar IP=-1.
; ----------------------------------------------------------------
caso14: ret

; EAX contiene el codigo. SYS 2 lo imprime en decimal desde DS+4.
informar_y_salir: call    reportar
        ret

reportar: mov     [DS+4], eax
        mov     eax, 0x01
        mov     edx, DS
        add     edx, 4
        ldh     ecx, 4
        ldl     ecx, 1
        sys     2
        ret
