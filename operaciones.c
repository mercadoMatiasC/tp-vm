#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h> //TIENE CONSTANTES CON VALORES MAXIMOS Y MINIMOS DE TIPOS ENTEROS
#include "operaciones.h"

uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[],int nBytes){
    int codSeg=(direcLogica>>16)&0x0000FFFF;  //La mascara es por si cambiamos direcLogica a int para evitar extension de signo
    int16_t offset=direcLogica & 0x0000FFFF;      //Obtengo el desplazamiento de la direcLogica
   
    
    if(codSeg<0 || codSeg>7){
        printf("Fallo de segmento: codSeg invalido");
        exit(1);
    }

    uint32_t direcBase=segTabla[codSeg].base; //Obtengo de la tabla de descriptores la direc base
    int32_t direcFisica=(int32_t)direcBase+(int32_t)offset;//Necesito castearlo a negativo por si entra un offset negativo, para asi poder valir la direcFisica
                                                    //Si trato a direcFisica y a direcBase como uint, el offset negativo me lo promocionan a positivo y dejaria de ser un valor invalido

    if((int32_t)direcBase>direcFisica){
        printf("Fallo de segmento: direccionFisica<direccionBase");
        exit(1);
    }
    int32_t limiteSegmento= (int16_t)direcBase+(int16_t)segTabla[codSeg].tamaño;//casteo direcBase a int y tamanio tambien
                                                                                //entonces si codSeg=2, tamanio y base seran tomados como -1 y no como un numero muy grande
    int32_t limiteAcceso=direcFisica+nBytes;
    
    if(limiteSegmento<limiteAcceso){
        
        printf("Fallo de segmento: limiteAcceso>limiteSegmento");
        exit(1);
    }
    return (uint32_t)direcFisica;
}

uint32_t leerMemoria(uint32_t direcLogica, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[],uint8_t nBytes){
    uint32_t direcFisica;
    int i;
    uint32_t resultado=0;


    regTabla[4]=direcLogica;//modifico el LAR
    

    direcFisica=convertirDirecLogica(direcLogica, segTabla,nBytes);
    
    regTabla[5] = ((uint32_t)nBytes << 16) | (direcFisica & 0xFFFF);
    for(i=0;i<nBytes;++i){
        resultado= (resultado<<8) | memoriaPrincipal[direcFisica+i];
    }
    
    if (nBytes == 1) {
        // Convierte el valor de 8 bits a int8_t para interpretar el bit de signo
        // y luego expande los 1s a los 32 bits del uint32_t
        resultado = (uint32_t)(int32_t)(int8_t)resultado;
    } else if (nBytes == 2) {
        // Convierte el valor de 16 bits a int16_t para interpretar el bit de signo
        // y luego expande los 1s a los 32 bits del uint32_t
        resultado = (uint32_t)(int32_t)(int16_t)resultado;
    }
    
    // Si nBytes == 4, no requiere extensión de signo porque ya ocupa los 32 bits completos.
    regTabla[6]=resultado; //modifico el registro MBR
    return resultado;
}

void escribirMemoria(uint32_t direcLogica, uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[],uint8_t nBytes){

    uint32_t direcFisica;
    int i;

    regTabla[4]=direcLogica;//modifico el LAR
    direcFisica=convertirDirecLogica(direcLogica, segTabla,nBytes);

    regTabla[5] = ((uint32_t)nBytes << 16) | (direcFisica & 0xFFFF);

    for(i=0;i<nBytes;i++){
        memoriaPrincipal[direcFisica+i]=(valor>>((nBytes-1-i)*8)) & 0xFF;
    }
    regTabla[6]=valor; //modifico el registro MBR
}

uint32_t leerOperando(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint8_t codOp = (regOp >> 24) & 0x000000FF;
    uint32_t codReg = regOp & 0x0000001F;
    
    if (codOp == 1) {
        return regTabla[codReg];
    }else
        if (codOp == 2)
            // Al castear primero a int16_t y luego a int32_t/uint32_t, 0xFFFF pasa a ser 0xFFFFFFFF (-1)
            return (uint32_t)(int32_t)(int16_t)(regOp & 0x0000FFFF);
        else {
            int16_t offset = (regOp >> 8) & 0x0000FFFF;
            uint32_t direcLogica = (regTabla[codReg] & 0xFFFF0000) | (uint16_t)((regTabla[codReg] & 0xFFFF) + offset);
            
           //Sumar offset no con registro sino con los 2 bytes menos significativos y despues conectarlo con los 2 bytes mas significativos del registro codReg
           
           return leerMemoria(direcLogica, regTabla, segTabla, memoriaPrincipal,4);
        }
}

void escribeOperando(uint32_t regOp, uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint8_t codOp=(regOp>>24)&0x000000FF;
    uint32_t codReg = regOp & 0x0000001F;

    if(codOp==1){
        regTabla[codReg]=valor;
    }
    else{
        int16_t offset = (regOp >> 8) & 0x0000FFFF;
        uint32_t direcLogica = (regTabla[codReg] & 0xFFFF0000) | (uint16_t)((regTabla[codReg] & 0xFFFF) + offset);
        
        escribirMemoria(direcLogica,valor,regTabla,segTabla,memoriaPrincipal,4);
    }
}

void actualizarCC_General(int32_t op1, int32_t op2, uint32_t regTabla[], int64_t resultado, int tipo) {
    uint32_t res32 = (uint32_t)resultado;
    int32_t  res32_signed = (int32_t)res32;

    uint32_t Z = (res32 == 0);
    uint32_t N = (res32_signed < 0);
    uint32_t C = 0, V = 0;

    //AUXILIARES UNSIGNED PARA CALCULAR EL CARRY
    uint64_t u1 = (uint32_t)op1;
    uint64_t u2 = (uint32_t)op2;
    
    //AUXILIARES SIGNED PARA CALCULAR EL OVERFLOW
    int64_t  s1 = (int32_t)op1;
    int64_t  s2 = (int32_t)op2;

    uint32_t shift = (uint32_t)op2; //POSICIONES A DESPLAZAR EN CASO SHIFT

    switch (tipo) {
        case 1: { // ADD
            uint64_t res_u = u1 + u2;
            int64_t  res_s = s1 + s2;

            C = (res_u > 0xFFFFFFFFULL); //OCURRE SI EL RESULTADO UNSIGNED ESCAPA LOS 32 BIT
            V = (res_s < INT32_MIN || res_s > INT32_MAX); //OCURRE SI LA REPRESENTACION DEL NUMERO ESCAPA LOS 32 BITS SIGNED
            break;
        }

        case 2: { // SUB y CMP
            int64_t res_s = s1 - s2;
            
            C = (u1 < u2); //BORROW
            V = (res_s < INT32_MIN || res_s > INT32_MAX);
            break;
        }

        case 3: // OPERACIONES LOGICAS: AND, OR, XOR, NOT, MOV
            C = 0;
            V = 0;
            break;

        case 5: { // MUL
            uint64_t res_u = u1 * u2;
            int64_t  res_s = s1 * s2;

            C = (res_u > 0xFFFFFFFFULL);
            V = (res_s < INT32_MIN || res_s > INT32_MAX);
            break;
        }

        case 6: // DIV
            C = 0;
            V = 0;
            break;

        case 7: // SHL
            if (shift > 0 && shift <= 32)
                C = ((u1 >> (32 - shift)) & 1); //C DEPENDE DEL ÚLTIMO BIT QUE QUEDÓ AFUERA
            else
                C = 0;

            V = 0;
            break;

        case 8: // SHR
        case 9: // SAR
            if (shift > 0 && shift <= 32)
                C = ((u1 >> (shift - 1)) & 1); //C DEPENDE DEL ÚLTIMO BIT QUE QUEDÓ AFUERA
            else
                C = 0;

            V = 0;
            break;
    }

    // ACTUALIZAMOS LOS BITS EN EL CC (Registro 17)
    regTabla[17] = (N << 31) | (Z << 30) | (C << 29) | (V << 28);
}

void nada(){
    printf("nada\n");
}

void devuelveBinario(int32_t num, char* cad, int nBits) {
    int idx = 0;
    int encontro_primer_uno = 0;
    uint32_t val = (uint32_t)num; // Casteo a uint32_t para evitar extensión de signo no deseada

    // Si el número es 0 directamente guardamos "0"
    if (val == 0) {
        cad[idx++] = '0';
        cad[idx] = '\0';
    }
    else {
        // CORRECCIÓN CLAVE: arrancamos en (nBits - 1)
        // Para 32 bits, i va desde 31 hasta 0 (32 iteraciones exactas)
        for (int i = nBits - 1; i >= 0; i--) {
            uint8_t bit = (val >> i) & 0x01;

            if (bit == 1) {
                encontro_primer_uno = 1;
            }

            if (encontro_primer_uno) {
                cad[idx++] = bit + '0';
            }
        }

        cad[idx] = '\0'; // Cierre de cadena
    }
}


int32_t devuelveNumero(char *cadBinaria, int cantBits) {
    // 1. Usar uint32_t evita el Undefined Behavior al desplazar el bit 31
    uint32_t resultado = 0;

    // Convertimos la cadena de texto a valor entero
    while (*cadBinaria != '\0') {
        if (*cadBinaria == '1') {
            resultado = (resultado << 1) | 1;
        } else if (*cadBinaria == '0') {
            resultado = (resultado << 1);
        }
        cadBinaria++;
    }

    // 2. Propagación de signo (Sign Extension) según la cantidad de bits
    if (cantBits == 8) {
        return (int32_t)(int8_t)resultado;
    } else if (cantBits == 16) {
        return (int32_t)(int16_t)resultado;
    }

    // Para 32 bits, el bit 31 se convierte en el bit de signo
    return (int32_t)resultado;
}

void  sys(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    int i;
    uint32_t direcLogica, valor;

    // Extraemos tamaño (16 bits superiores) y cantidad (16 bits inferiores) de ECX (regTabla[12])
    uint16_t cantBytes = (regTabla[12] >> 16) & 0xFFFF;
    uint8_t cantCeldas = regTabla[12] & 0xFFFF;
    char cadBinario[33];
    int32_t dato;


    valor = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    if (valor == 2) { // WRITE
        for (i = 0; i < cantCeldas; i++) {
            direcLogica = regTabla[13] + cantBytes * i; //EDX apuntando a la direc inicial + corrimiento

            // Leemos el valor de la memoria usando la dirección lógica
            dato = leerMemoria(direcLogica, regTabla, segTabla, memoriaPrincipal, cantBytes);

            uint8_t formatoDecimal  = (regTabla[10] >> 0) & 0x01; // Bit 0
            uint8_t formatoCaracter = (regTabla[10] >> 1) & 0x01; // Bit 1
            uint8_t formatoOctal    = (regTabla[10] >> 2) & 0x01; // Bit 2
            uint8_t formatoHexa     = (regTabla[10] >> 3) & 0x01; // Bit 3
            uint8_t formatoBinario  = (regTabla[10] >> 4) & 0x01; // Bit 4

            uint32_t direcFisica = convertirDirecLogica(direcLogica, segTabla,cantBytes);
            printf("[%04X]: ", direcFisica);
            if(formatoBinario==1){
                devuelveBinario(dato,cadBinario,8*cantBytes);
                printf(" 0b%s ",cadBinario);
            }
            if(formatoHexa==1){
                printf(" 0x%X ",dato);
            }
            if(formatoOctal==1){
                printf(" 0o%o ",dato);
            }
            if (formatoCaracter == 1) {
                // Recorremos los bytes contenidos en 'dato' de mayor a menor jerarquía
                for (int b = cantBytes - 1; b >= 0; b--) {
                    uint8_t caracter;
                    caracter = (dato >> (8 * b)) & 0xFF; // Extrae 1 byte 
                    if (caracter >= 32 && caracter <= 126)
                    printf("%c", (char)caracter);
                    else 
                        printf(".");
                    
                }
            }
            if(formatoDecimal == 1){
                printf(" %d ",dato);
            }
            printf("\n");
        }
    }
    else
        if(valor==1){//READ
            for(i=0;i<cantCeldas;++i){
                direcLogica = regTabla[13] + cantBytes * i;
                uint32_t direcFisica = convertirDirecLogica(direcLogica, segTabla,cantBytes);
                printf("[%04X]: ", direcFisica);
                if(regTabla[10]==0x10){
                    scanf(" %s",cadBinario);
                    dato=devuelveNumero(cadBinario,8*cantBytes);
                }
                else
                    if(regTabla[10]==0x08)
                        scanf(" %X",&dato);
                    else
                        if(regTabla[10]==0x04)
                            scanf(" %o",&dato);
                        else
                            if(regTabla[10]==0x02)
                                scanf(" %c",&dato);
                            else
                                if(regTabla[10]==0x01)
                                    scanf(" %d",&dato);

                escribirMemoria(direcLogica,dato,regTabla,segTabla,memoriaPrincipal,cantBytes);
            }
        }
}

void  jmp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    // 1. Leemos el offset almacenado en reg1, el offset es un operando inmediato
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset

    // 4. Actualizamos IP con la dirección Lógica
    regTabla[0] = direcLogica;
}

void   jp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset

    uint8_t bitNegativo=(regTabla[17]>>31) & 0x1;
    uint8_t bitCero=(regTabla[17]>>30) & 0x1;

    if(bitNegativo==0 && bitCero==0)
        regTabla[0]=direcLogica;
}

void   Jn(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitNegativo=(regTabla[17]>>31) & 0x1;
    if(bitNegativo)
        regTabla[0]=direcLogica;
}

void   jz(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitCero=(regTabla[17]>>30) & 0x1;
    if(bitCero)
        regTabla[0]=direcLogica;
}

void   jc(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitAcarreo=(regTabla[17]>>29) & 0x1;
    if(bitAcarreo)
        regTabla[0]=direcLogica;
}

void   jv(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitDesborda=(regTabla[17]>>28) & 0x1;
    if(bitDesborda)
        regTabla[0]=direcLogica;
}

void  jnp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitNegativo=(regTabla[17]>>31) & 0x1;
    uint8_t bitCero=(regTabla[17]>>30) & 0x1;
    if(bitNegativo || bitCero)
        regTabla[0]=direcLogica;
}

void  jnn(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitNegativo=(regTabla[17]>>31) & 0x1;
    uint8_t bitCero=(regTabla[17]>>30) & 0x1;
    if(bitNegativo==0 || bitCero)
        regTabla[0]=direcLogica;
}

void  jnz(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t offset = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t direcLogica = regTabla[26]+offset;//sumamos CS + offset
    uint8_t bitCero=(regTabla[17]>>30) & 0x1;
    if(bitCero==0)
        regTabla[0]=direcLogica;
}

void  Not(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);

    uint32_t resultado = ~valor; //NO EXISTE OVERFLOW/CARRY EN OPERACIONES LOGICAS

    escribeOperando(reg1, resultado, regTabla, segTabla, memoriaPrincipal);

    actualizarCC_General(valor, 0, regTabla, resultado, 3);
}

void stop(uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
    regTabla[0]=0xFFFFFFFF;
}

void  mov(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    
    uint32_t valor2=leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    
    escribeOperando(reg1,valor2,regTabla,segTabla,memoriaPrincipal);
    
    actualizarCC_General(reg1, valor2, regTabla, valor2, 3);
}

void  add(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 + (uint64_t)valor2; //HAGO LA SUMA SIN SIGNO PARA VER CUANTO DEBE DAR

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    actualizarCC_General(valor1, valor2, regTabla, resultado, 1); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void  sub(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 - (uint64_t)valor2; //HAGO LA RESTA SIN SIGNO PARA VER CUANTO DEBE DAR

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    actualizarCC_General(valor1, valor2, regTabla, resultado, 2); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void  mul(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 * (uint64_t)valor2; //HAGO LA MULTIPLICACION SIN SIGNO PARA NO PERDER BITS DE OVERFLOW

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    actualizarCC_General(valor1, valor2, regTabla, resultado, 5); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void  Div(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    //EVITAR DIVISION POR 0
    if (valor2 == 0) {
        printf("\nERROR: Division por cero.");
        exit(1);
    } else {
        //SE REALIZA LA DIVISION CON SIGNED DE 32 BITS
        int32_t v1_signed = (int32_t)valor1;
        int32_t v2_signed = (int32_t)valor2;

        int32_t cociente_signed = v1_signed / v2_signed;
        int32_t resto_signed    = v1_signed % v2_signed;

        //OBTENEMOS LA VERSION UNSIGNED PARA ESCRIBIR EN REGISTRO
        uint32_t cociente = (uint32_t)cociente_signed;
        uint32_t resto    = (uint32_t)resto_signed;

        escribeOperando(reg1, cociente, regTabla, segTabla, memoriaPrincipal);

        //ACTUALIZAR CC y AC
        actualizarCC_General(v1_signed, v2_signed, regTabla, (int64_t)cociente_signed, 6);
        regTabla[16] = resto;
    }
}

void  cmp(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1 - (uint64_t)valor2;

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, resultado, 2); //LE PASO EL RESULTADO TOTAL PROVISORIO PARA EVALUAR
}

void  And(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint32_t resultado = valor1 & valor2;

    printf("\nRESULTADO: %u", resultado);

    escribeOperando(reg1, resultado, regTabla, segTabla, memoriaPrincipal);

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, resultado, 3);
}

void   Or(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint32_t res = valor1 | valor2;

    escribeOperando(reg1, res, regTabla, segTabla, memoriaPrincipal);

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, res, 3);
}

void  Xor(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint32_t res = valor1 ^ valor2;

    escribeOperando(reg1, res, regTabla, segTabla, memoriaPrincipal);

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, res, 3);
}

void swap(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    Xor(reg1, reg2, regTabla, segTabla, memoriaPrincipal);
    Xor(reg2, reg1, regTabla, segTabla, memoriaPrincipal);
    Xor(reg1, reg2, regTabla, segTabla, memoriaPrincipal);
}

void  shl(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint64_t resultado = (uint64_t)valor1<<valor2;

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, resultado, 7);
}

void  shr(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    uint32_t resultado = valor1>>valor2;

    escribeOperando(reg1, resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, resultado, 8);
}

void  sar(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valor1 = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    int32_t resultado = (int32_t)valor1>>valor2; //casteamos ambos a int32 para que se propague el signo

    escribeOperando(reg1, (uint32_t)resultado, regTabla, segTabla, memoriaPrincipal); //SOLO PASO LOS BITS QUE PUEDE ENTENDER LA VM (sin carry ni overflow)

    //ACTUALIZAR CC
    actualizarCC_General(valor1, valor2, regTabla, resultado, 9);
}

void  ldl(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valorDestino = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valorOrigen  = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    //Conservar los 16 bits superiores de reg1 y reemplazar los 16 bits inferiores con la parte baja de reg2
    uint32_t resultado = (valorDestino & 0xFFFF0000) | (valorOrigen & 0x0000FFFF);

    escribeOperando(reg1, resultado, regTabla, segTabla, memoriaPrincipal);
}

void  ldh(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]) {
    uint32_t valorDestino = leerOperando(reg1, regTabla, segTabla, memoriaPrincipal);
    uint32_t valorOrigen  = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);

    //Conservar los 16 bits inferiores de reg1 y colocar la parte baja de reg2 en los 16 bits superiores
    uint32_t resultado = (valorOrigen << 16) | (valorDestino & 0x0000FFFF);
    
    escribeOperando(reg1, resultado, regTabla, segTabla, memoriaPrincipal);
}

void  rnd(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]){
    int32_t valor2 = leerOperando(reg2, regTabla, segTabla, memoriaPrincipal);
    uint32_t rndVal = rand() % (valor2 + 1);

    if(valor2>0)
        escribeOperando(reg1, rndVal,regTabla, segTabla, memoriaPrincipal);
    else {
        printf("ERROR: Argumento invalido");
        exit(1);
    }
}