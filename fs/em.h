typedef struct EM EM;
typedef vlong EMstring;

#pragma incomplete EM

extern int multiplier;

int	em2fs(EM*, File*, ssize);
uchar*	emptr(EM*, vlong);
u64int	emr64(EM*, vlong);
void	emw64(EM*, vlong, u64int);
void	emwrite(EM*, vlong, uchar*, int);
uchar*	emread(EM*, vlong, int, vlong*);
char*	emgetstring(EMstring);
EMstring	emputstring(char*, int);
void	emclose(EM*);
EM*	emopen(char*);
void	initem(void);
