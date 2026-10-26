









\\include "heap.asm"
; Estructura del nodo:
;    +------------+
;  0 |   entero   |  (4 bytes)
;    +------------+
; +2 | *siguiente |  (4 bytes)
;    +------------+
nodo_size   equ     8
valor equ 0
sig equ 4


;----------------------------------------
; crea un nuevo nodo de la lista 
; parámetros: +8 valor entero
;----------------------------------------
; invocación:
; push  <valor entero>
; call  nodo_nuevo
; add   sp, 4
; << eax puntero al nuevo nodo
;----------------------------------------
nodo_nuevo:     push    bp
                mov     bp, sp

                push    nodo_size
                call    alloc
                add     sp, 4      

                cmp     eax, null
                jz      nodo_nuevo_fin

                mov     [eax+val], [bp+8]
                mov     [eax+sig], null

nodo_nuevo_fin: mov     sp, bp
                pop     bp
                ret 


;----------------------------------------
; imprime una lista de valores enteros
; parámentros: +8 puntero a primer nodo
;----------------------------------------
; invocación:
; push  <nodo*>
; call  list_print
; add   sp, 4
;----------------------------------------
list_print:     push    bp
                mov     bp, sp
                push    eax
                push    ecx
                push    edx

                ldh     ecx, 4
                ldl     ecx, 1
                mov     eax, 0x1
                mov     edx, [bp+8]
list_print_imp: cmp     edx, null
                jz      list_print_fin
                sys     0x2
                mov     edx, [edx+sig]
                jmp     list_print_imp

list_print_fin: pop     edx
                pop     ecx
                pop     eax
                mov     sp, bp
                pop     bp
                ret


;----------------------------------------
; insertar un nodo en una lista ordenada
; parametro +8: doble puntero a la lista
; parametro +12: puntero al nodo a insertar
;----------------------------------------
; invocación:
; push <nodo* nuevo_nodo>
; push <nodo** head>
; call insert_nodo
; add  sp, 8
;----------------------------------------
insert_nodo:    push    bp
                mov     bp,sp
                push    eax
                push    ebx
                push    edx

                mov     edx, [bp+8]     ; doble puntero a la lista
                mov     eax, [bp+12]    ; puntero al nodo a insertar
                
                cmp     eax, null       ; si nuevo nodo es null
                jz      insert_fin      ; me voy YA!

                mov     ebx, [edx]      ; puntero a la lista

                cmp     ebx, null
                jz      insert_actual

                cmp     [eax+val], w[ebx+val]    ; si el nodo a insertar es menor que el primero
                jnp     insert_actual   ; insertar al principio

                add     ebx, sig
                push    eax
                push    ebx
                call    insert_nodo
                add     sp, 8
                jmp     insert_fin

insert_actual:  mov     [eax+sig], [edx]   ; insertar al principio
                mov     [edx], eax
                jmp     insert_fin
insert_fin:     pop     edx
                pop     ebx
                pop     eax
                mov     sp,bp
                pop     bp
                ret










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
;call buscaValorABB
;add sp,8
;Retorna en eax 1 si lo encontre 0 si no lo encontre


; ==================================================
; Subrutina: buscaValorABB
; Recibe en pila: 
;   [bp+12] = Puntero al nodo actual (root)
;   [bp+8]  = Valor entero a buscar
; ==================================================
buscaValorABB: push bp
        mov bp, sp

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
        call buscaValorABB
        add sp, 8
        jmp fin

recorre_izq: mov eax, [eax+left]
        push eax
        push [bp+8]
        call buscaValorABB
        add sp, 8
        jmp fin

noExist:  mov eax,0
        jmp fin

Exist: mov eax,1

fin:    mov sp, bp
        pop bp
        ret



;push <<lista**>>
;push <<arbol*>>
;call listaElim
;add sp,8

listaElim:  push bp
            mov bp,sp
            
            push eax
            push edx
            push ebx

            mov edx,[bp+12]
            cmp [edx],null
            jz termino
            mov ebx,[edx]

            
            push [bp+8]
            push [ebx+valor]
            call buscaValorABB
            add sp,8

            cmp eax,1
            jz elimNodo
            jmp continua
elimNodo:   mov eax,[ebx+sig]
            mov [edx],eax
            push edx
            push [bp+8]
            call listaElim
            add sp,8
            jmp termino

continua:   mov edx,ebx
            add edx,sig
            push edx
            push [bp+8]
            call listaElim
            add sp,8
termino:        pop ebx
            pop edx
            pop eax
            mov sp,bp
            pop bp
            ret









root  equ    4
head equ 8

MAIN:   push bp
        mov bp, sp
        sub sp, 4 ; root de ABB
        push eax
        push edx
      
        
        call    heap_init    ; inicializa memoria dinámica

        mov edx, bp
        sub edx, root        ; edx = &root
        mov [edx], null      ; inicializo root = null

        push 10
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

        push 8
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 1
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 16
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 21
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        
       
        push [edx]
        call preorder
        add sp, 4
        
        push [edx]
        push 25
        call buscaValorABB
        add sp,8

        ;Doble puntero de arbol es ahora eex
        mov eex,edx


        mov edx,ds
        add edx,100
        mov [ds+100],eax
        ldh ecx,4
        ldl ecx,1
        mov eax,0x1
        sys 0x2

        ;declaro un lista simplemente enlazada (sll)
        ; head->10->2->1->8->6->16->(null)

            mov     [bp-head], null     ; nodo* head = null
            mov     ebx, bp
            sub     ebx, head           ; ebx = &head

            
            
            push    10
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; head = nodo_nuevo(10);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; ebx = &nodo->sig

                      

            push    2
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo->sig = nodo_nuevo(11);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; &nodo11->sig   

            push    1
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo->sig = nodo_nuevo(11);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; &nodo11->sig 

            push    8
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo->sig = nodo_nuevo(11);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; &nodo11->sig 

            push    6
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo->sig = nodo_nuevo(11);
            mov     ebx, [ebx]          ;
            add     ebx, sig            ; &nodo11->sig 
          
            
            push    16
            call    nodo_nuevo
            add     sp, 4
            mov     [ebx], eax          ; nodo11->sig = nodo_nuevo(12)

           

            ; imprimir la lista
            push    [bp-head]
            call    list_print          ; list_print (head)
            add     sp, 4  


            mov efx ,bp
            sub efx,head
            push efx
            push [eex] 
            sys 0xF 
            call listaElim
            add sp,8

            ; imprimir la lista
            push    [efx]
            call    list_print          ; list_print (head)
            add     sp, 4  


            




main_end:   pop edx
        pop eax
        mov sp,bp
        pop bp
        stop 











