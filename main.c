#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   //INCLUYE EL TIPO uint8_t
#include <string.h>
#include "operaciones.h"


#define CANT_INSTRUCCIONES 48
#define TAM_VERSION1 2
#define TAM_VERSION2 12
#define TAM_ID_VERSION 6

#define TAM_RAM 16384
#define ID "VMX26"
#define VERSION1 1
#define VERSION2 2


// Definición real de la variable global en el main.c
char *nombreArchivoVMI = NULL;
uint16_t tamMemoria;


//DEFINICIONES
int validarID_VERSION(uint8_t[]);
void iniciarTablaSegmentos(regSegmento[], int);
void iniciarRegistros(uint32_t[]);

void asignoRegsOperar(uint32_t regTabla[], uint8_t instruccion, uint8_t memoriaPrincipal[], uint32_t direcFisicaIns);
uint32_t calcularTamInstruccion(uint8_t instruccion);
uint32_t* valorOpGenerico(uint32_t regOp, uint8_t memoriaPrincipal[], regSegmento segTabla[], uint32_t regTabla[] );
void disassembler(uint8_t instruccion,uint32_t direcFisica, uint32_t regOP1,uint32_t regOP2);
void mostrarCC(uint32_t regTabla[]);

void agregarSegmentoEnMemoria(uint8_t memoriaPrincipal[],uint32_t posicion,uint8_t vecSeg[],uint16_t tamSeg);
uint8_t* crearVecParam(int *argc, char *argv[],uint16_t *tamParam,char indiceVMX,char indiceVMI);
//IMPLEMENTACIONES
int validarID_VERSION(uint8_t cabecera[]){
    char id[6];
    uint8_t version=cabecera[5];

    
    memcpy(id,cabecera,5); //copia al vector id los primeros 5 bytes de cabecera
    id[5]='\0';
   
    if (!strcmp(id, ID) && (version==VERSION1 || version==VERSION2))
        return version;
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
void iniciarTablaSegmentos2(regSegmento segmentos[],uint16_t tamCodigo,uint16_t tamExtra,uint16_t tamStack,uint16_t tamConst,uint16_t tamData,uint16_t tamParam){
    uint16_t baseAct = 0; //Acumula los tamaño de los segmentos anteriores para usarlo como base del siguiente segmento
    uint8_t n=0;
    if(tamParam!=0){
        segmentos[n++].tamaño=tamParam;
    }
    if(tamConst!=0){
        segmentos[n++].tamaño=tamConst;
    }
    segmentos[n++].tamaño=tamCodigo;
    if(tamData!=0){
        segmentos[n++].tamaño=tamData;
    }
    if(tamExtra!=0){
        segmentos[n++].tamaño=tamExtra;
    }
    segmentos[n++].tamaño=tamStack;

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

void iniciarRegistros2(uint32_t registros[],uint16_t tamCodigo,uint16_t tamExtra,uint16_t tamStack,uint16_t tamConst,uint16_t tamData,uint16_t offsetEntry,uint16_t tamParam){
    memset(registros, 0, sizeof(uint32_t)*CANT_REGISTROS); // inicializo todo en 0 para mantener limpieza

    uint8_t posTabla=0; //posicion en tabla de descriptores
   
    
    if(tamParam!=0){ //PS
        registros[31]=0x00000000;
        posTabla++;
    }
    else
        registros[31]=0xFFFFFFFF;

    if(tamConst!=0){//KS
        registros[30]=posTabla<<16;
        posTabla++;
    }
    else
        registros[30]=0xFFFFFFFF;
    registros[26]=posTabla<<16; //CS
    posTabla++;
    if(tamData!=0){//DS
        registros[27]=posTabla<<16;
        posTabla++;
    }
    else
        registros[27]=0xFFFFFFFF;
    if(tamExtra!=0){//ES
        registros[28]=posTabla<<16;
        posTabla++;
    }
    else
        registros[28]=0xFFFFFFFF;
    registros[29]=posTabla<<16;//SS

    registros[7]=registros[29]+(tamStack&0xFFFF);//Inicializo el registro SP
    registros[0]=(registros[26]&0xFFFF0000) | (offsetEntry&0xFFFF);


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
    uint8_t tip1,tip2,vecHexa[100],vecAssembler[100][50];
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
        strcpy(vecAssembler[1], assemblerReg[(regOP1 & 0x1F)]);
    else
        if(tip1==2)
            sprintf(vecAssembler[1], "%d", (int16_t)(regOP1 & 0xFFFF));
        else
            if(tip1==3){
                // CORRECCIÓN: Los bits de tamaño están en el desplazamiento 5 (bits 5-7)
                uint8_t sizeBits = (regOP1 >> 5) & 0x07;
                char prefijo = 'l';

                if(sizeBits == 1)      prefijo = 'b'; // 001 -> byte
                else if(sizeBits == 2) prefijo = 'w'; // 010 -> word
                else if(sizeBits == 4) prefijo = 'l'; // 100 -> long

                sprintf(offset, "%d", (int16_t)((regOP1 >> 8) & 0xFFFF));
                sprintf(vecAssembler[1], "%c[%s", prefijo, assemblerReg[(regOP1 & 0x1F)]);
                
                if((int16_t)((regOP1 >> 8) & 0xFFFF) >= 0)
                    strcat(vecAssembler[1], "+");
                    
                strcat(vecAssembler[1], offset);
                strcat(vecAssembler[1], "]");
            }

    if(tip2==1)
        strcpy(vecAssembler[2], assemblerReg[(regOP2 & 0x1F)]);
    else
        if(tip2==2)
            sprintf(vecAssembler[2], "%d", (int16_t)(regOP2 & 0xFFFF));
        else
            if(tip2==3){
                // CORRECCIÓN: Los bits de tamaño están en el desplazamiento 5 (bits 5-7)
                uint8_t sizeBits = (regOP2 >> 5) & 0x07;
                char prefijo = 'l';

                if(sizeBits == 1)      prefijo = 'b'; // 001 -> byte
                else if(sizeBits == 2) prefijo = 'w'; // 010 -> word
                else if(sizeBits == 4) prefijo = 'l'; // 100 -> long

                sprintf(offset, "%d", (int16_t)((regOP2 >> 8) & 0xFFFF));
                sprintf(vecAssembler[2], "%c[%s", prefijo, assemblerReg[(regOP2 & 0x1F)]);
                
                if((int16_t)((regOP2 >> 8) & 0xFFFF) >= 0)
                    strcat(vecAssembler[2], "+");
                    
                strcat(vecAssembler[2], offset);
                strcat(vecAssembler[2], "]");
            }

    printf("\n[%04X] %02X ", direcFisica, vecHexa[0]);

    for(i=1;i<tamInstruccion;++i)
        printf(" %02X ",vecHexa[i]); 

    tamAssembler += tip1!=0;
    tamAssembler += tip2!=0;
    printf("| %s ",vecAssembler[0]);

    for(i=1;i<tamAssembler;++i)
        printf(" %s ",vecAssembler[i]);
}

void mostrarCC(uint32_t regTabla[]){
    uint32_t flags = (regTabla[17] >> 28) & 0x0F;

    printf("CC (NZCV): %u%u%u%u",
        (flags >> 3) & 1,  // Bit N
        (flags >> 2) & 1,  // Bit Z
        (flags >> 1) & 1,  // Bit C
        (flags >> 0) & 1   // Bit V
    );
}

void agregarSegmentoEnMemoria(uint8_t memoriaPrincipal[], uint32_t posicion, uint8_t vecSeg[], uint16_t tamSeg) {
    for (int i = 0; i < tamSeg; i++) {
        memoriaPrincipal[posicion + i] = vecSeg[i];
    }
}

uint8_t* crearVecParam(int *argc, char *argv[], uint16_t *tamParam, char indiceVMX, char indiceVMI) {
    int i;
    int indiceParam = -1;

    // 1. Buscar en argv dónde arranca la opción -p
    for (i = 0; i < *argc; ++i) {
        if (strcmp(argv[i], "-p") == 0) {
            indiceParam = i + 1;//A partir de aca comienzan los parametros que no vmx ni vmi
            break;
        }
    }

    // 2. Construir el vector de palabras (char**) internamente
    int cantPalabras = 0;
    cantPalabras+=indiceVMI!=-1;
    cantPalabras+=indiceVMX!=-1;

    cantPalabras += (*argc - indiceParam);

    if (cantPalabras == 0) {
        *tamParam = 0;
        *argc=0;
        return NULL;
    }

    // Puntero dinámico a la lista de palabras, las palabras son punteros a char y al ser dinamico es un vector de punteros a (punteros a char==palabras)
    char * *vecPalabras = (char **) malloc(cantPalabras * sizeof(char *));
    int posPalabra = 0;

    // Cargar el nombre del ejecutable (.vmx o .vmi)
    if (indiceVMX != -1) {
        vecPalabras[posPalabra++] = argv[(int)indiceVMX];
    }if (indiceVMI != -1) {
        vecPalabras[posPalabra++] = argv[(int)indiceVMI];
    }

    // Cargar las palabras tras el flag -p
    if (indiceParam != -1 && indiceParam < *argc) {
        for (i = indiceParam; i < *argc; ++i) {
            vecPalabras[posPalabra++] = argv[i];
        }
    }

    // 3. Calcular el tamaño en bytes para la sección de textos
    uint16_t tamBytesTexto = 0;
    for (i = 0; i < cantPalabras; ++i) {
        tamBytesTexto += (uint16_t)(strlen(vecPalabras[i]) + 1); // +1 por el '\0'
    }

    uint16_t tamBytesOffsets = cantPalabras * 4; // 4 bytes por offset de cada palabra

    // 4. Crear los 2 PUNTEROS DINÁMICOS auxiliares
    //el vecAuxLetras sera un parte del vector Param, en cada byte ira una letra
    uint8_t *vecAuxLetras = (uint8_t *) calloc(tamBytesTexto, sizeof(uint8_t));
    //el vecAuxOffsets almacenara cada 4 bytes un puntero a una palabra;
    uint8_t *vecAuxOffsets  = (uint8_t *) calloc(tamBytesOffsets, sizeof(uint8_t));

    uint16_t posTexto = 0;
    uint16_t posOffset = 0;

    // 5. Cargar simultáneamente el contenido en ambos vectores dinámicos
    for (i = 0; i < cantPalabras; ++i) {
        uint16_t largo = (uint16_t)strlen(vecPalabras[i]);

        // A) Copia de las letras/caracteres en vecAuxPalabras
        memcpy(&vecAuxLetras[posTexto], vecPalabras[i], largo + 1);//el +1 es por el \0

        // B) Guardar el offset actual (posTexto) empacado en 4 bytes en vecAuxOffsets
        uint32_t offsetActual = (uint32_t)posTexto;
        vecAuxOffsets[posOffset++] = (offsetActual >> 24) & 0xFF;
        vecAuxOffsets[posOffset++] = (offsetActual >> 16) & 0xFF;
        vecAuxOffsets[posOffset++] = (offsetActual >> 8)  & 0xFF;
        vecAuxOffsets[posOffset++] = offsetActual & 0xFF;

        // Avanzar la posición del puntero de texto
        posTexto += (largo + 1);
    }

    // 6. Concatenar todo en el vector final unificado
    uint16_t tamTotal = tamBytesTexto + tamBytesOffsets;
    uint8_t *vecParam = (uint8_t *) calloc(tamTotal, sizeof(uint8_t));

    memcpy(vecParam, vecAuxLetras , tamBytesTexto);
    memcpy(&vecParam[tamBytesTexto], vecAuxOffsets, tamBytesOffsets);

    // 7. Liberar la memoria de los vectores auxiliares
    free(vecPalabras);
    free(vecAuxLetras);
    free(vecAuxOffsets);

    *tamParam = tamTotal;
    *argc=cantPalabras;
    return vecParam;
}
void inicializarSubrutinaPrincipal(int argc,uint16_t tamParam,uint32_t regTabla[],uint8_t memoriaPrincipal[],regSegmento segTabla[]){
    uint32_t direcLogica= (regTabla[7] & 0xFFFF0000) | (uint16_t)( (( regTabla[7] & 0xFFFF) -4)&0xFFFF);//direcLogica que almacena el (SP), como 
                                                                              //Como el SP en un inicio apunta por fuera del SS, le restamos 4 para iniciar la pila 
     regTabla[7]=(regTabla[7] & 0xFFFF0000) | (uint16_t)(((regTabla[7] & 0xFFFF)-4)&0xFFFF);//Actualizo el SP para guardar *argv
   
    //Guardamos el *argv en la pila (consultar bien que seria por que no se armarlo)
    if(tamParam==0){
        escribirMemoria(direcLogica,0xFFFFFFFF,regTabla,segTabla,memoriaPrincipal,4);//Guardo el argv cuando no hay param.
        regTabla[7]=(regTabla[7] & 0xFFFF0000) | (uint16_t)(((regTabla[7] & 0xFFFF)-4)&0xFFFF);//Actualizo el SP para guardar argc
        direcLogica=regTabla[7];
        escribirMemoria(direcLogica, 0x00000000,regTabla, segTabla, memoriaPrincipal, 4);//Escribo argc=0
    }
    else{
        escribirMemoria(direcLogica, 0xFFFFFFFF,regTabla, segTabla, memoriaPrincipal, 4);//Escribo *argv cuando hay param.
        regTabla[7]=(regTabla[7] & 0xFFFF0000) | (uint16_t)(((regTabla[7] & 0xFFFF)-4)&0xFFFF);//Actualizo el SP para guardar argc
        direcLogica=regTabla[7];
        escribirMemoria(direcLogica, argc,regTabla, segTabla, memoriaPrincipal, 4);//Escribo argc
    }
    regTabla[7]=(regTabla[7] & 0xFFFF0000) | (uint16_t)(((regTabla[7] & 0xFFFF)-4)&0xFFFF);//Actualizo el SP para guardar el RET
    //Al ser la subrutina principal, el ret sera 0xFFFFFFFF, por lo que al ejecutar la instruccion RET, el programa se detendra
    direcLogica=regTabla[7];
    escribirMemoria(direcLogica, 0xFFFFFFFF,regTabla, segTabla, memoriaPrincipal, 4);//Escribo el RET
    //Y ahora el SP queda apuntando al RET 
    
}





     




int main(int argc, char *argv[]){
    int condDissasembler=0;
    char indiceVMX = -1;
    char indiceVMI = -1;
    tamMemoria = TAM_RAM;

    int hayParam = 0;
    // Recorremos todos los argumentos ingresados
    for (int i = 1; i < argc; i++) {
        // 1. Si encontramos -p, guardamos el índice y cortamos el bucle
        
        if (strcmp(argv[i], "-p") == 0) {
            hayParam=1;
            break;
        }

        // 2. Si es la opción de desensamblador -d
        if (strcmp(argv[i], "-d") == 0) {
            condDissasembler = 1;
        }
        // 3. Si indica el tamaño de memoria m=M
        else if (strncmp(argv[i], "m=", 2) == 0) {
            tamMemoria = atoi(&argv[i][2]); // Extrae el número después de "m="
        }
        // 4. Si termina en .vmx (es el archivo ejecutable)
        else if (strstr(argv[i], ".vmx") != NULL) {
            indiceVMX = i;
        }
        // 5. Si termina en .vmi (es el archivo de imagen)[cite: 1]
        else if (strstr(argv[i], ".vmi") != NULL) {
            indiceVMI =i;
        }
    }

    // --- VALIDACIÓN DE ERRORES ---
    if (indiceVMX == -1 && indiceVMI == -1) {
        printf("Error: Tenés que pasar al menos un archivo .vmx o .vmi\n");
        return 1;
    }

    if (indiceVMI != -1) {
        // Si el usuario pasó un archivo .vmi, guardamos su ruta/nombre
        // en la variable global para que el SYS 0xF (Breakpoint) pueda usarla.
        nombreArchivoVMI = argv[indiceVMI];
    }

    FILE *archExe = NULL;
    if(indiceVMX != -1) {
        // Si el usuario pasó un archivo .vmx, lo abrimos para lectura binaria
        // y ejecutamos el código dentro de él.
        archExe = fopen(argv[indiceVMX], "rb");
        if (!archExe) {
            printf("Error: No se pudo abrir el archivo %s\n", argv[indiceVMX]);
            return 1;
        }
    }

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

            (inst)push, //0B push
            (inst)pop, //0C pop
            (inst)call, //0D call
            (inst)ret, //0E ret
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

        uint8_t memoriaPrincipal[tamMemoria];
        uint8_t id_version[TAM_ID_VERSION];
        regSegmento segTabla[CANT_SEGMENTOS];
        uint16_t tamCodigo,tamExtra,tamStack,tamConst,tamData,offsetEntry,tamParam;
        uint32_t regTabla[CANT_REGISTROS];

        uint8_t instruccion;
        uint32_t direcFisicaIns, codIns, codOp1, codOp2, op1, op2;


        uint8_t *vecParam=NULL;

        if(archExe){//Recibe un .vmx
            if(hayParam)
                //OJO QUE LUEGO DE EJECUTAR ESTA FUNCION EL ARGC ES LA CANT DE PALABRAS DEL PARAM SEGMENT
                vecParam=crearVecParam(&argc,argv,&tamParam,indiceVMX,indiceVMI);
            
            fread(id_version, sizeof(uint8_t), 6, archExe);
            

            uint8_t version=validarID_VERSION(id_version);
            if(version){
                if(version==1){
                    printf("VERSION 1");
                    uint8_t tamSegmentos[TAM_VERSION1];//el vector almacena los bytes de tamaño del CS
                    fread(tamSegmentos, sizeof(uint8_t), TAM_VERSION1, archExe);
                    tamCodigo = ((uint16_t)tamSegmentos[0]<<8) | tamSegmentos[1];
                    uint8_t vecCodigo[tamCodigo];

                    if(hayParam==0){//no hay parametros
                        segTabla[0].tamaño = tamCodigo;           //CS
                        segTabla[1].tamaño = TAM_RAM - tamCodigo; //DS

                        iniciarTablaSegmentos(segTabla, 2);
                        iniciarRegistros(regTabla);
                        fread(memoriaPrincipal, sizeof(uint8_t), tamCodigo, archExe);
                    }else{//hay parametros y van primero en la memoria principal
                        fread(vecCodigo, sizeof(uint8_t), tamCodigo, archExe);
                        agregarSegmentoEnMemoria(memoriaPrincipal,0,vecParam,tamParam);
                        agregarSegmentoEnMemoria(memoriaPrincipal,tamParam,vecCodigo,tamCodigo);
                        iniciarTablaSegmentos2(segTabla,tamCodigo,0,0,0,0,tamParam);
                        iniciarRegistros2(regTabla,tamCodigo,0,0,0,0,0,tamParam);
                    }
                    
                    if (condDissasembler) {
                        uint32_t direcFisicaCodigo = (hayParam == 0) ? 0 : tamParam;
                        uint32_t cursor = direcFisicaCodigo;
                        uint32_t limite = direcFisicaCodigo + tamCodigo;

                        printf("\n> DISASSEMBLER - INICIO <");
                        while (cursor < limite) {
                            uint8_t instruccion = memoriaPrincipal[cursor];
                            uint32_t regTablaAux[4] = {0};
                            
                            asignoRegsOperar(regTablaAux, instruccion, memoriaPrincipal, cursor);
                            disassembler(instruccion, cursor, regTablaAux[2], regTablaAux[3]);
                            
                            cursor += tamInstruccion(instruccion);
                        }
                        printf("\n> DISASSEMBLER - FIN <\n");
                    }
                    
                    while (((regTabla[0] & 0xFFFF) < tamCodigo) && (regTabla[0] != 0xFFFFFFFF)){ //TRAER Y EJECUTAR HASTA QUE SE TERMINE EL CS O HASTA STOP
                        /*Como IP es un puntero a memoria, tiene en sus 16 bits significativos el codSeg y en el resto un offset
                        por lo que debemos convertir la direccion logica que almacena a una fisica para usarla en el vector de memoria*/
                        direcFisicaIns=convertirDirecLogica(regTabla[0], segTabla,1);
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
                        }else
                            if(codIns>=0x00 && codIns<=0x0A)
                                ((void (*)(uint32_t,uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla[2], regTabla, segTabla, memoriaPrincipal);
                            else
                                if(codIns==0x0F)
                                    ((void (*)(uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla, segTabla, memoriaPrincipal);
                                else{
                                    printf("\nERROR: Instruccion invalida codIns: %X",codIns);
                                    exit(1); //termina de forma abrupta la ejecucion del programa
                                }
                            
                    }
                }
                else{//version2
                    printf("VERSION 2");
                    uint8_t tamSegmentos[TAM_VERSION2];//el vector almacena los bytes de tamaño de los segmentos CS,DS,ES,SS,CS y el offset del entry point
                    fread(tamSegmentos, sizeof(uint8_t), TAM_VERSION2, archExe);
                    tamCodigo = ((uint16_t)tamSegmentos[0]<<8) | tamSegmentos[1];
                    tamData=((uint16_t)tamSegmentos[2]<<8) | tamSegmentos[3];
                    tamExtra=((uint16_t)tamSegmentos[4]<<8) | tamSegmentos[5];
                    tamStack=((uint16_t)tamSegmentos[6]<<8) | tamSegmentos[7];
                    tamConst=((uint16_t)tamSegmentos[8]<<8) | tamSegmentos[9];
                    offsetEntry=((uint16_t)tamSegmentos[10]<<8) | tamSegmentos[11];
                    
                    uint8_t vecCodigo[tamCodigo];

                    fread(vecCodigo, sizeof(uint8_t), tamCodigo, archExe);

                    uint8_t vecConst[tamConst];
                    fread(vecConst, sizeof(uint8_t), tamConst, archExe);
                    iniciarTablaSegmentos2(segTabla,tamCodigo,tamExtra,tamStack,tamConst,tamData,tamParam);
                    
                    iniciarRegistros2(regTabla,tamCodigo,tamExtra,tamStack,tamConst,tamData,offsetEntry,tamParam);
                   
                    uint32_t direcFisicaCodigo = 0;
                    if(tamParam!=0){
                        agregarSegmentoEnMemoria(memoriaPrincipal,direcFisicaCodigo,vecParam,tamParam);
                        direcFisicaCodigo+=tamParam;
                    }
                    if(tamConst!=0){
                        agregarSegmentoEnMemoria(memoriaPrincipal,direcFisicaCodigo,vecConst,tamConst);
                        direcFisicaCodigo+=tamConst;
                    }
                    agregarSegmentoEnMemoria(memoriaPrincipal,direcFisicaCodigo,vecCodigo,tamCodigo);
                    direcFisicaCodigo+=tamCodigo;
                    if(tamData!=0){
                        direcFisicaCodigo+=tamData; //guardo el espacio en memoria del Data segment
                    }
                    if(tamExtra!=0){
                        direcFisicaCodigo+=tamExtra; //guardo el espacio en memoria del Extra segment
                    }
                    //Inicio la pila en la memoria principal.
                    inicializarSubrutinaPrincipal(argc,tamParam,regTabla,memoriaPrincipal,segTabla);
                    
                    
                    if (condDissasembler) {
                        uint32_t direcFisicaCodigo = (hayParam == 0) ? 0 : tamParam;
                        uint32_t cursor = direcFisicaCodigo;
                        uint32_t limite = direcFisicaCodigo + tamCodigo;

                        printf("\n> DISASSEMBLER - INICIO <");
                        while (cursor < limite) {
                            uint8_t instruccion = memoriaPrincipal[cursor];
                            uint32_t regTablaAux[4] = {0};
                            
                            asignoRegsOperar(regTablaAux, instruccion, memoriaPrincipal, cursor);
                            disassembler(instruccion, cursor, regTablaAux[2], regTablaAux[3]);
                            
                            cursor += tamInstruccion(instruccion);
                        }
                        printf("\n> DISASSEMBLER - FIN <\n");
                    }
                    


                    
                    while (((regTabla[0] & 0xFFFF) < tamCodigo) && (regTabla[0] != 0xFFFFFFFF)){ //TRAER Y EJECUTAR HASTA QUE SE TERMINE EL CS O HASTA STOP
                        
                        /*Como IP es un puntero a memoria, tiene en sus 16 bits significativos el codSeg y en el resto un offset
                        por lo que debemos convertir la direccion logica que almacena a una fisica para usarla en el vector de memoria*/
                        direcFisicaIns=convertirDirecLogica(regTabla[0], segTabla,1);
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
                        }else
                            if(codIns>=0x00 && codIns<=0x0D)
                              
                                ((void (*)(uint32_t,uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla[2], regTabla, segTabla, memoriaPrincipal);
                            else
                                if(codIns==0x0F || codIns==0x0E){
                                    
                                    ((void (*)(uint32_t[], regSegmento[], uint8_t[]))vecInstrucciones[codIns])(regTabla, segTabla, memoriaPrincipal);
                                }else{
                                    printf("\nERROR: Instruccion invalida %X",codIns);
                                    exit(1); //termina de forma abrupta la ejecucion del programa
                                }
                            
                            
                    }
                    

                }
            }else
                printf("\nERROR: Cabecera invalida");
            fclose(archExe);
        }else
            printf("\nERROR: Archivo invalido");

}
