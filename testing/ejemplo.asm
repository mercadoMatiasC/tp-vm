\\DATA 65000

pop EAX  ; Saca el -1 (RET) SP sube a 1016
pop EBX  ; Saca el argc SP sube a 1020
pop ECX  ; Saca el *argv SP sube a 1024 (tope original)
pop edx