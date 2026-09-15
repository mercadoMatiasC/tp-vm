#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include "operaciones.h"

uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[]){
    int codSeg=(direcLogica>>16)&0x0000FFFF;  //La mascara es por si cambiamos direcLogica a int para evitar extension de signo
    int offset=direcLogica & 0x0000FFFF;      //Obtengo el desplazamiento de la direcLogica
    uint32_t direcBase=segTabla[codSeg].base; //Obtengo de la tabla de descriptores la direc base

    return direcBase + offset;
}

uint32_t leerMemoria(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]){
    uint32_t codReg=regOp & 0x0000001F;
    uint32_t offset, direcLogica, direcFisica;

    offset=(regOp>>8) & 0x0000FFFF;
    direcLogica=offset+regTabla[codReg];
    direcFisica=convertirDirecLogica(direcLogica, segTabla);

    
    return ((uint32_t)memoriaPrincipal[direcFisica]<<24) | (uint32_t)(memoriaPrincipal[direcFisica+1]<<16) | (uint32_t)(memoriaPrincipal[direcFisica+2])<<8 | (uint32_t)(memoriaPrincipal[direcFisica+3]);
}

uint32_t escribirMemoria(uint32_t regOp,uint32_t valor, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]){
    uint32_t codReg=regOp & 0x0000001F;
    uint32_t offset, direcLogica, direcFisica;

    offset=(regOp>>8) & 0x0000FFFF;
    direcLogica=offset+regTabla[codReg];
    direcFisica=convertirDirecLogica(direcLogica, segTabla);

    memoriaPrincipal[direcFisica]=(valor>>24) & 0xFF; //gardo 8 bits mas significativos en la primer celda
    memoriaPrincipal[direcFisica+1]=(valor>>16) & 0xFF;
    memoriaPrincipal[direcFisica+2]=(valor>>8) & 0xFF;
    memoriaPrincipal[direcFisica+3]=valor & 0xFF;

}

uint32_t leerOperando(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]){
    uint8_t codOp=(regOp>>24)&0x000000FF;
    uint32_t codReg;

    if(codOp==1){
        codReg=regOp & 0x0000001F;
        return regTabla[codReg];
    }else
        if(codOp==2)
            return (int16_t)(regOp & 0x00FFFF);
        else
            return leerMemoria(regOp, regTabla, segTabla, memoriaPrincipal);
}

void escribeOperando(uint32_t regOp,uint32_t valor, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]){
    uint8_t codOp=(regOp>>24)&0x000000FF;
    uint32_t codReg;
    if(codOp==1){
        codReg=regOp & 0x0000001F;
        regTabla[codReg]=valor;
    }
    else{
        escribirMemoria(regOp,valor,regTabla,segTabla,memoriaPrincipal);
    }
}

//operaciones auxiliares
void actualizarCC_General(int32_t op1, int32_t op2,  uint32_t regTabla[], int64_t resultado, int tipo) {
    uint32_t Z = (resultado == 0);
    uint32_t N = ((int64_t) resultado) < 0; //4294967295 = FFFFFFFF
    uint32_t C = 0, V = 0;
 
    uint32_t b1 = ((int32_t) op1) < 0;//bit mas significativo de b1, idem con b2 y bR
    uint32_t b2 = ((int32_t) op2) < 0;
    uint32_t bR = ((int64_t) resultado) < 0;

    switch (tipo) {
        case 1://ADD
            C = (int64_t)resultado < (uint32_t)op1;//el resultado sin signo es menor a cualquiera de los operandos sin signos
            V = ((b1 == b2) && (b1 != bR));//los operandos son de igual signo pero el resultado no
            printf("Suma");
            break;

        case 2://SUB Y CMP
            C = (int64_t)resultado < (uint32_t)(op1);   //evaluo que el resultado sin signo sea menor al minuendo 
                                                        //Aclaracion: la resta de 2 numeros en binario es igual a la suma del primero con el complemento a2 del segundo.
                                                        //Por esa razon el criterio de C es identico al de la suma.
                                                        //A considerar si el = debe estar (unico caso en el que se cumple es al restar 0)

            V = (b1 != b2) && (b1 != bR);//el signo del minuendo es distinto tanto del del substraendo como del resultado 
            break;

        case 3://AND,OR,XOR,NOT,MOV
            C = 0;
            V = 0;
            break;

        /*case 4://SHL,SHR,SAR DUDOSO
            C = bitPerdido; // El bit que salio de los limites
            V = 0;
            break;
        */

        //Faltan MUL Y DIV
        
    }

    // Actualizar el registro CC de la VMX26
    regTabla[17] = (N<<31) + (Z<<30) + (C<<29) + (V<<28);

    printf("\nN: %u, Z: %u, C: %u, V: %u", N, Z, C, V);
    printf("\nResultado: %016X", resultado);
} 

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

void mov(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
    uint32_t valor2=leerOperando(reg2,regTabla,segTabla,memoriaPrincipal);

    escribeOperando(reg1,valor2,regTabla,segTabla,memoriaPrincipal);
    actualizarCC_General(reg1, valor2, regTabla, valor2, 3);
}

void add(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) { //FALTA ACTUALIZAR CC
    uint32_t valor1=leerOperando(reg1,regTabla,segTabla,memoriaPrincipal);
    uint32_t valor2=leerOperando(reg2,regTabla,segTabla,memoriaPrincipal);
    uint64_t resultado = 0;

    escribeOperando(reg1, valor1+valor2, regTabla, segTabla, memoriaPrincipal);

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, valor1+valor2, 1);
}

void sub() { //FALTA ACTUALIZAR CC

}

void mul(void) { //FALTA ACTUALIZAR CC
   printf("nada");
}

void div(void) { //FALTA ACTUALIZAR CC Y AC
    printf("nada");
}