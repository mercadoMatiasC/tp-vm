#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>

#define TAMCABECERA 8
#define RAM_SIZE 16384
#define IDENTIFICADOR "VMX26"
#define VERSION 1



//DEFINICIONES
int validarCabecera(uint8_t cabecera[]);
//IMPLEMENTACIONES
int validarCabecera(uint8_t cabecera[]){

    char id[6];
    uint8_t version=cabecera[5];

    memcpy(id,cabecera,5); //copia al vector id los primeros 5 bytes de cabecera
    id[5]='\0'; 

    if (!strcmp(id, IDENTIFICADOR) && version==VERSION){
        return 1;
    }else
        return 0;
}

int main(){
    FILE *archExe = fopen("main.vmx", "rb");
    uint8_t mainMemory[RAM_SIZE];
    uint8_t cabecera[TAMCABECERA]; //vector de 8 bytes
    uint16_t codesize;

    if(archExe){
        fread(cabecera, sizeof(uint8_t), 8,archExe);
        if(validarCabecera(cabecera)){
            uint16_t codeSize= ( (uint16_t)cabecera[6]<<8) | cabecera[7];
            fread(mainMemory,sizeof(uint8_t),codesize,archExe);
            printf("%x\n",mainMemory[0]);
            //fread
        }
    }
    

    return 0;
}
