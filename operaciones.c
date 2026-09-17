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

uint32_t leerMemoria(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
    uint32_t codReg=regOp & 0x0000001F;
    uint32_t offset, direcLogica, direcFisica;

    offset=(regOp>>8) & 0x0000FFFF;
    direcLogica=offset+regTabla[codReg];
    direcFisica=convertirDirecLogica(direcLogica, segTabla);


    return ((uint32_t)memoriaPrincipal[direcFisica]<<24) | (uint32_t)(memoriaPrincipal[direcFisica+1]<<16) | (uint32_t)(memoriaPrincipal[direcFisica+2])<<8 | (uint32_t)(memoriaPrincipal[direcFisica+3]);
}

void escribirMemoria(uint32_t regOp, uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
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

uint32_t leerOperando(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint8_t codOp = (regOp >> 24) & 0x000000FF;
    uint32_t codReg;

    if (codOp == 1) {
        codReg = regOp & 0x0000001F;
        return regTabla[codReg];
    } else
        if (codOp == 2)
            // Al castear primero a int16_t y luego a int32_t/uint32_t, 0xFFFF pasa a ser 0xFFFFFFFF (-1)
            return (uint32_t)(int32_t)(int16_t)(regOp & 0x0000FFFF);
        else
            return leerMemoria(regOp, regTabla, segTabla, memoriaPrincipal);
}

void escribeOperando(uint32_t regOp, uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint8_t codOp=(regOp>>24)&0x000000FF;
    uint32_t codReg;

    if(codOp==1){
        codReg=regOp & 0x0000001F;
        regTabla[codReg]=valor;
    }
    else
        escribirMemoria(regOp,valor,regTabla,segTabla,memoriaPrincipal);
}

void actualizarCC_General(int32_t op1, int32_t op2, uint32_t regTabla[], int64_t resultado, int tipo) {
    //GUARDO LOS BITS QUE PUEDE ENTENDER LA VM NOMAS
    uint32_t res32 = (uint32_t)resultado;
    int32_t  res32_signed = (int32_t)res32;

    uint32_t Z = (res32 == 0);
    uint32_t N = (res32_signed < 0);
    uint32_t C = 0, V = 0;

    uint32_t b1 = ((uint32_t)op1 >> 31);
    uint32_t b2 = ((uint32_t)op2 >> 31);
    uint32_t bR = (res32 >> 31);

    switch (tipo) {
        case 1: //ADD
            C = ((uint64_t)resultado > 0xFFFFFFFFULL);
            V = ((b1 == b2) && (b1 != bR));
            break;

        case 2: // SUB y CMP
            C = ((uint32_t)op1 < (uint32_t)op2);
            V = ((b1 != b2) && (b1 != bR));
            break;

        case 3: //OPERACIONES LOGICAS: AND, OR, XOR, NOT, MOV
            C = 0;
            V = 0;
            break;

        case 5: //MUL
            C = V = ((uint64_t)resultado > 0xFFFFFFFFULL);
            break;

        case 6: //DIV
            C = 0;
            V = 0;
            break;
    }

    //ACTUALIZAMOS LOS BITS EN EL CC
    regTabla[17] = (N << 31) | (Z << 30) | (C << 29) | (V << 28);
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

void Not(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    valor = ~valor;

    escribeOperando(reg1, valor, regTabla, segTabla, memoriaPrincipal);
}

void mov(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
    uint32_t valor2=leerOperando(reg2,regTabla,segTabla,memoriaPrincipal);

    escribeOperando(reg1,valor2,regTabla,segTabla,memoriaPrincipal);
    actualizarCC_General(reg1, valor2, regTabla, valor2, 3);
}

void add(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 + (uint64_t)valor2; //HAGO LA SUMA SIN SIGNO PARA VER CUANTO DEBE DAR

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    actualizarCC_General(valor1, valor2, regTabla, resultado, 1); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void sub(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 - (uint64_t)valor2; //HAGO LA RESTA SIN SIGNO PARA VER CUANTO DEBE DAR

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    actualizarCC_General(valor1, valor2, regTabla, resultado, 2); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void mul(void) { //FALTA ACTUALIZAR CC
   printf("nada");
}

void div(void) { //FALTA ACTUALIZAR CC Y AC
    printf("nada");
}
