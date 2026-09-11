typedef void (*inst)(void);

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
void not(void);

void mov(void);
void add(void);
void sub(void);
void mul(void);
void Div(void); //no es div por que stdlib tiene una funcion de igual nombre
void cmp(void);
void and(void);
void or(void);
void xor(void);
void swap(void);
void shl(void);
void shr(void);
void sar(void);
void ldl(void);
void ldh(void);
void rnd(void);

void stop(void);