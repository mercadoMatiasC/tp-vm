#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>
#include "operaciones.h"

#define CANT_INSTRUCCIONES 26 
#define TAM_CABECERA 8
#define CANT_SEGMENTOS 8
#define TAM_RAM 16384
#define ID "VMX26"
#define VERSION 1
#define CANT_REGISTROS 32

typedef struct{
  uint16_t base;
  uint16_t tamaño;
} regSegmento;


//DEFINICIONES
int validarCabecera(uint8_t[]);
void iniciarTablaSegmentos(regSegmento[], int);
void iniciarRegistros(uint32_t[]);
uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[]);
void asignoRegsOperar(uint32_t regTabla[], uint8_t instruccion, uint8_t memoriaPrincipal[], uint32_t direcFisicaIns);
uint32_t calcularTamInstruccion(uint8_t instruccion);
uint32_t* valorOpGenerico(uint32_t regOp, uint8_t memoriaPrincipal[], regSegmento segTabla[], uint32_t regTabla[] );


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

uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[]){
    int codSeg=(direcLogica>>16)&0x0000FFFF;  //La mascara es por si cambiamos direcLogica a int para evitar extension de signo
    int offset=direcLogica & 0x0000FFFF;      //Obtengo el desplazamiento de la direcLogica
    uint32_t direcBase=segTabla[codSeg].base; //Obtengo de la tabla de descriptores la direc base

    return direcBase + offset;
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

uint32_t* valorOpGenerico(uint32_t regOp, uint8_t memoriaPrincipal[], regSegmento segTabla[], uint32_t regTabla[] ){
    uint32_t codOp=(regOp>>24) & 0x000000FF;
    uint32_t codReg=regOp & 0x0000001F;
    uint32_t offset, direcLogica, direcFisica;

    if(codOp==1) //operando de registro
        return &regTabla[codReg];
    else{ //operando de memoria
        offset=(regOp>>8) & 0x0000FFFF;
        direcLogica=offset+regTabla[codReg];
        direcFisica=convertirDirecLogica(direcLogica, segTabla);

        // Castea el puntero de uint8_t* a uint32_t* para acceder a 4 bytes continuos
        return (uint32_t *)&memoriaPrincipal[direcFisica];
    }
}


int main(){
    FILE *archExe = fopen("ejemplo.vmx", "rb");

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
        (inst)not,  //0A

        (inst)nada, //0B
        (inst)nada, //0C
        (inst)nada, //0D
        (inst)nada, //0E
        (inst)nada, //0F

        (inst)mov,  //10
        (inst)add,  //11
        (inst)sub,  //12
        (inst)mul,  //13
        (inst)div   //14
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

                //Actualizo IP
                regTabla[0]+=tamInstruccion(instruccion);
                
                printf("\n[%04X] Instruccion:%X | op1: %08X | op2: %08X", direcFisicaIns, instruccion, regTabla[3], regTabla[2]);

                //Ejecuto la instruccion
                codIns=instruccion & 0x1F;
                //Aclaracion los Op1 y Op2 son los valores con los que realizaremos la instruccion
                //mas no significa que coincidan con lo que guardan los registros OP1 y OP2

                if(codIns>=0x10 && codIns<=0x1F){  //Instruccion de 2 operandos
                    codOp1=(regTabla[2]>>24)&0x000000FF;
                    codOp2=(regTabla[3]>>24)&0x000000FF;

                    if(codOp2==2)
                        op2=regTabla[3] & 0x00FFFFFF;
                    else
                        op2=*(valorOpGenerico(regTabla[3], memoriaPrincipal, segTabla, regTabla));
                    
                
                ((void (*)(uint32_t*, uint32_t))vecInstrucciones[codIns])(valorOpGenerico(regTabla[2],memoriaPrincipal,segTabla,regTabla), op2);
                }

                printf("\nEAX: 0x%X", regTabla[10]); //VER REGISTRO EAX
            }
        }else
            printf("CABECERA INVALIDA!");

        fclose(archExe);
    }

    return 0;
}