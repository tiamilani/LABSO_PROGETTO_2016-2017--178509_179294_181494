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

//Return -1 se errno è stato modificato, altrimenti 0
int returnErrno() {
	if(errno != 0)
	{
		return -1;
	}

	return 0;
}

//Stampa un elenco dei comandi disponibili
char* phelp() {
	return "NOME\n\tpManager - Gestore processi attraverso una shell virtuale\n\nUTILIZZO\n\tpmanager\t\t->\tesegue la shell custom\n\tpmanager [input_file]\t->\tesegue la shell custom eseguendo i comandi definiti in inputFile\n\nCOMANDI SHELL\n\tphelp\n\t\tstampa questo elenco\n\n\tplist\n\t\telenca i processi generati dalla shell custom\n\n\tplist [nome]\n\t\telenca i processi generati dalla shell custom partendo partendo da [nome]\n\n\tpnew [nome]\n\t\tcrea un nuovo processo con nome [nome]\n\n\tpinfo [nome]\n\t\tfornisce informazioni sul processo [nome]\n\n\tpclose [nome]\n\t\tchiede al processo [nome] di chiudersi\n\n\tpclose [nome*]\n\t\tchiede al processo [nome] di chiudersi dopo di che utilizzando '*' come wildcard chiederà a tutti i processi che iniziano con [nome] e qualsiasi altro carattere dopo di chiudersi\n\n\tpclose [*nome]\n\t\tchiede al processo <nome> di chiudersi dopo di che utilizzando '*' come wildcard chiederà a tutti i processi che finiscono con [nome] e qualsiasi altro carattere prima di chiudersi\n\n\tquit\n\t\tesce dalla shell custom\n\n\tpspawn [nome]\n\t\tchiede al processo [nome] di clonarsi creando [nome_i] con i progressivo\n\n\tpspawn [nome] [n]\n\t\tchiede al processo [nome] di clonarsi [n] volte creando [nome_i] con i progressivo\n\n\tprmall [nome]\n\t\tchiede al processo [nome] di chiudersi chiudendo anche eventuali cloni\n\n\tptree\n\t\tmostra la gerarchia completa dei processi generati\n\n\tptree [nome]\n\t\tmostra la gerarchia completa dei processi generati partendo da [nome]\n\n\tpfile [nome]\n\t\tapre il file [nome] e ne utilizza i comandi all'interno\n\n\tpexport\n\t\tesporta su di un file locale la gerarchia attuale\n\n\t";
}

//Legge una stringa dalla pipe che gli viene passata
int readLine (int idPipeLettura,char *str) {
	int n;

	do {  //Read characters until '\0' or end-of-input
		n = read(idPipeLettura, str, 1); //Read one character
	} while (n > 0 && *str++ != '\0'); //Controllo che non ci siano errori

	//Return -1 se ci sono stati errori
	return returnErrno();
}

//Apro una pipe e mi metto in lettura, metto la stringa letta in str
int readPipe(Node* nodo,char *str) {

	nodo->idPipeLettura = open(nodo->nomePipeLettura, O_RDONLY); // Open it for reading

	//Se ci sono stati errori nell'apertura della pipe ritorno un errore 10
	if(errno != 0)
		return 10;

	//Leggo dalla pipe
	int ris = readLine(nodo->idPipeLettura,str);

	//chiude il file
	if(close(nodo->idPipeLettura) == EOF)
	{
		perror("Error");
		return 10;
	}

	return ris;
}

//Scrivo sulla pipe
int writePipe(Node* nodo, char* str) {

	//Tento di aprire la pipe interessata
	nodo->idPipeScrittura = open(nodo->nomePipeScrittura, O_WRONLY);

	if(returnErrno() != 0)
	{
		//Se ci sono stati errori lo stampo a video e ritorno
		printf("ERRORE NELL'APERTURA DELLA PIPE\n");
		return 10;
	}

	//Scrivo il messaggio
	int n = write (nodo->idPipeScrittura, str, strlen(str));
	if(n < 0)
		return 10;

	if(close(nodo->idPipeScrittura) == EOF)
	{
		perror("Error");
		return 10;
	}

	return 0;
}

//Creo le pipe che vengono utilizzate
int creaPipe(Node* nodo) {
	//Rimuovo le pipe eventualmente residue
	unlink(nodo->nomePipeLettura);
	unlink(nodo->nomePipeScrittura);

	//Reimposto errno, dato che se le pipe non vengano trovate modificano errno
	errno = 0;

	mknod(nodo->nomePipeLettura, S_IFIFO, 0); // Create named pipe
	chmod(nodo->nomePipeLettura, 0666); // Change its permissions

	mknod (nodo->nomePipeScrittura, S_IFIFO, 0); // Create named pipe
	chmod (nodo->nomePipeScrittura, 0666); // Change its permissions

	return returnErrno();
}

//Chiudo le pipe
int chiudiPipe(Node* nodo) {
	//Chiudo i file descriptor
	close(nodo->idPipeLettura);
	close(nodo->idPipeScrittura);

	//Rimuovo le pipe
	unlink(nodo->nomePipeLettura);
	unlink(nodo->nomePipeScrittura);

	return 0;
}

//Ritorno il pid come stringa
char* getPid(Node* n) {
	char* str = (char*)calloc(MAXLEN, sizeof(char*));
	if(str == NULL) //Controllo eventuali errori di allocazione
		return str;

	snprintf(str, MINLEN, "%d", n->pid);

	return str;
}

//Ritorno il numero di figli come stringa
char* getnFigli(Node* n) {
	char* str = (char*)calloc(MAXLEN, sizeof(char*));
	if(str == NULL) //Controllo eventuali errori di allocazione
		return str;

	snprintf(str, MINLEN, "%d", n->nFigli);

	return str;
}

//Funzione per il get del name
char* getName(Node* n) {
	return n->name;
}

//Richiedo il pid del nodo padre
char* getFatherPid(Node* n) { //se ritorna null c'è stato un errore in fase di allocazione della memoria
	return getPid(n->father);
}

//Funzione get del nodo padre
Node* getFather(Node* n) {
	return n->father;
}

//Funzione get del nome del nodo padre
char* getFatherName(Node* n) { //se ritorna null c'è stato un errore in fase di allocazione della memoria
	return getName(n->father);
}

//Scrivo su una stringa le informazioni del processo passato
int pinfo(Node* n, char* str) {
	char* tmp = (char*)calloc(MAXLEN,sizeof(char));
	if(tmp == NULL)
		return 9;

	strcat(str,"Nome\t\t");
	tmp = getName(n);
	strcat(str, tmp);
	strcat(str, "\n");


	strcat(str,"Pid\t\t");
	tmp = getPid(n);
	if(tmp == NULL)
		return 9;
	strcat(str, tmp);
	strcat(str, "\n");

	strcat(str,"Padre\t\t");
	tmp = getFatherName(n);
	if(tmp == NULL)
		return 9;
	strcat(str, tmp);
	strcat(str, "\n");

	strcat(str,"PPid\t\t");
	tmp = getFatherPid(n);
	if(tmp == NULL)
		return 9;
	strcat(str, tmp);
	strcat(str, "\n");

	strcat(str,"numero figli\t");
	tmp = getnFigli(n);
	if(tmp == NULL)
		return 9;
	strcat(str, getnFigli(n));
	strcat(str, "\n");

	strcat(str,"System pid\t");
	snprintf(tmp, MINLEN, "%d", n->systemPid);
	strcat(str,tmp);

	strcat(str, "\n");

	return 0;
}

//Scrivo su di una stringa la lista dei processi
int plist(Node* nodo, char* ch) {

	//Controllo per garantire che la stringa sia lunga a sufficenza
	if(ch != NULL && strlen(ch) > 0)
	{
		//Ottengo un valore di lunghezza della stringa rispetto alla lunghezza massima attuale
		double alpha = ((double)strlen(ch))/((double)longlen);
		//Se il rapporto supera 0,8 raddoppio la dimensione della stringa e della lunghezza massima pre impostata
		if(alpha > 0.8)
		{
			ch = (char*)realloc(ch,strlen(ch)*2*(sizeof(char)));
			longlen = longlen*2;
		}
	}

	//Inserisco il nome del nodo attuale
	if(nodo->morto == 0)
		strcat(ch, getName(nodo));
	else
	{
		strcat(ch,ANSI_COLOR_RED);
		strcat(ch, getName(nodo));
		strcat(ch, " (chiuso)");
		strcat(ch,ANSI_COLOR_RESET);
	}
	strcat(ch, "\n");

	//Aggiungo il nome di tutti i miei figli
	int i;
	for(i = 0; i < nodo->nFigli; i++)
		plist(nodo->figli[i], ch);
	
	for(i = 0; i < nodo->nFigliMorti; i++)
	{
		plist(nodo->figliMorti[i], ch);
	}

	return 0;
}

//Scrivo l'albero dei processi su di una stringa
void ptree(Node* nodo, int tab,char* ch) {

	//Come per il plist devo controllare la dimensione della stringa
	if(ch != NULL && strlen(ch) > 0)
	{
		double alpha = ((double)strlen(ch))/((double)longlen);
		if(alpha > 0.8)
		{
			ch = (char*)realloc(ch,strlen(ch)*2*(sizeof(char)));
			longlen = longlen*2;
		}
	}

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
	
	for(i = 0; i < nodo->nFigliMorti; i++)
	{
		int j;
		for(j = 1; j < tab; j++)
			strcat(ch, "|   ");

		strcat(ch, ANSI_COLOR_RED);
		strcat(ch, "|");
		strcat(ch, "-");
		strcat(ch, "> ");

		strcat(ch, nodo->figliMorti[i]->name);
		strcat(ch, " (chiuso)");
		strcat(ch, "\n");
		
		strcat(ch, ANSI_COLOR_RESET);
		
		if(nodo->figliMorti[i]->nFigliMorti > 0)
		{
			ptree(nodo->figliMorti[i], tab+1, ch);
		}
		
	}
}

//Funzione per la ricerca utilizzando il nome partendo da un determinato nodo
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

//Funzione per ottenere il pid di un processo da un messaggio che mi è appena arrivato
int ottieniPid(char* test) {
	char* testo = (char*)calloc(MAXLEN, sizeof(char));
	char* pid = (char*)calloc(MINLEN, sizeof(char));

	if(testo == NULL || pid == NULL)
		return -1;

	testo = strtok(test, ",");
	pid = strtok(NULL, ",");

	return (int)strtol(pid, (char **)NULL, 10);
}

//Funzione per la creazione di un nuovo nodo
int pnew(Node *start, char* nome, int signalChildren) { //SignalChildren 0 faccio qui il fork, altrimenti lo fa il processo
	contPid++;

	//Controllo se il nodo esisteva già
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

	//Vettore d'appoggio
	Node** vettoreFigli = (Node**)calloc(start->nFigli, sizeof(Node));
	if(vettoreFigli == NULL)
		return 8;

	//Inserisco i nomi delle stringhe nelle rispettive variabili
	snprintf(n->nomePipeScrittura, PIPELEN, "%s%d", "pipe/Scrittura_", contPid);
	snprintf(n->nomePipeLettura, PIPELEN, "%s%d", "pipe/Lettura_", contPid);

	char* strpid = (char*)calloc(MINLEN, sizeof(char));
	if(strpid == NULL)
		return 8;

	//Variabili di supporto
	char* test = (char*)calloc(MAXLEN, sizeof(char));
	if(test == NULL)
		return 8;
	char* test2 = (char*)calloc(MAXLEN, sizeof(char));
	if(test == NULL)
		return 8;

	snprintf(strpid, MINLEN, "%d", contPid);

	//Creo le pipe
	if(creaPipe(n) != 0)
	{
		//Nel caso in cui non sia riuscito a crearle chiudo eventuali resti delle pipe e ritorno l'errore 8
		chiudiPipe(n);
		return 8;
	}

	//In base al flag signal children so se devo creare un nuovo processo o richiedere una clonazione
	if(signalChildren == 0)
	{
		int p = fork();
		if(p == 0)
			execl("build/child", n->name, strpid, (char*) NULL);
		else if (p < 0)
			return 8;
	}
	else if(signalChildren == 1) //Clonazioen verbose
	{
		char* message = (char*)calloc(MAXLEN,sizeof(char));
		if(message == NULL)
			return 8;

		//Preparo il messaggio
		strcat(message,"CLON ");
		strcat(message,n->name);
		strcat(message," ");
		strcat(message,strpid);
		strcat(message,".");

		//Invio il messaggio al figlio
		if(writePipe(start, message) != 0)
		{
			printf("Problema riscontrato nelle pipe interne\n");
			return 10;
		}

		//Attendo il messaggio di risposta
		int k = readPipe(start,test);
		if(k != 0)
		{
			printf("Problema riscontrato nelle pipe interne\n");
			return 10;
		}
		if(strstr(test,"IMPOSSIBILE") != NULL) //Controllo il caso in cui ci siano stati problemi nella creazione del clone
		{
			printf("%s\n", test);
			return 8;
		}
		//Stampo il messaggio che mi è stato mandato dal children
		printf("%s\n", test);
	}
	else //Clonazione non verbose, non stampo i messaggi ritornati
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
			return 10;
		if(strstr(test,"IMPOSSIBILE") != NULL)
			return 8;
	}

	//Attendo un messaggio di conferma creazioen da parte del figlio
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

	//mi ricavo il pid di sistema che mi è stato passato, in modo che non sia visibile all'utente
	n->systemPid = ottieniPid(test2);
	if(n->systemPid == -1)
	{
		printf("ERRORE nella lettura del messaggio ricevuto\nNon riesco a ricavare il pid di sistema\n");
		return 10;
	}
	if(signalChildren != 2) //In caso di verbose stampo
		printf("%s\n", test2);

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

//Funzione per spostare il puntatore di un figlio alla sezione dei processi morti
int spostaSulVettoreDeiMorti(int i,Node* nodo) {
	//Vettore d'appoggio
	Node** vettoreFigli = (Node**)calloc(nodo->nFigliMorti, sizeof(Node));
	if(vettoreFigli == NULL)
		return 10;
	
	copiaVettore(vettoreFigli,nodo->figliMorti,nodo->nFigliMorti);
	
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
	
	tmp->morto = 1;
	
	if(spostaSulVettoreDeiMorti(i,father) != 0)
		return 10;
	
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
int closeMe(Node* nodo, int multiQuit) {
	//devo chiudermi
	//Controllo se ho figli
	if(nodo->nFigli != 0)
		return 6;

	//chiedo a mio padre di inviarmi il segnale di chiusura al processo
	return fatherCloseMe(nodo->father, nodo->name, 0, multiQuit);
}

//Funzione per chiudere un processo con <name>
int pClose(Node* start, char* name) {

	Node *tmp = (Node*)calloc(1, sizeof(Node));
	if(tmp == NULL)
		return 9;

	tmp = cerca(start, name);

	if(tmp == NULL)
		return 1; //Errore, nodo non trovato
	else
	{
		int ris = closeMe(tmp, 0);

		return ris;
	}
}

//Funzione per chiudere tutti i processi partendo da un nodo (compreso)
int closeAll(Node* start, int multiQuit) {
	int ris = 0;
	while(start->nFigli > 0)
		ris = closeAll(start->figli[0], multiQuit);

	if(ris != 0)
		return ris;

	ris = closeMe(start, multiQuit);

	return ris;
}

//Killa un processo
void killProc(int pid){
	 kill(pid, SIGTERM);
}

//Funzione per chiudere forzatamente tutti i processi partendo da un nodo
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

//Quit dalla shell forzatamente
void errorquit(Node* start){
 	int i = start->nFigli - 1;
 	while(i >= 0)
 	{
 		errorcloseAll(start->figli[i]);
 		i--;
 	}

  	free(start);
  }

//Funzione che clona un certo processo
int pspawn(Node* start, char* name, int multiSpawn) {
	//Cerco il processo da clonare
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

		if(multiSpawn == 0)
			return pnew(tmp, childrenName, 1);
		else
			return pnew(tmp, childrenName, 2);
	}
}

//Funzione per clonare più volte
int prePSpawn(Node* start, char* name, char* option){
	int num, i;
	num = (int)strtol(option, (char **)NULL, 10);

	if(num < 0)
		return 11;

	int ris = 0;
	for(i = 0; i < num; i++)
	{
		fflush(stdout);
		printf("\rCloning... %d/%d", i, num);
		ris = pspawn(start, name, 1);
		if(ris != 0)
			return ris;
	}
	fflush(stdout);
	printf("\rCloning... %d/%d -> DONE\n", i, num);

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
		return closeAll(tmp, 0);
}

//Funzione per chiudere correttamente processi e shell
int quit(Node* start) {
	int ris = 0;
	float j = 0;
	int n = start->nFigli;

	float ratio;
	ratio = (float)(100.0/(float)n);

	while(n > 0)
	{
		fflush(stdout);
		printf("\rQuitting... %.1f %%", j);
		ris = closeAll(start->figli[0], 1);
		n = start->nFigli;
		j += ratio;
	}
	fflush(stdout);
	printf("\rQuitting... %d %% -> DONE\n", 100);

	free(start);
	printf("Chiusura di tutti i processi avvenuta.\n");

	return ris;
}

//Funzione che permette di utilizzare il wildcard all'interno dei comandi
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

//Funzione per chiudere un processo con <name>* e riconoscere il wildcard
int pCloseWildCard(Node* padre, char* nome) {
	int i = 0, n, ris = 0;
	while(i < padre->nFigli)
	{
		n = padre->nFigli;

		if(wildcard(padre->figli[i]->name,nome) == 1)
			ris = prmall(padre,padre->figli[i]->name);
		else
			ris = pCloseWildCard(padre->figli[i], nome);

		if(ris != 0)
			return ris;

		
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

//Ottiene la gerarchia tramite pspawn
void ottieniGerarchia(Node* nodo,char* test){
	if(nodo->nFigli > 0)
	{
		strcat(test,"pspawn ");
		strcat(test, getName(nodo));
		strcat(test, " ");
		strcat(test, getnFigli(nodo));
		strcat(test,"\n");

		int i;
		for(i = 0; i < nodo->nFigli; i++)
			ottieniGerarchia(nodo->figli[i],test);
	}
}

//Eport della gerarchia come file
int pexport(Node* nodo){
	FILE* file  = fopen("src/log.txt", "w+");

	if(file)
	{
		int i;
		for(i = 0; i < nodo->nFigli; i++)
		{
			char* test = (char*)calloc(LONGLEN,sizeof(char));
			if(test == NULL)
			{
				fclose(file);
				return 9;
			}

			fprintf(file, "%s", "pnew ");
			fprintf(file, "%s", nodo->figli[i]->name);
			fprintf(file, "%s", "\n");

			ottieniGerarchia(nodo->figli[i],test);
			fprintf(file, "%s", test);
		}
	}
	else
	{
		fclose(file);
		return 9;  //File non trovato
	}

	fclose(file);
	return 0;
}

//Funzione per inizializzare il nodo padre
Node* init() {
	Node *padre = (Node*)calloc(1,sizeof(Node));
	if(padre!=NULL){
		padre->pid = contPid;
		padre->systemPid = getpid();
		padre->father = NULL;
		padre->name = "pmanager";
		padre->nFigli = 0;
		padre->numCloni = 0;
		return padre;
	}

	perror("Error: ");
	return NULL;
}
