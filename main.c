#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>

#define RAM_SIZE 16384
#define IDENTIFICADOR "VMX26"

uint8_t MAIN_MEMORY[RAM_SIZE];

//DEFINICIONES
int validarCabecera(char nombre[]);

//IMPLEMENTACIONES
int validarCabecera(char nombre[]){
    FILE *archExe = fopen(nombre, "rb");

    //CABECERA
    char id[6];
    uint8_t version;
    uint16_t codeSize;

    if (archExe){
        fread(id, 5, 1, archExe);
        fread(&version, sizeof(uint8_t), 1, archExe);
        fread(&codeSize, sizeof(uint16_t), 1, archExe);

        codeSize = (codeSize << 8) | (codeSize >> 8); //CAMBIO DE CODIFICACION

        if (!strcmp(id, IDENTIFICADOR)){
            printf("ARCHIVO VALIDO!: ID: %s | VER:  %u | SIZE:  %u", id, version, codeSize);
            return 1;
        }else
            return 0;
    }
}

int main(){
    int respuesta = validarCabecera("main.vmx");

    return 0;
}
