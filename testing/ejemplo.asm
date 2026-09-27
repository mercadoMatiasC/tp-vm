mov eax, 1
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
