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

void escribirMemoria(uint32_t regOp,uint32_t valor, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]){
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
    }
    else{
        if(codOp==2){
            return regOp & 0x00FFFF;
        }
        else{
            return leerMemoria(regOp,regTabla,segTabla,memoriaPrincipal);
        }
    }
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
void actualizarCC_General(int32_t op1, int32_t op2, int32_t resultado, int tipo,uint32_t regTabla[]) {
    uint32_t Z = (resultado == 0);
    uint32_t N = (resultado >> 31) & 0x1;
    uint32_t C = 0, V = 0;

    uint32_t b1 = (op1 >> 31)&0x1;//bit mas significativo de b1, idem con b2 y bR
    uint32_t b2 = (op2 >> 31)&0x1;
    uint32_t bR = (resultado >> 31)&0x1;

    switch (tipo) {
        case 1://ADD
            C = (uint32_t)resultado < (uint32_t)op1;//el resultado sin signo es menor a cualquiera de los operandos sin signos
            V = ((b1 == b2) && (b1 != bR));//los operandos son de igual signo pero el resultado no
            break;

        case 2://SUB Y CMP
            C = (uint32_t)resultado < (uint32_t)(op1); /*evaluo que el resultado sin signo sea menor al minuendo
                                                        Aclaracion: la resta de 2 numeros en binario es igual a la suma del primero con el complemento a2 del segundo.
                                                        Por esa razon el criterio de C es identico al de la suma.
                                                        //A considerar si el = debe estar (unico caso en el que se cumple es al restar 0)*/

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
        //Faltan MUL Y DIV
        */
    }


    // Actualizar el registro CC de la VMX26
    regTabla[17] = (N<<31) + (Z<<30) + (C<<29) + (V<<28);
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

void mov(uint32_t reg1,uint32_t reg2, uint32_t regTabla[],regSegmento segTabla[],uint8_t memoriaPrincipal[]){
    uint32_t valor2=leerOperando(reg2,regTabla,segTabla,memoriaPrincipal);

    escribeOperando(reg1,valor2,regTabla,segTabla,memoriaPrincipal);
}

void add(uint32_t reg1,uint32_t reg2, uint32_t regTabla[],regSegmento segTabla[],uint8_t memoriaPrincipal[]) { //FALTA ACTUALIZAR CC
    uint32_t valor1=leerOperando(reg1,regTabla,segTabla,memoriaPrincipal);
    uint32_t valor2=leerOperando(reg2,regTabla,segTabla,memoriaPrincipal);

    escribeOperando(reg1,valor1+valor2,regTabla,segTabla,memoriaPrincipal);
}


void sub(void) { //FALTA ACTUALIZAR CC
    printf("nada");
}

void mul(void) { //FALTA ACTUALIZAR CC
   printf("nada");
}

void div(void) { //FALTA ACTUALIZAR CC Y AC
    printf("nada");
    }
