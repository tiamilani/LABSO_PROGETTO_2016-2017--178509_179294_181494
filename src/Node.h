#ifndef NODE_H
#define NODE_H

#define SINGLELEN 1
#define MINLEN 10
#define PIPELEN 25
#define MAXLEN 100
#define LONGLEN 100000

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int contPid = 1;
int longlen = 100000;

typedef struct Node Node;

struct Node{
	int pid;
	int systemPid;
	char *name;
	int nFigli;
	int nFigliMorti;
	Node* father;
	Node** figli;
	Node** figliMorti;
	int idPipeScrittura;
	int idPipeLettura;
	char *nomePipeScrittura;
	char *nomePipeLettura;
	int numCloni;
	int morto;
};

Node *padre;

Node* init();
Node* cercaPid(Node*, int);
char* phelp();
int plist(Node*, char*);
int pnew(Node*, char*, int);
int pinfo(Node*,char *);
int prePClose(Node*, char*);
int quit(Node*);
int pspawn(Node*, char*, int, Node*);
int prmall(Node*, char*, int);
void ptree(Node*, int, char*);

#endif
