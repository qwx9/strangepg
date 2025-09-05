typedef int EM;
typedef	vlong EMstring;

extern int multiplier;

u64int	emr64(EM, vlong);
u32int	emr32(EM, vlong);
u16int	emr16(EM, vlong);
u8int	emr8(EM, vlong);
void	emw64(EM, vlong, u64int);
void	emw32(EM, vlong, u32int);
void	emw16(EM, vlong, u16int);
void	emw8(EM, vlong, u8int);
void	emreadi(EM, vlong, void*, int);
void	emwritei(EM, vlong, void*, int);
char*	emgetstr(EMstring);
EMstring	emputstr(char*, int);
void	emclose(EM);
EM	emopen(char*);
void	initem(void);
