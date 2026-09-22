#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>
#include "operaciones.h"

#define CANT_INSTRUCCIONES 48
#define TAM_CABECERA 8
#define CANT_SEGMENTOS 8
#define TAM_RAM 16384
#define ID "VMX26"
#define VERSION 1
#define CANT_REGISTROS 32

//DEFINICIONES
int validarCabecera(uint8_t[]);
void iniciarTablaSegmentos(regSegmento[], int);
void iniciarRegistros(uint32_t[]);

void asignoRegsOperar(uint32_t regTabla[], uint8_t instruccion, uint8_t memoriaPrincipal[], uint32_t direcFisicaIns);
uint32_t calcularTamInstruccion(uint8_t instruccion);
uint32_t* valorOpGenerico(uint32_t regOp, uint8_t memoriaPrincipal[], regSegmento segTabla[], uint32_t regTabla[] );
void disassembler(uint8_t instruccion,uint32_t direcFisica, uint32_t regOP1,uint32_t regOP2);
void mostrarCC(uint32_t regTabla[]);



//IMPLEMENTACIONES
int validarCabecera(uint8_t cabecera[]){
    char id[6];
    uint8_t version=cabecera[5];

    memcpy(id,cabecera,5); //copia al vector id los primeros 5 bytes de cabecera
    id[5]='\0';

    if (!strcmp(id, ID) && version==VERSION)
        return 1;
    else
        return 0;
}

void iniciarTablaSegmentos(regSegmento segmentos[], int n){
    uint16_t baseAct = 0; //Acumula los tamaño de los segmentos anteriores para usarlo como base del siguiente segmento

    for (int i = 0; i < n; i++){  // Calculo la base actual de cada segmento de acuerdo al tamaño anterior
        segmentos[i].base = baseAct;
        baseAct += segmentos[i].tamaño;
    }

    for(int i = n; i < CANT_SEGMENTOS; i++){ // Resto del vector completo en -1
        segmentos[i].base = 0xFFFF;
        segmentos[i].tamaño = 0xFFFF;
    }
}

void iniciarRegistros(uint32_t registros[]){
    memset(registros, 0, sizeof(uint32_t)*CANT_REGISTROS); // inicializo todo en 0 para mantener limpieza

    registros[26] = 0x00000000;               // defino el registro CS, 4High = id segmento,
    registros[27] = 0x0001 << 16;             // defino el registro DS, 4High = id segmento,
    registros[0]  = registros[26];            // defino IP apuntando al primer byte del segmento de codigo

    //Nose si se requieren mas inicializaciones, dejo abierto a actualizacion;
}


void asignoRegsOperar(uint32_t regTabla[], uint8_t instruccion, uint8_t memoriaPrincipal[], uint32_t direcFisicaIns){
    uint32_t auxTipo,tip1,tip2,op1,op2;
    int tamInstruccion=1,i;

    regTabla[1]=instruccion & 0x1F; //Al registro OPC le asigno el codigo de la instruccion
    auxTipo=(instruccion>>4) & 0x03; //Guardo el tipo que esta en los bits 5 y 4

    if(auxTipo==0x00){//instruccion de 1 o ningun operando
        tip1=(instruccion>>6) & 0x03; //Como los bits 5 y 4 son cero, a operandoA/registro OP1 le corresponden los bits 7 y 6
        tip2=auxTipo;
    }else{//instruccion de 2 operandos
        tip1=auxTipo;//Guardo el tipo del operando A/registro OP1
        tip2=(instruccion>>6) & 0x03; //Guardo el tipo del operando B/registro OP2
                                      // Aclaracion: la mascara es por si a regTabla pasa a ser int32_t
    }

    regTabla[2]=tip1<<24;//Dejo en los 8 bits mas significativos el tipo de operando
    regTabla[3]=tip2<<24;//Si el tipo fuera cero al desplazar 24 bits, todo el registro queda en cero

    //Por ultimo se asignan los valores de los operandos
    op1=0x0;
    op2=0x0; //si sus tipos son cero entonces los operandos seran cero

    for(i=0;i<tip2;i++){
        op2=(op2<<8) | (uint8_t)memoriaPrincipal[direcFisicaIns+tamInstruccion];//Lee un byte de la memoria principal
        //el casteo es por si luego memoria queda con int32
        tamInstruccion++;//El tamanio de la instruccion incremento un byte
    }

    for(i=0;i<tip1;i++){
        op1=(op1<<8) | (uint8_t)memoriaPrincipal[direcFisicaIns+tamInstruccion];
        tamInstruccion++;
    }

    regTabla[2]=regTabla[2] | op1;
    regTabla[3]=regTabla[3] | op2;
}

uint32_t tamInstruccion(uint8_t instruccion){
    uint32_t tamAux=1; //De por si la instruccion es minimo de 1 byte por el codigo de operacion
    uint32_t tipo1, tipo2;

    tipo1=(instruccion>>4) & 0x03; //Guardo el tipo que esta en los bits 4 y 5
    tipo2=(instruccion>>6) & 0x03; //Guardo el tipo que esta en los bits 6 y 7
    tamAux+=tipo1+tipo2;

    return tamAux;
}

void disassembler(uint8_t instruccion,uint32_t direcFisica, uint32_t regOP1,uint32_t regOP2){
    uint8_t tip1,tip2,vecHexa[100],vecAssembler[100][8];
    int tamInstruccion=1,i,index=1,tamAssembler=1;
    char offset[16];
    char assemblerReg[32][8]={
        "IP",
        "OPC",
        "OP1",
        "OP2",
        "LAR",
        "MAR",
        "MBR",
        "NADA",
        "NADA",
        "NADA",
        "EAX",
        "EBX",
        "ECX",
        "EDX",
        "EEX",
        "EFX",
        "AC",
        "CC",
        "NADA",
        "NADA",
        "NADA",
        "NADA",
        "NADA",
        "NADA",
        "NADA",
        "NADA",
        "CS",
        "DS",
        "NADA",
        "NADA",
        "NADA",
        "NADA"
    };
    char assemblerOp[32][8]={
            "SYS",  //00
            "JMP",  //01
            "JP",   //02
            "JN",   //03
            "JZ",   //04
            "JC",   //05
            "JV",   //06
            "JNP",  //07
            "JNN",  //08
            "JNZ",  //09
            "NOT",  //0A

            "NADA", //0B
            "NADA", //0C
            "NADA", //0D
            "NADA", //0E
            "STOP", //0F

            "MOV",  //10
            "ADD",  //11
            "SUB",  //12
            "MUL",  //13
            "DIV",   //14
            "CMP",  //15
            "AND",  //16
            "OR",   //17
            "XOR",  //18
            "SWAP", //19
            "SHL",  //1A
            "SHR",  //1B
            "SAR",  //1C
            "LDL",  //1D
            "LDH",  //1E
            "RND"   //1F
    };
    tip1=regOP1>>24;
    tip2=regOP2>>24;

    vecHexa[0]=instruccion;
    strcpy(vecAssembler[0],assemblerOp[instruccion&0x1F]);//segun el codOp la instruccion que almacena
    for(i=0;i<tip2;i++){
        vecHexa[i+1]=(regOP2>>((tip2-1-i)*8))&0xFF; //almaceno a partir del byte 1 del regOP1
        tamInstruccion++;//El tamanio de la instruccion incremento un byte
    }
    for(i=0;i<tip1;i++){
        vecHexa[i+1+tip2]=(regOP1>> ((tip1-1-i)*8)) &0xFF;
        tamInstruccion++;
    }

    if(tip1==1)
        strcpy(vecAssembler[1],assemblerReg[(regOP1 & 0x1F)]);
    else
        if(tip1==2)
            sprintf(vecAssembler[1],"%d",(int16_t)(regOP1 & 0xFFFF));
        else
            if(tip1==3){
                sprintf(offset,"%d",(regOP1 >>8) & 0xFFFF);
                strcpy(vecAssembler[1],"[");
                strcat(vecAssembler[1],assemblerReg[(regOP1 & 0x1F)]);
                strcat(vecAssembler[1],"+");
                strcat(vecAssembler[1],offset);
                strcat(vecAssembler[1],"]");
            }

    if(tip2==1)
        strcpy(vecAssembler[2],assemblerReg[(regOP2 & 0x1F)]);
    else
        if(tip2==2)
            sprintf(vecAssembler[2],"%d",(int16_t)(regOP2 & 0xFFFF));
        else
            if(tip2==3){
                sprintf(offset,"%d",(regOP2 >>8) & 0xFFFF);
                strcpy(vecAssembler[2],"[");
                strcat(vecAssembler[2],assemblerReg[(regOP2 & 0x1F)]);
                strcat(vecAssembler[2],"+");
                strcat(vecAssembler[2],offset);
                strcat(vecAssembler[2],"]");
            }


    printf("\n[%04X] codIns: %02X ",direcFisica,vecHexa[0]);
    for(i=1;i<tamInstruccion;++i){
        printf(" %02X ",vecHexa[i]);
    }

    tamAssembler+= tip1!=0;
    tamAssembler+= tip2!=0;
    printf("| %s ",vecAssembler[0]);

    for(i=1;i<tamAssembler;++i){
        printf(" %s ",vecAssembler[i]);
    }
    printf("\n");
}

void mostrarCC(uint32_t regTabla[]){
    uint32_t flags = (regTabla[17] >> 28) & 0x0F;

    printf("\nCC (NZCV): %u%u%u%u",
        (flags >> 3) & 1,  // Bit N
        (flags >> 2) & 1,  // Bit Z
        (flags >> 1) & 1,  // Bit C
        (flags >> 0) & 1   // Bit V
    );
}


int main(int argc, char *argv[]){
    int condDissasembler;
    if(argc<=3){
        condDissasembler = (argc >2 && !(strcmp(argv[2],"-d")));
        FILE *archExe = fopen(argv[1], "rb");

        inst vecInstrucciones[CANT_INSTRUCCIONES] = {
            (inst)sys,  //00
            (inst)jmp,  //01
            (inst)jp,   //02
            (inst)Jn,   //03
            (inst)jz,   //04
            (inst)jc,   //05
            (inst)jv,   //06
            (inst)jnp,  //07
            (inst)jnn,  //08
            (inst)jnz,  //09
            (inst)Not,  //0A

            (inst)nada, //0B
            (inst)nada, //0C
            (inst)nada, //0D
            (inst)nada, //0E
            (inst)stop, //0F

            (inst)mov,  //10
            (inst)add,  //11
            (inst)sub,  //12
            (inst)mul,  //13
            (inst)Div,  //14
            (inst)cmp,  //15
            (inst)And,  //16
            (inst)Or,   //17
            (inst)Xor,  //18
            (inst)swap, //19
            (inst)shl,  //1A
            (inst)shr,  //1B
            (inst)sar,  //1C
            (inst)ldl,  //1D
            (inst)ldh,  //1E
            (inst)rnd   //1F
        };

        uint8_t memoriaPrincipal[TAM_RAM];
        uint8_t cabecera[TAM_CABECERA]; //vector de 8 bytes
        regSegmento segTabla[CANT_SEGMENTOS];
        uint16_t tamCodigo;
        uint32_t regTabla[CANT_REGISTROS];

        uint8_t instruccion;
        uint32_t direcFisicaIns, codIns, codOp1, codOp2, op1, op2;

        if(archExe){
            fread(cabecera, sizeof(uint8_t), 8, archExe);

            if(validarCabecera(cabecera)){
                tamCodigo = ((uint16_t)cabecera[6]<<8) | cabecera[7];
                segTabla[0].tamaño = tamCodigo;           //CS
                segTabla[1].tamaño = TAM_RAM - tamCodigo; //DS

                iniciarTablaSegmentos(segTabla, 2);
                iniciarRegistros(regTabla);

                fread(memoriaPrincipal, sizeof(uint8_t), tamCodigo, archExe);

                while (((regTabla[0] & 0xFFFF) < tamCodigo) && (regTabla[0] != 0xFFFFFFFF)){ //TRAER Y EJECUTAR HASTA QUE SE TERMINE EL CS O HASTA STOP
                    /*Como IP es un puntero a memoria, tiene en sus 16 bits significativos el codSeg y en el resto un offset
                    por lo que debemos convertir la direccion logica que almacena a una fisica para usarla en el vector de memoria*/
                    direcFisicaIns=convertirDirecLogica(regTabla[0], segTabla);
                    instruccion=memoriaPrincipal[direcFisicaIns];

                    //Le asigna a los registros OPC,OP1 Y OP2 sus correspondientes valores
                    asignoRegsOperar(regTabla, instruccion, memoriaPrincipal, direcFisicaIns);


                    //Actualizo IP, si ocurre un salto se modifica en la misma funcion de salto
                    regTabla[0]+=tamInstruccion(instruccion);

                    //Ejecuto la instruccion
                    codIns=instruccion & 0x1F;

                    if(codIns>=0x10 && codIns<=0x1F){  //Instruccion de 2 operandos
                        codOp1=(regTabla[2]>>24)&0x000000FF;

                        ((void (*)(uint32_t, uint32_t,uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla[2], regTabla[3], regTabla, segTabla, memoriaPrincipal);
                        if(codOp1==3){//memoria
                            uint32_t regOp = regTabla[2]; // O el regOp que corresponda
                            uint32_t codReg = regOp & 0x0000001F;
                            uint32_t offset = (regOp >> 8) & 0x0000FFFF;
                            uint32_t direcLogica = offset + regTabla[codReg];

                            uint32_t valorEscrito = leerMemoria(direcLogica, regTabla, segTabla, memoriaPrincipal, 4);
                            printf("\nValor recien escrito en memoria: %d (Hex:0x%08X)", valorEscrito, valorEscrito);
                        }
                    }else
                        if(codIns>=0x00 && codIns<=0x0A)
                            ((void (*)(uint32_t,uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla[2], regTabla, segTabla, memoriaPrincipal);
                        else
                            if(codIns==0x0F)
                                 ((void (*)(uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla, segTabla, memoriaPrincipal);
                            else{
                                printf("\nERROR: Instruccion invalida");
                                exit(1); //termina de forma abrupta la ejecucion del programa
                            }

                    //DEBUG
                    disassembler(instruccion, direcFisicaIns, regTabla[2], regTabla[3]);

                    printf("ECX: %d\n", (int32_t)regTabla[12]);
                    printf("EAX: %d\n", (int32_t)regTabla[10]);
                    //printf("\nAC: %u", regTabla[16]);
                    mostrarCC(regTabla);

                    printf("\n\n-------------------------------------------------------\n");
                    //DEBUG
                }
            }else
                printf("\nERROR: Cabecera invalida");
        }else
            printf("\nERROR: Archivo invalido");
        fclose(archExe);
    }else
        printf("\nERROR: Cantidad de argumentos invalidos");
    return 0;
}
