#ifndef NODE_H
#define NODE_H

#define MINLEN 10
#define PIPELEN 25
#define MAXLEN 100
#define LONGLEN 100000

int contPid = 1;
int longlen = 100000;

typedef struct Node Node;

struct Node{
	int pid;
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
int pspawn(Node*, char*);
int prmall(Node*, char*);
void ptree(Node*, int, char*);

#endif
