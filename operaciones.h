#include <stdint.h>

typedef void (*inst)(void);

typedef struct{
  uint16_t base;
  uint16_t tamaño;
} regSegmento;

uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[]);
void escribirMemoria(uint32_t regOp,uint32_t valor, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]);
uint32_t leerMemoria(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[],uint8_t memoriaPrincipal[]);

void nada(void);

void sys(void);
void jmp(void);
void jp(void);
void Jn(void);// jn es una funcion que existe en c
void jz(void);
void jc(void);
void jv(void);
void jnp(void);
void jnn(void);
void jnz(void);
void Not(void);

void mov(uint32_t reg1,uint32_t reg2, uint32_t regTabla[],regSegmento segTabla[],uint8_t memoriaPrincipal[]);
void add(uint32_t reg1,uint32_t reg2, uint32_t regTabla[],regSegmento segTabla[],uint8_t memoriaPrincipal[]);
void sub(void);
void mul(void);
void Div(void); //no es div por que stdlib tiene una funcion de igual nombre
void cmp(void);
void And(void);
void Or(void);
void Xor(void);
void swap(void);
void shl(void);
void shr(void);
void sar(void);
void ldl(void);
void ldh(void);
void rnd(void);

void stop(void);
