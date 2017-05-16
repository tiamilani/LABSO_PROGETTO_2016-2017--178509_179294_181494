#include "Node.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
/*
System rm della pipe in caso di error quit
Ricreare la pipe tentare
Non fatal error ma error 12 processi killati ma solo quelli con pipe rotta, esecuzione non interrotta
Problema di concorrenza pmanager

Phelp più completo
Ptree dal processo
File open dal pmanager
Pinfo verbose
Pnew più processi
Pspawn percentuale per i multipli e magari meno printf dei cloni multipli
Plist nodo diverso dal padre
Plist simil ls (per le colonne)
Prmall con wildcard
Esporta gerarchia attuale
*/

char* color(int num)
{
	int color = num%6;

	switch(color)
	{
		case 1:
			return ANSI_COLOR_RED_BG;
		case 2:
			return ANSI_COLOR_GREEN_BG;
		case 3:
			return ANSI_COLOR_YELLOW_BG;
		case 4:
			return ANSI_COLOR_BLUE_BG;
		case 5:
			return ANSI_COLOR_MAGENTA_BG;
		case 6:
			return ANSI_COLOR_CYAN_BG;
		default:
			return ANSI_COLOR_RESET_BG;
	}
}

int returnErrno() {
	if(errno != 0)
	{
		//perror("ERROR: ");
		//errno = 0;
		return -1;
	}

	return 0;
}

char* phelp() {
	//Stampa un elenco dei comandi disponibili
	return "Comandi disponibili utilizzando la shell virtuale pManager:\n\tphelp : stampa questo elenco\n\tplist : elenca i processi generati dalla shell custom\n\tplist <nome processo>: elenca i processi generati dalla shell custom partendo dal processo specificato\n\tpnew <nome> : crea un nuovo processo con nome <nome>\n\tpnew <nome1> ... <nome n> : crea n nuovi processi con nomi rispettivamente da <nome1> a <nome n>\n\tpinfo <nome> : fornisce informazioni sul processo <nome>\n\tpinfo <nome> -v: fornisce informazioni sul processo <nome> con l'argomento '-v' si ottiene un output verboso\n\tpclose <nome> : chiede al processo <nome> di chiudersi\n\tpclose <nome>* : chiede al processo <nome> di chiudersi dopo di che utlizzando '*' come wildcard chiderà a tutti i processi che iniziano con <nome> e qualsiasi altro carattere dopo di chiudersi\n\tquit : esce dalla shell custom\n\tpspawn <nome> : chiede al processo <nome> di clonarsi creando <nome_i> con i progressivo\n\tpspawn <nome> <n>: chiede al processo <nome> di clonarsi <n> volte creando <nome_i> con i progressivo processi cloni di <nome>\n\tprmall <nome> : chiede al processo <nome> di chiudersi chiudendo anche eventuali cloni\n\tprmall <nome>*: chiede al processo <nome> di chiudersi chiudendo anche eventuali cloni, dopo di che come per il pclose fa la stessa cosa con tutti i processi che hanno un qualsiasi carattere dopo <nome>\n\tptree : mostra la gerarchia completa dei processi generati attivi\n\tptree <nome>: mostra la gerarchia completa dei processi generati attivi partendo da <nome>\n\tpfile <nome>: apre il file <nome> e ne utilizza i comandi all'interno\n\tpexport: esporta su di un file locale la gerarchia attuale\n";
}

int readLine (int idPipeLettura,char *str) {
	int n;

	do {  //Read characters until '\0' or end-of-input
		n = read(idPipeLettura, str, 1); //Read one character
	} while (n > 0 && *str++ != '\0'); //Controllo che non ci siano errori

	return returnErrno();
}

int readPipe(Node* nodo,char *str) {
	//char* str = (char*)calloc(MAXLEN,sizeof(char));

	nodo->idPipeLettura = open(nodo->nomePipeLettura, O_RDONLY); /* Open it for reading */

	if(errno != 0)
		return 10;

	int ris = readLine(nodo->idPipeLettura,str); /* Display received messages */

	/* chiude il file */
	if(close(nodo->idPipeLettura) == EOF)
	{
		perror("Error");
		return 10;
	}

	return ris;
}

int writePipe(Node* nodo, char* str) {
	nodo->idPipeScrittura = open(nodo->nomePipeScrittura, O_WRONLY); /* Open it for reading */
	if(returnErrno() != 0)
	{
		printf("ERRORE NELL'APERTURA DELLA PIPE\n");
		return 10;
	}

	int n = write (nodo->idPipeScrittura, str, strlen(str)); /* Write message down pipe */
	if(n < 0)
		return 10;

	if(close(nodo->idPipeScrittura) == EOF)
	{
		perror("Error");
		return 10;
	}

	return 0;
}

int creaPipe(Node* nodo) {
	unlink(nodo->nomePipeLettura);
	unlink(nodo->nomePipeScrittura);

	errno = 0;

	mknod(nodo->nomePipeLettura, S_IFIFO, 0); /* Create named pipe */
	chmod(nodo->nomePipeLettura, 0666); /* Change its permissions */

	mknod (nodo->nomePipeScrittura, S_IFIFO, 0); /* Create named pipe */
	chmod (nodo->nomePipeScrittura, 0666); /* Change its permissions */

	return returnErrno();
}

void chiudiPipe(Node* nodo) {
	close(nodo->idPipeLettura);
	close(nodo->idPipeScrittura);
	unlink(nodo->nomePipeLettura);
	unlink(nodo->nomePipeScrittura);
}

char* getPid(Node* n) {
	char* str = (char*)calloc(1, sizeof(char*));
	snprintf(str, MINLEN, "%d", n->pid);

	return str;
}

char* getnFigli(Node* n) {
	char* str = (char*)calloc(1, sizeof(char*));
	snprintf(str, MINLEN, "%d", n->nFigli);

	return str;
}

char* getName(Node* n) {
	return n->name;
}

char* getFatherPid(Node* n) {
	return getPid(n->father);
}

Node* getFather(Node* n) {
	return n->father;
}

char* getFatherName(Node* n) {
	return getName(n->father);
}

int pinfo(Node* n, char* str) {

	int spacing = 0;
	int i;

	spacing = strlen(n->name);
	strcat(str,"Nome");
	for(i=0;i<spacing;i++)
		strcat(str," ");

	strcat(str,"Pid");
	spacing = strlen(getPid(n));
	for(i=0;i<spacing;i++)
		strcat(str," ");

	strcat(str,"Padre");

	spacing = strlen(getFatherName(n));
	for(i=0;i<spacing;i++)
		strcat(str," ");

	strcat(str,"PPid\n");

	strcat(str, getName(n));
	for(i=0;i<strlen("Nome");i++)
		strcat(str," ");

	strcat(str,getPid(n));

	for(i=0;i<strlen("Pid");i++)
		strcat(str," ");

	strcat(str, getFatherName(n));

	for(i = 0; i < strlen("Padre"); i++)
		strcat(str," ");

	strcat(str, getFatherPid(n));
	return 0;
}

void plist(Node* nodo, char* ch) {

	if(ch != NULL && strlen(ch) > 0)
	{
		double alpha = ((double)strlen(ch))/((double)longlen);
		if(alpha > 0.8)
		{
			ch = (char*)realloc(ch,strlen(ch)*2*(sizeof(char)));
			longlen = longlen*2;
		}
	}

	strcat(ch, getName(nodo));
	strcat(ch, "\n");
	int i;

	for(i = 0; i < nodo->nFigli; i++)
		plist(nodo->figli[i],ch);
}

void ptree(Node* nodo, int tab,char* ch) {

	if(ch != NULL && strlen(ch) > 0)
	{
		double alpha = ((double)strlen(ch))/((double)longlen);
		if(alpha > 0.8)
		{
			ch = (char*)realloc(ch,strlen(ch)*2*(sizeof(char)));
			longlen = longlen*2;
		}
	}


	//strcat(ch,color(tab));

	if(tab == 1)
	{
		strcat(ch, nodo->name);
		strcat(ch, "\n");
	}

	int i;
	for(i = 0; i < nodo->nFigli; i++)
	{
		int j;
		for(j = 1; j < tab; j++)
			strcat(ch, "|   ");
		strcat(ch, "|");
		strcat(ch, "-");
		strcat(ch, "> ");

		strcat(ch, nodo->figli[i]->name);
		strcat(ch, "\n");

		if(nodo->figli[i]->nFigli > 0)
			ptree(nodo->figli[i], tab+1,ch);
	}

	//strcat(ch,color(tab-1));
}

Node* cerca(Node *start,char* name) {
	if(strcmp(start->name, name) == 0)
		return start;
	else
	{
		int i = 0;
		for(i = 0; i < start->nFigli; i++)
		{
			Node* tmp = cerca(start->figli[i], name);
			if(tmp != NULL)
				return tmp;
		}

		return NULL;
	}
}

//Funzione per copiare da v2 a v1 fino ad n
void copiaVettore(Node** v1, Node** v2, int n) {
	int i;
	for(i = 0; i < n; i++)
		v1[i] = v2[i];
}

//Funzione per spostare a sinistra da i+1 fino ad n
void spostaASinistra(int i, Node** v, int n) {
	int k;
	for(k = i+1; k < n; k++)
	{
		v[i] = v[k];
		i++;
	}
}

int ottieniPid(char* test)
{
	char* testo = (char*)calloc(MAXLEN, sizeof(char));
	char* pid = (char*)calloc(MINLEN, sizeof(char));

	testo = strtok(test, ",");
	pid = strtok(NULL, ",");
	//strcpy(test, testo);

	return (int)strtol(pid, (char **)NULL, 10);
}

//Return -1 se il nodo esisteva già tra i figli successivi di start
int pnew(Node *start, char* nome,int signalChildren) { //SignalChildren 0 faccio qui il fork, altrimenti lo fa il processo
	contPid++;
	if(cerca(start, nome) != NULL)
		return 7;

	//Inserisco i dati nella struttura dati
	Node *n = (Node*)calloc(1, sizeof(Node));
	if(n == NULL)
		return 8;

	n->pid = contPid;
	n->father = start;
	n->name = (char*)calloc(100,sizeof(char));
	if(n->name == NULL)
		return 8;

	strcat(n->name,nome);
	n->nFigli = 0;
	n->numCloni = 0;

	n->nomePipeScrittura = (char*)calloc(PIPELEN, sizeof(char));
	n->nomePipeLettura = (char*)calloc(PIPELEN, sizeof(char));

	if((n->nomePipeScrittura == NULL) || (n->nomePipeLettura == NULL))
		return 8;

	Node** vettoreFigli = (Node**)calloc(start->nFigli, sizeof(Node));
	if(vettoreFigli == NULL)
		return 8;

	snprintf(n->nomePipeScrittura, PIPELEN, "%s%d", "assets/Scrittura_", contPid);
	snprintf(n->nomePipeLettura, PIPELEN, "%s%d", "assets/Lettura_", contPid);
	char* strpid = (char*)calloc(MINLEN, sizeof(char));
	if(strpid == NULL)
		return 8;

	char* test = (char*)calloc(MAXLEN, sizeof(char));
	if(test == NULL)
		return 8;
	char* test2 = (char*)calloc(MAXLEN, sizeof(char));
	if(test == NULL)
		return 8;

	snprintf(strpid, MINLEN, "%d", contPid);

	if(creaPipe(n) != 0)
	{
		chiudiPipe(n);
		return 8;
	}

	if(signalChildren == 0)
	{
		int p = fork();
		if(p == 0)
			execl("build/child", n->name, strpid, (char*) NULL);
		else if (p < 0)
			return 8;
	}
	else
	{
		char* message = (char*)calloc(MAXLEN,sizeof(char));
		if(message == NULL)
			return 8;

		strcat(message,"CLON ");
		strcat(message,n->name);
		strcat(message," ");
		strcat(message,strpid);
		strcat(message,".");

		if(writePipe(start, message) != 0)
		{
			printf("Problema riscontrato nelle pipe interne\n");
			return 10;
		}

		int k = readPipe(start,test);
		if(k != 0)
		{
			//printf("Problema riscontrato nelle pipe interne\n");
			return 10;
		}
		if(strstr(test,"IMPOSSIBILE") != NULL)
		{
			printf("%s\n", test);
			return 8;
		}
		printf("%s\n", test);
		/*
		tentativi = 0;

		do{
			int k = readPipe(n,test)
			if(k != 0 && k != -1)
				return 10;

			if(strstr(test, "terminato") != NULL)
			{
				printf("%s\n", test);
				break;
			}
			else
				printf("Non ho ricevuto risposta dal processo...riprovo");

			tentativi++;
		}while(tentativi < 3)

		if(tentativi == 3)
			return 10;*/
	}

	int k = readPipe(n, test2);

	if(k != 0)
	{
		printf("Problema riscontrato nelle pipe interne\n");
		return 10;
	}
	if(strstr(test,"ERRORI") != NULL)
	{
		printf("%s\n", test2);
		return 8;
	}

	n->systemPid = ottieniPid(test2);

	printf("%s\n", test2);

	/*tentativi = 0;

	do{
		int k = readPipe(n,test)
		if(k != 0 && k != -1)
			return 10;

		if(strstr(test, "terminato") != NULL)
		{
			printf("%s\n", test);
			break;
		}
		else
			printf("Non ho ricevuto risposta dal processo...riprovo");

		tentativi++;
	}while(tentativi < 3)

	if(tentativi == 3)
		return 10;*/

	//Riduco di una dimensione la memoria allocata al vettore dei figli
	copiaVettore(vettoreFigli, start->figli, start->nFigli);
	//free(start->figli);
	//Diminuisco di uno il numero di miei figli
	start->nFigli++;
	start->figli = (Node**)calloc(start->nFigli, sizeof(Node));
	if(start->figli == NULL)
		return 10;

	//Ricopio il vettore originale
	copiaVettore(start->figli, vettoreFigli, start->nFigli - 1);

	//Assegno il nuovo figlio
	start->figli[start->nFigli - 1] = n;

	//Ritorno 0 se è tutto a posto
	return 0;
}

//Funzione che permette di chiudere un figlio attraverso il padre
int fatherCloseMe(Node* father, char *childName) {
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
	printf("%s\n", test);
	/*
	do{
		int k = readPipe(tmp,test)
		if(k != 0 && k != -1)
			return 10;

		if(strstr(test, "terminato") != NULL)
		{
			printf("%s\n", test);
			break;
		}
		else
			printf("Non ho ricevuto risposta dal processo...riprovo");

		tentativi++;
	}while(tentativi < 3)

	if(tentativi == 3)
		return 10;

	chiudiPipe(tmp);*/

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

//closeMe() controllo se ho figli, se non ne ho mi chiudo, altrimenti errore
int closeMe(Node* nodo) {
	//devo chiudermi
	//Controllo se ho figli
	if(nodo->nFigli != 0)
		return 6;

	//chiedo a mio padre di inviarmi il segnale di chiusura al processo
	return fatherCloseMe(nodo->father, nodo->name);
}

//Funzione per chiudere un processo con <name>
int pClose(Node* start, char* name) {

	Node *tmp = (Node*)calloc(1, sizeof(Node));
	tmp = cerca(start, name);

	if(tmp == NULL)
		return 1; //Errore, nodo non trovato
	else
	{
		int ris = closeMe(tmp);
		//Dealloco la mia memoria
		if(ris == 0)
			free(tmp);

		return ris;
	}
}

int closeAll(Node* start) {
	int ris = 0;
	while(start->nFigli > 0)
		ris = closeAll(start->figli[0]);

	if(ris != 0)
		return ris;

	ris = closeMe(start);
	if(ris == 0)
		free(start);

	return ris;
}

void killProc(int pid)
{
	 kill(pid, SIGTERM);
}

void errorcloseAll(Node* start) {
	int i = start->nFigli - 1;
	while(i >= 0)
	{
		errorcloseAll(start->figli[i]);
		i--;
	}

	killProc(start->systemPid);
	printf("Processo <%d> terminato\n",start->pid);
	free(start);
}

//Funzione che clona un certo processo
//Ipoteticamente la clonazione genera un figlio
int pspawn(Node* start,char* name) {
	//Cerco il processo da clonare
	//printf("ENTRO\n");
	Node *tmp = (Node*)calloc(1, sizeof(Node));
	if(tmp == NULL)
		return 9;

	tmp = cerca(start, name);

	if(tmp == NULL)
		return 1; //Errore, nodo non trovato
	else
	{
		tmp->numCloni++;

		char* childrenName = (char*)calloc(strlen(name) + 5, sizeof(char));
		if(childrenName == NULL)
			return 9;

		childrenName = strcat(childrenName, name);

		if(tmp->nFigli > 0)
			snprintf(childrenName, strlen(childrenName) + 6, "%s%s%d", tmp->name, "_", tmp->numCloni);
		else
			childrenName = strcat(childrenName,"_1");

		return pnew(tmp, childrenName, 1);
	}
}

int prePSpawn(Node* start, char* name, char* option){
	int num, i;
	num = atoi(option); //fa schifo

	if(num < 0)
		return 11;

	int ris = 0;
	for(i = 0; i < num; i++)
	{
		ris = pspawn(start, name);
		if(ris != 0)
			return ris;
	}

	return ris;
}

//Funzione per chiudere tutto partendo da un nome
int prmall(Node* start, char* name) {
	Node *tmp = (Node*)calloc(1, sizeof(Node));

	if(tmp == NULL)
		return 9;

	tmp = cerca(start, name);
	if(tmp == NULL)
		return 1; //Errore, nodo non trovato
	else
		return closeAll(tmp);
}

int quit(Node* start) {
	int ris = 0;
	while(start->nFigli > 0)
		ris = closeAll(start->figli[0]);

	free(start);

	return ris;
}

void errorquit(Node* start){
	int i = start->nFigli - 1;
	while(i >= 0)
	{
		errorcloseAll(start->figli[i]);
		i--;
	}

	free(start);
}

int wildcard(char *string, char *pattern) {
	while(*string)
	{
		switch(*pattern)
		{
			case '*':
				do {
					++pattern;
				} while(*pattern == '*');

				if(!*pattern)
					return 1;

				while(*string != '\0')
				{
					if(*pattern != *string)
						string++;
					else
						break;
				}
					if(wildcard(string++,pattern++)== 1 )
						return 1;

				return 0;

			default :
				while(*pattern != '\0')
				{
					if(*pattern == '*')
						if(wildcard(string, pattern) == 1)
							return 1;

					if(*string !=*pattern)
						return 0;

					pattern++;
					string++;
				}

				return 1;
				break;
		}

		++pattern;
		++string;
	}

	while (*pattern == '*')
		++pattern;

	return !*pattern;
}

//Funzione per chiudere un processo con <name>
int pCloseWildCard(Node* padre, Node* start, char* nomeC, char* name) {
	int i = 0, n, ris = 0;
	while(i < start->nFigli)
	{
		n = start->nFigli;
		pCloseWildCard(padre, start->figli[i], nomeC, name);
		if (n == start->nFigli)
			i++;
	}

	if(wildcard(start->name, nomeC) == 1)
	 	ris = prmall(padre, start->name);

	return ris;
}

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

		return pCloseWildCard(padre, padre, attributo, nome);
	}
}

Node* init() {
	Node *padre = (Node*)calloc(1,sizeof(Node));
	if(padre!=NULL){
		padre->pid = contPid;
		padre->systemPid = getpid();
		padre->father = NULL;
		padre->name = "pManager";
		padre->nFigli = 0;
		padre->numCloni = 0;
		return padre;
	}

	perror("Error: ");
	return NULL;
}
