typedef struct EM EM;

#pragma incomplete EM

int	em2fs(EM*, File*, ssize);
uchar*	emptr(EM*, vlong);
usize	emr64(EM*, vlong);
void	emw64(EM*, vlong, u64int);
void	emclose(EM*);
EM*	emopen(char*, int);
