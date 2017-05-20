//Inserisco il nome del nodo attuale
	strcat(ch, getName(nodo));
	if(nodo->morto == 1)
		strcat(ch, " (chiuso)");
	strcat(ch, "\n");

	//Aggiungo il nome di tutti i miei figli
	int i;
	for(i = 0; i < nodo->nFigli; i++)
		plist(nodo->figli[i], ch);
	
	strcat(ch,ANSI_COLOR_RED);
	
	printf("Numero di figli morti: %d\n",nodo->nFigliMorti);
	
	for(i = 0; i < nodo->nFigliMorti; i++)
		plist(nodo->figliMorti[i], ch);
		
	strcat(ch,ANSI_COLOR_RESET);
	
	
//Funzione per spostare il puntatore di un figlio alla sezione dei processi morti
int spostaSulVettoreDeiMorti(int i,Node* nodo) {
	//Vettore d'appoggio
	Node** vettoreFigli = (Node**)calloc(nodo->nFigliMorti, sizeof(Node));
	if(vettoreFigli == NULL)
		return 10;
	
	copiaVettore(vettoreFigli,nodo->figli,nodo->nFigliMorti);
	
	nodo->nFigliMorti++;
	
	nodo->figliMorti = (Node**)calloc(nodo->nFigliMorti, sizeof(Node));
	if(nodo->figliMorti == NULL)
		return 10;
		
	copiaVettore(nodo->figliMorti,vettoreFigli,nodo->nFigliMorti - 1);
	
	nodo->figliMorti[nodo->nFigliMorti - 1] = nodo->figli[i];
	
	return 0;
}

//Funzione che permette di chiudere un figlio attraverso il padre
int fatherCloseMe(Node* father, char *childName, int flag, int multiQuit) {
	//Trovo il processo tra i miei figli per avere l'indice i esimo
	int i = 0;
	Node *tmp = (Node*)calloc(1, sizeof(Node));
	if(tmp == NULL)
		return 9;

	char* test = (char*)calloc(MAXLEN,sizeof(char));
	if(test == NULL)
		return 9;


	Node** vettoreFigli = (Node**)calloc((father->nFigli-1), sizeof(Node));
	if(vettoreFigli == NULL)
		return 9;

	for(i = 0; i < father->nFigli; i++)
	{
		tmp = father->figli[i];
		if(strcmp(tmp->name, childName) == 0)
			break;
	}

	if(flag == 0)
	{
		if(signal(SIGCHLD, SIG_IGN) == SIG_ERR)
			return 9;

		if(writePipe(tmp, "EXIT") != 0)
			return 10;

		//int tentativi = 0;

		int k = readPipe(tmp,test);
		if(k != 0)
		{
			printf("Problema riscontrato nelle pipe interne\n");
			return 10;
		}
		if(multiQuit != 1)
			printf("%s\n", test);
	}
	
	//tmp->morto = 1;
	
	//if(spostaSulVettoreDeiMorti(i,father) != 0)
		//return 10;
	
	//Sposto di una posizione tutti i figli che stanno alla destra del figlio che ho eliminato
	spostaASinistra(i, father->figli, father->nFigli);
	//Riduco di una dimensione la memoria allocata al vettore dei figli
	copiaVettore(vettoreFigli, father->figli, father->nFigli-1);
	free(father->figli);
	//Diminuisco di uno il numero di miei figli
	father->nFigli--;
	father->figli = (Node**)calloc(father->nFigli, sizeof(Node));
	if(father->figli == NULL)
		return 10;

	//Ricopio il vettore originale
	copiaVettore(father->figli, vettoreFigli, father->nFigli);

	return 0;
}




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

Node* init();
char* phelp();
int plist(Node*, char*);
int pnew(Node*, char*, int);
int pinfo(Node*,char *);
int prePClose(Node*, char*);
int quit(Node*);
int pspawn(Node*, char*, int);
int prmall(Node*, char*);
void ptree(Node*, int, char*);

#endif





//Funzione per chiudere un processo con <name>* e riconoscere il wildcard
int pCloseWildCard(Node* padre, char* nomeC) {
	int i = 0, n, ris = 0;
	while(i < padre->nFigli)
	{
		n = padre->nFigli;
		
		
		if(wildcard(padre->figli[i]->name,nomeC) == 1)
		{
			return prmall(padre,padre->figli[i]->name);
		}
		else
		{
			ris = pCloseWildCard(padre->figli[i], nomeC);
			if(ris != 0)
				return ris;
		}
		
		if (n == padre->nFigli)
			i++;
	}

	return 0;
}

//Funzione per chiudere tutti i processi con il wildcard
int prePClose(Node* padre, char* attributo) {
	if(strstr(attributo, "*") == NULL)
		return pClose(padre, attributo);
	else
	{
		char* nome = (char*)calloc(sizeof(attributo),sizeof(char));
		if(nome == NULL)
			return 9;

		int i = 0, j = 0;
		while(i < strlen(attributo))
		{
			if(attributo[i] != '*')
			{
				nome[j] = attributo[i];
				i++;
				j++;
			}
			else
				i++;
		}

		return pCloseWildCard(padre, nome);
	}
}
