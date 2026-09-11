#include <stdio.h>
#include <stdio.h>
#include <stdint.h>

void nada(){
    printf("nada\n");
}

void sys(void) {
    printf("SYS \n");
}

void jmp(void) {
    printf("JPM \n");
}

void jp(void) {
    printf("JP \n");
}

void Jn(void) {
    printf("JN \n");
}

void jz(void) {
    printf("JZ \n");
}

void jc(void) {
    printf("JC \n");
}

void jv(void) {
    printf("JV \n");
}

void jnp(void) {
    printf("JNP \n");
}

void jnn(void) {
    printf("JNN \n");
}

void jnz(void) {
    printf("JNZ \n");
}

void not(void) {
    printf("NOT \n");
}

void mov(void){
    printf("MOV \n");
}

void add(uint32_t *destino, uint32_t origen) { //FALTA ACTUALIZAR CC
    *destino = *destino + origen;
}

void sub(uint32_t *destino, uint32_t origen) { //FALTA ACTUALIZAR CC
    *destino = *destino - origen;
}

void mul(uint32_t *destino, uint32_t origen) { //FALTA ACTUALIZAR CC
    *destino = *destino*origen;
}

void div(uint32_t *destino, uint32_t origen) { //FALTA ACTUALIZAR CC Y AC
    *destino = *destino/origen;
}