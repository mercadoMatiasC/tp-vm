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
}regSegmento;


//DEFINICIONES
int validarCabecera(uint8_t[]);
void iniciarTablaSegmentos(regSegmento[],int);
void iniciarRegistros(uint32_t[],uint16_t);
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

void iniciarTablaSegmentos(regSegmento segmentos[],int n){

    uint16_t baseAct = 0;

    for (int i = 0; i < n; i++){  // Calculo la base actual de cada segmento de acuerdo al tamaño anterior
        segmentos[i].base = baseAct;
        baseAct += segmentos[i].tamaño;
    }
    for(int i = n; i < CANT_SEGMENTOS; i++){ // Resto del vector completo en -1
        segmentos[i].base = 0xFFFF;
        segmentos[i].tamaño = 0xFFFF;
    }
}

void iniciarRegistros(uint32_t registros[],uint16_t tamCodigo){

    memset(registros, 0, sizeof(uint32_t)*CANT_REGISTROS); // inicializo todo en 0 para mantener limpieza

    registros[26] = 0x00000000;                             // defino el registro CS, 4High = id segmento, 4Low offset
    registros[27] = (0x0001 << 16) | tamCodigo;             // defino el registro DS, 4High = id segmento, 4Low offset
    registros[0] = registros[26];                           // defino IP apuntando al primer byte del segmento de codigo

    //Nose si se requieren mas inicializaciones, dejo abierto a actualizacion;
}

int main(){
    FILE *archExe = fopen("ejemplo.vmx", "rb");

    //inst vecInstrucciones[CANT_INSTRUCCIONES]={mov};

    int8_t memoriaPrincipal[TAM_RAM];
    uint8_t cabecera[TAM_CABECERA]; //vector de 8 bytes
    regSegmento segTabla[CANT_SEGMENTOS];
    uint16_t tamCodigo;
    uint32_t regTabla[CANT_REGISTROS];

    if(archExe){
        fread(cabecera, sizeof(uint8_t), 8,archExe);
        if(validarCabecera(cabecera)){
            tamCodigo = ( (uint16_t)cabecera[6]<<8) | cabecera[7];
            segTabla[0].tamaño = tamCodigo;
            segTabla[1].tamaño = TAM_RAM - tamCodigo;

            iniciarTablaSegmentos(segTabla,2);

            iniciarRegistros(regTabla,tamCodigo);

            fread(memoriaPrincipal,sizeof(uint8_t),tamCodigo,archExe);
            
            
            
                
                
                
                

            


        }
        fclose(archExe);
    }


    return 0;
}
