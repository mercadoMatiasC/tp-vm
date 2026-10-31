inicio:
    
    ; 1. Configurar registros para SYS 1 (READ)
    
    mov edx, DS      ; Base lógica: Segmento de Datos
    ; add edx, 10    ; (Opcional) Le podés sumar un offset si querés guardarlo más adelante

    mov ecx,50

    ; 2. Ejecutar la lectura
    sys 0x3          ; Pausa la VM, te muestra el prompt "[XXXX]: " y espera tu letra
     
    mov edx, DS      ; Base lógica: Segmento de Datos
    ; add edx, 10    ; (Opcional) Le podés sumar un offset si querés guardarlo más adelante

    mov ecx,50

    ; 2. Ejecutar la lectura
    sys 0x3          ; Pausa la VM, te muestra el prompt "[XXXX]: " y espera tu letra


    ; 3. Clavar el BREAKPOINT justo después
    sys 0xF          ; La VM genera la imagen ".vmi" y te abre el menú "vmg>"
    sys 0xF
    stop             ; Fin del programa
