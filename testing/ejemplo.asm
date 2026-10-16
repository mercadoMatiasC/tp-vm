; ==============================================================================
; ARCHIVO .ASM DE PRUEBA: Segunda Entrega (Corregido sin inmediatos de 32 bits)
; ==============================================================================

inicio:
    ; 1. Configurar el segmento de datos
    mov edx, DS

    ; --------------------------------------------------------------------------
    ; TEST 1: Escritura y Lectura de Registros Parciales (Sectores)
    ; Usamos una base de 16 bits como inmediato y operamos con registros
    ; --------------------------------------------------------------------------
    mov eax, 0x5678          ; Inmediato válido de 16 bits en EAX
    mov al,  0xAA            ; Modifica solo el byte bajo (Sector 1)
    mov ah,  0xBB            ; Modifica solo el tercer byte (Sector 2)
    mov ax,  0xCCDD          ; Modifica el word bajo (Sector 3)

    ; Guardamos este EAX modificado en memoria para validarlo despues
    mov [DS+0], eax          

    ; --------------------------------------------------------------------------
    ; TEST 2: Acceso a Memoria con distintos tamaños de bytes (cantBytes)
    ; --------------------------------------------------------------------------
    
    ; Caso 2A: Escribir un byte individual en [DS+4] usando AL
    mov [DS+4], al           
    
    ; Caso 2B: Escribir un word de 16 bits en [DS+6] usando AX
    mov [DS+6], ax           

    ; --------------------------------------------------------------------------
    ; TEST 3: Validación por pantalla con SYS 0x2
    ; --------------------------------------------------------------------------

    ; Imprimir la celda de la prueba 1 (EAX completo guardado en DS+0)
    mov eax, 0x08            ; Activa formato Hexadecimal
    mov edx, DS              ; Base DS
    add edx, 0               ; Offset 0
    ldl ecx, 1               ; 1 celda
    ldh ecx, 4               ; Tamaño: 4 bytes
    sys 0x2                  

    ; Imprimir la celda de la prueba 2B (Word guardado en DS+6)
    mov eax, 0x08            ; Activa formato Hexadecimal
    mov edx, DS              ; Base DS
    add edx, 6               ; Offset 6
    ldl ecx, 1               ; 1 celda
    ldh ecx, 4               ; Tamaño: 4 bytes
    sys 0x2     

    sys 0x0             

    stop