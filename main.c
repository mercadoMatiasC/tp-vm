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
#define CANT_REGISTROS 17



//DEFINICIONES
int validarCabecera(uint8_t cabecera[]);
void iniciarTablaSegmentos(int32_t tabla[],uint16_t tamCodigo);
void iniciarRegTabla(int32_t regTabla[]);


//IMPLEMENTACIONES
int validarCabecera(uint8_t cabecera[]){

    char id[6];
    uint8_t version=cabecera[5];

    memcpy(id,cabecera,5); //copia al vector id los primeros 5 bytes de cabecera
    id[5]='\0'; 

    if (!strcmp(id, ID) && version==VERSION){
        return 1;
    }else
        return 0;
}


int main(){
    FILE *archExe = fopen("ejemplo.vmx", "rb");

    //inst vecInstrucciones[CANT_INSTRUCCIONES]={mov};

    int8_t memoriaPrincipal[TAM_RAM];
    uint8_t cabecera[TAM_CABECERA]; //vector de 8 bytes
    int8_t instruccion;
    uint8_t codOp,tipoOp1,tipoOp2,tamInstruccion;
    uint32_t Op1,Op2;
    uint32_t segTabla[CANT_SEGMENTOS];
    uint16_t tamCodigo;
    int32_t regTabla[CANT_REGISTROS];

    int i;

    if(archExe){
        fread(cabecera, sizeof(uint8_t), 8,archExe);
        if(validarCabecera(cabecera)){
            tamCodigo = ( (uint16_t)cabecera[6]<<8) | cabecera[7];
            fread(memoriaPrincipal,sizeof(uint8_t),tamCodigo,archExe);
            
            
            
                
                
                
                

            


        }
        fclose(archExe);
    }
    

    return 0;
}
