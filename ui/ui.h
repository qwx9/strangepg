typedef struct Event Event;

enum{
	EVkbd,
	EVmouse,
	EVnil,
};
struct Event{
	int type;
	int key;
	int down;
};

int	panview(Vertex);

void	initsysui(void);
void	initui(void);
void	evloop(void);
Event	waitevent(void);
void	show(void);

int	mouseprompt(char*, char*, int);
void	errmsg(char*, ...);
