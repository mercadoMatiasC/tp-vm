mov eax, 0x01
mov edx, DS
ldh ecx, 4
ldl ecx, 1
sys 1
SHL [DS],2
