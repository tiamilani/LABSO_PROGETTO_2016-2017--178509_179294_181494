#ifndef NODE_H
#define NODE_H

#define MINLEN 10
#define PIPELEN 25
#define MAXLEN 100
#define LONGLEN 100000

#define ANSI_COLOR_RED_BG     	"\x1b[41m"
#define ANSI_COLOR_GREEN_BG		"\x1b[42m"
#define ANSI_COLOR_YELLOW_BG  	"\x1b[43m"
#define ANSI_COLOR_BLUE_BG    	"\x1b[44m"
#define ANSI_COLOR_MAGENTA_BG 	"\x1b[45m"
#define ANSI_COLOR_CYAN_BG    	"\x1b[46m"
#define ANSI_COLOR_RESET_BG   	"\x1b[0m"

int contPid = 1;
int longlen = 100000;

typedef struct Node Node;

struct Node{
	int pid;
	int systemPid;
	char *name;
	int nFigli;
	Node* father;
	Node** figli;
	int idPipeScrittura;
	int idPipeLettura;
	char *nomePipeScrittura;
	char *nomePipeLettura;
	int numCloni;
};

Node* init();
char* phelp();
void plist(Node*, char*);
int pnew(Node*, char*, int);
int pinfo(Node*,char *);
int prePClose(Node*, char*);
int quit(Node*);
int pspawn(Node*, char*, int);
int prmall(Node*, char*);
void ptree(Node*, int, char*);

#endif
