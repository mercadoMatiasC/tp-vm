#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> //INCLUYE EL TIPO uint8_t
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
        fread(id, 1, 5, archExe);
        fread(&version, 1, 1, archExe);
        fread(&codeSize, 1, 2, archExe);

        if (!strcmp(id, IDENTIFICADOR)){
            printf("ARCHIVO VALIDO!: ID: %s | VER: %d | SIZE: %d");
            return 1;
        }else
            return 0;
    }
}