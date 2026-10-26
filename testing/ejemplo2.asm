\\include  "btree.asm"

root  equ    4

MAIN:   push bp
        mov bp, sp
        sub sp, 4 ; root de ABB
        push eax
        push edx
        sys 0xF
        
        call    heap_init    ; inicializa memoria dinámica

        mov edx, bp
        sub edx, root        ; edx = &root
        mov [edx], null      ; inicializo root = null

        push 20
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 20
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 10
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 15
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 30
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 25
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        
        sys 0xF
        push [edx]
        call preorder
        add sp, 4
        
        push [edx]
        push 5
        call informaExiste1
        add sp,8


main_end:   pop edx
        pop eax
        mov sp,bp
        pop bp
        stop 
