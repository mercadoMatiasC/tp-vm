#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>

#define TAMCABECERA 8
#define RAM_SIZE 16384
#define IDENTIFICADOR "VMX26"

uint8_t MAIN_MEMORY[RAM_SIZE];

//DEFINICIONES
int validarCabecera(uint8_t cabecera[]);

//IMPLEMENTACIONES
int validarCabecera(uint8_t cabecera[]){

    char id[6];
    uint8_t version=cabecera[5];
    uint16_t codeSize= ( (uint16_t)cabecera[6]<<8) | cabecera[7];

    memcpy(id,cabecera,5);
    id[5]='\0'; 

    if (!strcmp(id, IDENTIFICADOR)){
        printf("ARCHIVO VALIDO!: ID: %s | VER:  %u | SIZE:  %u", id, version, codeSize);
        return 1;
    }else
        return 0;
}

int main(){
    FILE *archExe = fopen("main.vmx", "rb");
    char id[6];
    uint8_t cabecera[TAMCABECERA]; //vector de 8 bytes

    if(archExe){
        fread(cabecera, sizeof(uint8_t), 8,archExe);
        if(validarCabecera(cabecera))
            printf("Valido");
    }
    

    return 0;
}
