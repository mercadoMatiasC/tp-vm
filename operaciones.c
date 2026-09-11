#include <stdio.h>
#include <stdio.h>
#include <stdint.h>



typedef void op2operandos(int16_t *,int16_t *);

void mov(int16_t *a,int16_t *b){
    *a=*b;
}
void add(int16_t *a,int16_t *b){
    *a+=*b;
}
void sub(int16_t *a,int16_t *b){
    *a-=*b;
}
int esNegativo(int16_t a){
    return (a&(0x8000))>>15 & 0x0001; //seria mejor comparar con !=0 en lugar de 0x0001?
}
int esCero(int16_t a){
    return a==0;
}
int esPositivo(int16_t a){
    return !esNegativo(a) && !esCero(a);
}
int igualSigno(int16_t a,int16_t b){
    return esNegativo(a)==esNegativo(b);
}
int esDesbordamiento(int16_t a,int16_t b,int16_t res){
    return igualSigno(a,b) && !igualSigno(a,res) ;
}