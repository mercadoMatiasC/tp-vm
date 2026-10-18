
main:
    ; 1. Cargamos un valor inicial en un registro
    mov eax, 11        ; EAX = 10

    ; 2. Llamamos a la subrutina llamada "multiplicar"
    
    call multiplicar   ; Aquí la VM guarda el IP actual en la pila y salta
    sys 0xF
    ; 5. El programa continúa acá después del RET
    ; Si todo salió bien, EAX ahora vale 20
    
    ; Terminamos imprimiendo el resultado con SYS (Servicio 2 WRITE, por ejemplo)
    mov [DS], eax       ; Copiamos el resultado a EBX para mostrarlo
    mov eax, 0x1         ; Servicio WRITE
    mov edx,DS
    ldl ecx,1
    ldh ecx,4
    sys  0x2            ; Ejecuta la llamada al sistema

    ret               ; Fin del programa
    



multiplicar:  add eax, eax       ; EAX = 10 + 10 = 20

    ; 4. Retornamos al punto donde fuimos llamados
    sys 0xF
    ret                ; La VM saca la dirección de la pila y vuelve a la línea 9

