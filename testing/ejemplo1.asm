\\include "heap1.asm"
; Binary Trees

; Estructura del nodo:
;    +------------+
;  0 |   entero   |  (2 bytes)
;    +------------+
; +2 |   *left    |  (4 bytes)
;    +------------+
; +6 |   *right   |  (4 bytes)
;    +------------+
btn_size    equ     10 ; Binary Tree Node
val         equ     0
left        equ     2
right       equ     6


;----------------------------------------
; crea un nuevo nodo de árbol binario
; parámetros: +8 valor entero
;----------------------------------------
; invocación:
; push  <valor entero>
; call  btn_new
; add   sp, 4
; << eax puntero al nuevo nodo
;----------------------------------------
btn_new:        push    bp
                mov     bp, sp

                push    btn_size
                call    alloc
                add     sp, 4      

                cmp     eax, null
                jz      btn_new_fin

                mov     w[eax+val], [bp+8]
                mov     [eax+left], null
                mov     [eax+right], null

btn_new_fin:    mov     sp, bp
                pop     bp
                ret 

;----------------------------------------
; agrega nodo a árbol binario de búsqueda (BST)
; parámetros: 
;  +8 doble puntero a root
; +12 puntero a nodo de arbol a insertar
;----------------------------------------
; invocación:
; push  <*bnt>
; push  <**root>
; call  bst_add
; add   sp,8
; (no devuelve nada)
;----------------------------------------
bst_add:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    edx            

                mov     edx, [bp+8]     ; **root
                mov     ebx, [edx]      ; *root
                mov     eax, [bp+12]    ; *bnt a insertar

                cmp     eax, null
                jz      bst_add_end

                cmp     ebx, null
                jz      bst_append

                cmp     w[ebx+val],w[eax+val]
                jz      bst_add_end

                jp      bst_add_left 
                jn      bst_add_right

bst_add_left:   add     ebx, left
                push    eax
                push    ebx
                call    bst_add
                add     sp, 8
                jmp     bst_add_end

bst_add_right:  add     ebx, right
                push    eax
                push    ebx
                call    bst_add
                add     sp, 8
                jmp     bst_add_end

bst_append:     mov     [edx], eax
bst_add_end:    pop     edx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret

;----------------------------------------
; invocación:
; push  <*root>
; call  preorder
; add   sp,4
; (no devuelve nada)
;----------------------------------------
preorder:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    ecx
                push    edx

                mov     ebx, [bp+8]     ; *root
                
                cmp     ebx, null
                jz      preorder_end

                ; preparo en edx la dirección de la variable aux
                mov     edx, ebx
                add     edx, val    
                mov     eax, 0x0001
                ldh     ecx, 0x02
                ldl     ecx, 0x01
                sys     0x0002
                ; llamo por izquierda
                push    [ebx+left]
                call    preorder
                add     sp, 4

                ; llamo por derecha
                push    [ebx+right]
                call    preorder
                add     sp, 4

preorder_end:    pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret



dato equ 0
izq equ 2
der equ 6


;push <<arbol**>>
;call maximo
;add sp,4
;En eax retorna el doble puntero al subarbol cuya raiz es maximo

maximo: push bp
        mov bp,sp
        push ebx
        push edx
        push ecx
        push eex
        push efx
        mov edx,[bp+8]; edx es A
        mov ebx,edx;ebx es max
        cmp [edx],null
        jz final
        mov eex,[edx]
        add eex,izq
        push eex
        call maximo
        add sp,4
        mov efx,eax
        mov eex,[edx]
        add eex,der
        push eex
        call maximo
        add sp,4
        cmp [efx],null
        jz analizoDerecha
        mov eex,[efx]
        mov ecx,[ebx]
        cmp w[eex+dato],w[ecx+dato]
        jnp analizoDerecha
        mov ebx,efx
analizoDerecha: cmp [eax],null
                jz final
                mov eex,[eax]
                mov ecx,[ebx]
                cmp w[eex+dato],w[ecx+dato]
                jnp final
                mov ebx,eax
final:  mov eax,ebx
        pop efx
        pop eex
        pop ecx
        pop edx
        pop ebx
        mov sp,bp
        pop bp
        ret







root  equ    4



MAIN:   push bp
        mov bp, sp
        sub sp, 4 ; root de ABB
        push eax
        push edx

        
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

        push 55
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        

        push [edx]
        call preorder
        add sp, 4

        
        push edx
        call maximo
        add sp,4

        push [eax]
        call preorder
        add sp,4

        


main_end:   pop edx
        pop eax
        
        mov sp,bp
        pop bp
        stop 
