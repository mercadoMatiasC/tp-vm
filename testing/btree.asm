\\include "heap1.asm"
; Binary Trees

; Estructura del nodo:
;    +------------+
;  0 |   entero   |  (4 bytes)
;    +------------+
; +4 |   *left    |  (4 bytes)
;    +------------+
; +8 |   *right   |  (4 bytes)
;    +------------+
btn_size    equ     12 ; Binary Tree Node
val         equ     0
left        equ     4
right       equ     8


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

                mov     [eax+val], [bp+8]
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

                cmp     [ebx+val],[eax+val]
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
; imprime en in-order árbol binario de búsqueda (BST)
; parámetros: 
;  +8 puntero simple a root
;----------------------------------------
; invocación:
; push  <*root>
; call  inorder
; add   sp,4
; (no devuelve nada)
;----------------------------------------
inorder:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    ecx
                push    edx

                mov     ebx, [bp+8]     ; *root
                
                cmp     ebx, null
                jz      inorder_end

                ; llamo por izquierda
                push    [ebx+left]
                call    inorder
                add     sp, 4

                ; preparo en edx la dirección de la variable aux
                mov     edx, ebx
                add     edx, val    
                mov     eax, 0x0001
                ldh     ecx, 0x04
                ldl     ecx, 0x01
                sys     0x0002

                ; llamo por derecha
                push    [ebx+right]
                call    inorder
                add     sp, 4

inorder_end:    pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret



;----------------------------------------
; imprime en post-order árbol binario de búsqueda (BST)
; parámetros: 
;  +8 puntero simple a root
;----------------------------------------
; invocación:
; push  <*root>
; call  postorder
; add   sp,4
; (no devuelve nada)
;----------------------------------------
postorder:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    ecx
                push    edx

                mov     ebx, [bp+8]     ; *root
                
                cmp     ebx, null
                jz      postorder_end

                ; llamo por izquierda
                push    [ebx+left]
                call    postorder
                add     sp, 4

                ; llamo por derecha
                push    [ebx+right]
                call    postorder
                add     sp, 4

                ; preparo en edx la dirección de la variable aux
                mov     edx, ebx
                add     edx, val    
                mov     eax, 0x0001
                ldh     ecx, 0x04
                ldl     ecx, 0x01
                sys     0x0002
          

postorder_end:    pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret




;----------------------------------------
; imprime en post-order árbol binario de búsqueda (BST)
; parámetros: 
;  +8 puntero simple a root
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
                ldh     ecx, 0x04
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


;push <<root*>>
;push <<valorEntero>>
;call informaExiste
;add sp,8

existe   equ "Esta en el arbol"
noExiste equ " \n\tNo esta en el arbol"


; ==================================================
; Subrutina: informaExiste
; Recibe en pila: 
;   [bp+12] = Puntero al nodo actual (root)
;   [bp+8]  = Valor entero a buscar
; ==================================================
informaExiste1: push bp
        mov bp, sp
        push eax

        ; 1. Obtenemos el puntero al nodo actual ([bp+12])
        mov eax, [bp+12]
        
        ; 2. Caso base: si el puntero es null (0), no está
        cmp eax, null
        jz noExist

        cmp [eax+val],[bp+8]
        
        jz Exist              ; Si dio cero, son iguales -> ¡Lo encontró!
        jn recorre_der        ; Si dio negativo (el nodo es mayor al buscado), va a la derecha
        jp recorre_izq       ; Si dio positivo (el nodo es menor al buscado), va a la izquierda

recorre_der: mov eax, [eax+right]
        push eax
        push [bp+8]
        call informaExiste1
        add sp, 8
        jmp fin

recorre_izq: mov eax, [eax+left]
        push eax
        push [bp+8]
        call informaExiste1
        add sp, 8
        jmp fin

noExist:  mov edx, ks
        add edx, noExiste
        sys 0x4
        jmp fin

Exist: mov edx, ks
        add edx, existe
        sys 0x4

fin:  pop eax
        mov sp, bp
        pop bp
        ret