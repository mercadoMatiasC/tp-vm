
mensaje     equ "Ingrese hasta 15 caracteres: "
resultado   equ "Cadena almacenada: "

main:       mov edx, KS
            add edx, mensaje
            sys 4

            mov edx, DS
            mov ecx, -1
            sys 3

            mov edx, KS
            add edx, resultado
            sys 4

            mov edx, DS
            sys 4
            ret
