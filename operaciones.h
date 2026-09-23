#include <stdint.h>

typedef void (*inst)(void);

typedef struct{
  uint16_t base;
  uint16_t tamaño;
} regSegmento;

uint32_t convertirDirecLogica(uint32_t direcLogica, regSegmento segTabla[]);
uint32_t leerMemoria(uint32_t direcLogica, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[],uint8_t nBytes);
void escribirMemoria(uint32_t direcLogica,uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[],uint8_t nBytes);
uint32_t leerOperando(uint32_t regOp, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void escribeOperando(uint32_t regOp, uint32_t valor, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void actualizarCC_General(int32_t op1, int32_t op2, uint32_t regTabla[], int64_t resultado, int tipo);

void nada(void);

void  sys(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  jmp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void   jp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void   Jn(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void   jz(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void   jc(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void   jv(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  jnp(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  jnn(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  jnz(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  Not(uint32_t reg1, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);

void mov(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void add(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void sub(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void mul(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void Div(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void cmp(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void And(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void  Or(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void Xor(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void swap(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void shl(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void shr(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void sar(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void ldl(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void ldh(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
void rnd(uint32_t reg1, uint32_t reg2, uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);

void stop(uint32_t regTabla[], regSegmento segTabla[], uint8_t memoriaPrincipal[]);
