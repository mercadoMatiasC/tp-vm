; VM EDGE CASES - PROGRAMA UNICO DE PRUEBA
; =========================================
;
; Este archivo fue disenado leyendo main.c y operaciones.c.
; No modifica la VM y no evalua al ensamblador.
;
; USO
; ----
; 1. Ensamblar este archivo una sola vez.
; 2. Ejecutar el VMX resultante.
; 3. Ingresar primero el ID decimal del escenario.
; 4. Para los casos 9..13, 106 y 107 ingresar tambien el dato indicado.
;
; Cada escenario termina con STOP, exit, una senal o un salto fuera de CS.
; Por eso se ejecuta un ID por proceso, siempre usando el mismo VMX.
;
; CASOS FUNCIONALES
;   1  ADD, SUB, MUL, CMP y flags de borde
;   2  NOT, AND, OR, XOR, LDL y LDH
;   3  DIV valida, resto y division de un negativo tratada como unsigned
;   4  SWAP normal y SWAP con alias
;   5  SHL, SHR y SAR con cantidades validas
;   6  JMP y todos los saltos condicionales, tomados y no tomados
;   7  operandos de memoria como origen y destino
;   8  SYS WRITE con todos los formatos y dos celdas
;   9  SYS READ decimal; segundo dato sugerido: -2147483648
;  10  SYS READ hexadecimal; segundo dato sugerido: FFFFFFFF
;  11  SYS READ octal; segundo dato sugerido: 37777777777
;  12  SYS READ binario; segundo dato sugerido: 10000000000000000000000000000001
;  13  SYS READ caracter; segundo dato sugerido: Z
;  14  RND valido y preservacion de CC
;  15  lectura de OPC, CS, DS y MBR
;  16  escritura manual de CC y condicion extrema de JNN
;
; CASOS PELIGROSOS - EJECUTAR AISLADOS Y CON LIMITES
;  90  division por cero
;  91  SHL por 32
;  92  SHR por 32
;  93  SAR por 32
;  94  SHL por 64
;  95  RND con limite 0
;  96  RND con limite -1; puede provocar division por cero del host
;  97  escritura del registro interno OP1
;  98  direccion logica anterior al comienzo de CS
;  99  escritura de IP con -1
; 100  escritura de DS y automodificacion del comienzo de CS
; 101  offset de memoria negativo que puede acarrear al selector siguiente
; 102  acceso que excede el final fisico de la RAM
; 103  SYS con 256 celdas; revela truncamiento a 8 bits
; 104  SYS con ancho de celda 5
; 105  numero de servicio SYS desconocido
; 106  READ decimal invalido; segundo dato sugerido: texto
; 107  READ binario demasiado largo; segundo dato sugerido: 64 unos
; 108  STOP seguido por una instruccion que debe quedar inalcanzable
;
; OBSERVACION
; -----------
; La VM actual imprime una traza despues de cada instruccion. Para los flags,
; leer la linea CC inmediatamente posterior a la operacion bajo prueba: las
; instrucciones usadas para guardar e imprimir resultados vuelven a cambiar CC.
;
; Un ASM valido no puede probar cabeceras VMX truncadas, tamanos de codigo
; falsos, opcodes 0B..0E ni instrucciones cortadas. Esos riesgos se enumeran
; en el analisis, pero requieren construir o editar un VMX a mano.

inicio: mov eax, 0b00001
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov efx, [DS]

cmp efx, 1
jz caso01
cmp efx, 2
jz caso02
cmp efx, 3
jz caso03
cmp efx, 4
jz caso04
cmp efx, 5
jz caso05
cmp efx, 6
jz caso06
cmp efx, 7
jz caso07
cmp efx, 8
jz caso08
cmp efx, 9
jz caso09
cmp efx, 10
jz caso10
cmp efx, 11
jz caso11
cmp efx, 12
jz caso12
cmp efx, 13
jz caso13
cmp efx, 14
jz caso14
cmp efx, 15
jz caso15
cmp efx, 16
jz caso16

cmp efx, 90
jz caso90
cmp efx, 91
jz caso91
cmp efx, 92
jz caso92
cmp efx, 93
jz caso93
cmp efx, 94
jz caso94
cmp efx, 95
jz caso95
cmp efx, 96
jz caso96
cmp efx, 97
jz caso97
cmp efx, 98
jz caso98
cmp efx, 99
jz caso99
cmp efx, 100
jz caso100
cmp efx, 101
jz caso101
cmp efx, 102
jz caso102
cmp efx, 103
jz caso103
cmp efx, 104
jz caso104
cmp efx, 105
jz caso105
cmp efx, 106
jz caso106
cmp efx, 107
jz caso107
cmp efx, 108
jz caso108
jmp selector_invalido

; -----------------------------------------------------------------------------
; ID 1 - ARITMETICA Y FLAGS
; Salida hexadecimal esperada:
;   0x80000000  resultado de INT32_MAX + 1, NZCV=1001
;   0xFFFFFFFB  resultado de 5 - 10, NZCV=1010
;   0xFFFFFFFE  resultado de 0xFFFFFFFF * 2, NZCV=1011
; Tambien deja en la traza los casos cero+carry y CMP sin escritura.
; -----------------------------------------------------------------------------
caso01: mov eax, 0
add eax, 0
mov eax, -1
add eax, 1
mov eax, -1
ldh eax, 0x7fff
add eax, 1
mov ebx, 5
sub ebx, 10
cmp ebx, -5
mov eex, -1
mul eex, 2
mov [DS], eax
mov [DS+4], ebx
mov [DS+8], eex
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 3
sys 2
stop

; -----------------------------------------------------------------------------
; ID 2 - LOGICA Y CARGAS DE MEDIA PALABRA
; Salida esperada: 0xFFFFF608 y 0x11225566.
; -----------------------------------------------------------------------------
caso02: mov eax, 0x5678
ldh eax, 0x1234
not eax
and eax, 0x0f0f
or eax, 0x00f0
xor eax, -1
mov ecx, 0x3344
ldh ecx, 0x1122
ldl ecx, 0x5566
mov [DS], eax
mov [DS+4], ecx
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 2
sys 2
stop

; -----------------------------------------------------------------------------
; ID 3 - DIVISION VALIDA, RESTO Y SIGNO
; Salida esperada:
;   0x2, 0x1 para 7/3
;   0x7FFFFFFE, 0x0 para -4/2 con la implementacion unsigned actual
; -----------------------------------------------------------------------------
caso03: mov eax, 7
div eax, 3
mov [DS], eax
mov [DS+4], AC
mov eex, -4
div eex, 2
mov [DS+8], eex
mov [DS+12], AC
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 4
sys 2
stop

; -----------------------------------------------------------------------------
; ID 4 - SWAP NORMAL Y CON ALIAS
; Salida deseable: 0x2222, 0x1111, 0x3333.
; La implementacion XOR-swap actual probablemente imprime 0 como tercer valor.
; -----------------------------------------------------------------------------
caso04: mov eax, 0x1111
mov ebx, 0x2222
swap eax, ebx
mov ecx, 0x3333
swap ecx, ecx
mov [DS], eax
mov [DS+4], ebx
mov [DS+8], ecx
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 3
sys 2
stop

; -----------------------------------------------------------------------------
; ID 5 - SHIFTS VALIDOS
; Salida esperada: 0x2, 0x40000000, 0xC0000000 y 0xC0000000.
; Revisar tambien C en las trazas posteriores a SHL/SHR/SAR.
; -----------------------------------------------------------------------------
caso05: mov eax, 1
ldh eax, 0x8000
shl eax, 1
mov eex, eax
mov eax, 1
ldh eax, 0x8000
shr eax, 1
mov efx, eax
mov eax, 1
ldh eax, 0x8000
sar eax, 1
mov ebx, eax
sar eax, 0
mov [DS], eex
mov [DS+4], efx
mov [DS+8], ebx
mov [DS+12], eax
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 4
sys 2
stop

; -----------------------------------------------------------------------------
; ID 6 - SALTOS
; EEX contiene el subcaso que fallo. La salida decimal 600 significa que todos
; los saltos tomados/no tomados siguieron el flujo previsto.
; -----------------------------------------------------------------------------
caso06: mov eex, 611
mov eax, 1
jp b_jp_ok
jmp b_fin
b_jp_ok: mov eex, 612
mov eax, -1
jp b_fin
mov eex, 613
cmp eax, eax
jp b_fin

mov eex, 621
mov eax, -1
jn b_jn_ok
jmp b_fin
b_jn_ok: mov eex, 622
mov eax, 1
jn b_fin

mov eex, 631
cmp eax, eax
jz b_jz_ok
jmp b_fin
b_jz_ok: mov eex, 632
mov eax, 1
jz b_fin

mov eex, 641
mov eax, -1
add eax, 2
jc b_jc_ok
jmp b_fin
b_jc_ok: mov eex, 642
mov eax, 1
jc b_fin

mov eex, 651
mov eax, -1
ldh eax, 0x7fff
add eax, 1
jv b_jv_ok
jmp b_fin
b_jv_ok: mov eex, 652
mov eax, 1
jv b_fin

mov eex, 661
mov eax, -1
jnp b_jnp_n_ok
jmp b_fin
b_jnp_n_ok: mov eex, 662
cmp eax, eax
jnp b_jnp_z_ok
jmp b_fin
b_jnp_z_ok: mov eex, 663
mov eax, 1
jnp b_fin

mov eex, 671
mov eax, 1
jnn b_jnn_ok
jmp b_fin
b_jnn_ok: mov eex, 672
mov eax, -1
jnn b_fin

mov eex, 681
mov eax, 1
jnz b_jnz_ok
jmp b_fin
b_jnz_ok: mov eex, 682
cmp eax, eax
jnz b_fin

mov eex, 699
jmp b_jmp_ok
jmp b_fin
b_jmp_ok: mov eex, 600
b_fin: mov [DS], eex
mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

; -----------------------------------------------------------------------------
; ID 7 - MEMORIA COMO ORIGEN Y DESTINO
; Salida esperada: dos veces 0xEDCBA986.
; -----------------------------------------------------------------------------
caso07: mov ebx, 0x5678
ldh ebx, 0x1234
mov [DS], ebx
mov eax, [DS]
add [DS], 1
not [DS]
mov ebx, [DS]
mov [DS+4], ebx
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 2
sys 2
stop

; -----------------------------------------------------------------------------
; ID 8 - SYS WRITE CON TODOS LOS FORMATOS
; EAX=0x1F solicita binario, hexa, octal, caracter y decimal simultaneamente.
; Para 4 bytes, el helper binario actual intenta desplazar por 32: revisar UB.
; Caracteres esperados: AB.. y 01...
; -----------------------------------------------------------------------------
caso08: mov ebx, 0x7f80
ldh ebx, 0x4142
mov [DS], ebx
mov ebx, 0x2e7f
ldh ebx, 0x3031
mov [DS+4], ebx
mov eax, 0x1f
mov edx, DS
ldh ecx, 4
ldl ecx, 2
sys 2
stop

; -----------------------------------------------------------------------------
; IDs 9..13 - SYS READ
; Cada caso necesita un segundo dato despues del selector.
; Todos muestran en hexadecimal lo que finalmente quedo en memoria.
; -----------------------------------------------------------------------------
caso09: mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

caso10: mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

caso11: mov eax, 0x04
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

caso12: mov eax, 0x10
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

caso13: mov eax, 0x02
mov edx, DS
ldh ecx, 1
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

; -----------------------------------------------------------------------------
; ID 14 - RND VALIDO
; Cada valor debe estar entre 0 y 10 inclusive. La linea CC posterior a cada
; RND deberia conservar 0100, establecido por el CMP inmediatamente anterior.
; -----------------------------------------------------------------------------
caso14: mov eax, 0
cmp eax, 0
rnd ebx, 10
mov [DS], ebx
cmp eax, 0
rnd ebx, 10
mov [DS+4], ebx
cmp eax, 0
rnd ebx, 10
mov [DS+8], ebx
mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 3
sys 2
stop

; -----------------------------------------------------------------------------
; ID 15 - REGISTROS INTERNOS
; Salida prevista: OPC=0x10, CS=0, DS=0x10000 y MBR=0x12345678.
; -----------------------------------------------------------------------------
caso15: mov ebx, OPC
mov ecx, CS
mov eex, DS
mov eax, 0x5678
ldh eax, 0x1234
mov [DS], eax
mov efx, MBR
mov [DS], ebx
mov [DS+4], ecx
mov [DS+8], eex
mov [DS+12], efx
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 4
sys 2
stop

; -----------------------------------------------------------------------------
; ID 16 - CC ESCRIBIBLE Y JNN
; Se fuerza la combinacion N=1,Z=1. El codigo actual evalua JNN como !N || Z,
; por lo que salta y muestra 1600. Una definicion estricta !N mostraria 1601.
; -----------------------------------------------------------------------------
caso16: mov eex, 1601
ldh CC, 0xc000
jnn c16_tomado
jmp c16_fin
c16_tomado: mov eex, 1600
c16_fin: mov [DS], eex
mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

; -----------------------------------------------------------------------------
; ID 90 - DIVISION POR CERO
; La implementacion actual imprime error pero continua. Si continua, deberia
; mostrar EAX=0x7 y AC=0x9, ambos sin modificar.
; -----------------------------------------------------------------------------
caso90: mov AC, 9
mov eax, 7
div eax, 0
mov [DS], eax
mov [DS+4], AC
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 2
sys 2
stop

; -----------------------------------------------------------------------------
; IDs 91..94 - CANTIDADES DE SHIFT EN EL LIMITE O FUERA DE RANGO
; SHR/SAR por 32 y SHL por 64 alcanzan expresiones indefinidas en C.
; No asumir un resultado correcto aunque una plataforma imprima uno estable.
; -----------------------------------------------------------------------------
caso91: mov eax, 1
shl eax, 32
mov [DS], eax
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

caso92: mov eax, -1
shr eax, 32
mov [DS], eax
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

caso93: mov eax, -1
sar eax, 32
mov [DS], eax
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

caso94: mov eax, 1
shl eax, 64
mov [DS], eax
mov eax, 0x08
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop

; -----------------------------------------------------------------------------
; IDs 95..96 - RND INVALIDO
; RND -1 evalua rand()%0 antes de validar el argumento.
; -----------------------------------------------------------------------------
caso95: rnd eax, 0
stop

caso96: rnd eax, -1
stop

; -----------------------------------------------------------------------------
; ID 97 - ESCRITURA DE OP1
; MOV reemplaza el descriptor de su propio operando antes del desensamblado.
; -----------------------------------------------------------------------------
caso97: mov OP1, 0
stop

; -----------------------------------------------------------------------------
; ID 98 - DIRECCION ANTERIOR AL SEGMENTO
; DS-1 se convierte en selector CS con offset -1 y debe disparar el fallo.
; -----------------------------------------------------------------------------
caso98: mov edx, DS
sub edx, 1
mov eax, [edx]
stop

; -----------------------------------------------------------------------------
; ID 99 - ESCRITURA DE IP
; STOP debe quedar inalcanzable porque MOV deja IP=0xFFFFFFFF.
; -----------------------------------------------------------------------------
caso99: mov IP, -1
stop

; -----------------------------------------------------------------------------
; ID 100 - DS ESCRIBIBLE Y AUTOMODIFICACION DE CS
; Al poner DS=0, [DS] apunta al inicio del codigo y sobrescribe 4 bytes.
; -----------------------------------------------------------------------------
caso100: mov DS, 0
mov edx, DS
mov ebx, 0x1234
mov [edx], ebx
stop

; -----------------------------------------------------------------------------
; ID 101 - OFFSET NEGATIVO EN OPERANDO MEMORIA
; EDX=DS+4 y offset -4 deberian volver a DS. La suma unsigned actual puede
; acarrear desde el selector 1 al selector 2 y acceder fuera de la RAM.
; -----------------------------------------------------------------------------
caso101: mov edx, DS
add edx, 4
mov [edx-4], 0x1234
stop

; -----------------------------------------------------------------------------
; ID 102 - ACCESO FUERA DEL FINAL DE RAM
; El codigo no valida offset+tamanio contra el limite del segmento/RAM.
; El offset grande tambien estresa el buffer corto del desensamblador.
; -----------------------------------------------------------------------------
caso102: mov [DS+16000], 0x1234
stop

; -----------------------------------------------------------------------------
; ID 103 - CANTIDAD DE CELDAS 256
; ECX.low16=256, pero SYS la guarda en uint8_t: la cantidad observada es 0.
; Por eso no deberia aparecer ninguna linea de dato de SYS WRITE.
; -----------------------------------------------------------------------------
caso103: mov [DS], 0x1234
mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 256
sys 2
stop

; -----------------------------------------------------------------------------
; ID 104 - ANCHO DE CELDA NO SOPORTADO
; SYS acepta 5 aunque leerMemoria/escribirMemoria fueron pensadas para 1,2,4.
; -----------------------------------------------------------------------------
caso104: mov [DS], 0x1234
mov eax, 0x01
mov edx, DS
ldh ecx, 5
ldl ecx, 1
sys 2
stop

; -----------------------------------------------------------------------------
; ID 105 - SERVICIO SYS DESCONOCIDO
; La implementacion actual no informa error para el numero 3.
; -----------------------------------------------------------------------------
caso105: sys 3
stop

; -----------------------------------------------------------------------------
; ID 106 - ENTRADA DECIMAL INVALIDA
; Ingresar "texto" como segundo dato. scanf falla y dato puede quedar sin
; inicializar; despues se muestra en hexadecimal lo que se escribio.
; -----------------------------------------------------------------------------
caso106: mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

; -----------------------------------------------------------------------------
; ID 107 - ENTRADA BINARIA DEMASIADO LARGA
; Ingresar por ejemplo 64 caracteres '1'. scanf("%s") escribe sobre un buffer
; local de 33 bytes sin limite de longitud.
; -----------------------------------------------------------------------------
caso107: mov eax, 0x10
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, 0x08
sys 2
stop

; -----------------------------------------------------------------------------
; ID 108 - STOP
; La escritura posterior debe ser inalcanzable.
; -----------------------------------------------------------------------------
caso108: stop
mov [DS], -1
stop

; Selector no reconocido: imprime -1 en decimal y termina.
selector_invalido: mov [DS], -1
mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 2
stop
