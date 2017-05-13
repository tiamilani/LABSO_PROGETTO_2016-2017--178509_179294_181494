#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "Node.c"

//Per aumentare la sicurezza del programma, nella libreria andrà implementata la funzione restituisci_radice
//Che mi ritornerà la radice già creata, da lì io potrò fare solo le operazioni sulla radice

Node *padre;
#define MAX_LENGTH 1024

//calloc NULL se non è andata a buon fine
//signal SIG_ERR 
//read -1 in caso di errore
//write -1
//fclose EOF
//printf numero negativo

void gestisciErrori(int error)
{
	/*
		0 -> Tutto OK
		1 -> Nodo non trovato
		2 -> Nome troppo lungo
		3 -> Nome non consentito
		4 -> Carattere speciale non consentito
		5 -> Comando non trovato
		6 -> Nodo con figli
		7 -> figlio già esistente
		8 -> impossibile creare il nuovo nodo
		9 -> error in errno stampato da perror, comando non eseguito
		10 -> FATAL ERROR
		11 -> argomento non valido
	*/
	switch(error)
	{
		case 0:
			break;
		case 1:
			printf("Nodo non trovato!\n");
			break;
		case 2:
			printf("Inserisci un nome piu' breve!\n");
			break;
		case 3:
			printf("Nome non consentito!\n");
			break;
		case 4:
			printf("Nome non valido, rimuovi il carattere _\n");
			break;
		case 5:
			printf("Comando non trovato, usa phelp per la lista dei comandi!\n");
			break;
		case 6:
			printf("Il processo non puo' terminare poiche' ha figli!\n");
			break;
		case 7:
			printf("Esiste già un figlio con questo nome\n");
			break;
		case 8:
			printf("Impossibile creare il processo\n");
			break;
		case 9:
			perror("Error");
			errno = 0;
			printf("Comando non eseguito\n");
			break;
		case 10:
			//errorquit(padre);
			printf("FATAL ERROR, prova di nuovo, sarai più fortuanto\n");
			exit(10);
			break;
		case 11:
			printf("Argomento non valido\n");
			break;
	}
}

int psystem(char *line) {
	
	char *pos;
	if((pos = strchr(line, '\n')) != NULL)
		*pos = '\0';

	char* comando = (char*)calloc(6, sizeof(char));
	char* attributo = (char*)calloc(MAXLEN, sizeof(char));
	comando = strtok(line, " ");
	attributo = strtok(NULL, " ");

	if(comando != NULL)
	{
		if(attributo == NULL)
		{
			if(strcmp(comando, "phelp") == 0)
				printf("%s\n", phelp());
			else if(strcmp(comando, "plist") == 0)
			{
				char* ch = calloc(LONGLEN,1);
				if(ch != NULL)
				{
					plist(padre,ch);
					printf("%s\n", ch);
				}
				else
					gestisciErrori(9);
			}
			else if(strcmp(comando, "ptree") == 0)
			{
				char* ch = calloc(LONGLEN,1);
				if(ch != NULL)
				{
					ptree(padre,1,ch);
					printf("%s\n", ch);
				}
				else
					gestisciErrori(9);
			}
			else if(strcmp(comando, "quit") == 0)
			{
				quit(padre);
				return 1;
			}
			else if(strcmp(comando, "errorquit") == 0)
			{
				errorquit(padre);
				return 1;
			}
			else
				gestisciErrori(5);
		}
		else
		{
			if(strlen(attributo) > MAXLEN)
			{
				gestisciErrori(2);
				return 0;
			}
			else if(strcmp(attributo, "pManager") == 0)
			{
				gestisciErrori(3);
				return 0;
			}
			else if(strcmp(comando, "pnew") == 0)
			{
				if(strstr(attributo, "_") == NULL)
				{
					int ris = pnew(padre, attributo, 0);
					if(ris != 0)
						gestisciErrori(ris);
				}
				else
					gestisciErrori(4);
			}
			else if(strcmp(comando, "pinfo") == 0)
			{
				Node* tmp = (Node*)calloc(1, sizeof(Node));
				if(tmp != NULL)
				{
					tmp = cerca(padre, attributo);
					if(tmp != NULL)
					{
						char* ch = calloc(LONGLEN,1);
						if(ch != NULL)
						{
							pinfo(tmp,ch);
							printf("%s\n", ch);
						}
						else
							gestisciErrori(9);
					}
					else
						gestisciErrori(1);
				}
				else
					gestisciErrori(9);
			}
			else if(strcmp(comando, "pclose") == 0)
			{
				int ris = prePClose(padre, attributo);
				if(ris != 0)
					gestisciErrori(ris);
			}
			else if(strcmp(comando, "pspawn") == 0)
			{
				char* option = (char*)calloc(6, sizeof(char));
				if(option != NULL)
				{
					option = strtok(NULL, " ");
					if(option != NULL)
					{
						int ris = prePSpawn(padre, attributo, option);
						if(ris != 0)
							gestisciErrori(ris);
					}	
					else
					{
						int ris = pspawn(padre, attributo);
						if(ris != 0)
							gestisciErrori(ris);
					}
				}
				else
				{
					gestisciErrori(9);
				}
			}
			else if(strcmp(comando, "prmall") == 0)
			{
				prmall(padre, attributo);
			}
			else
				gestisciErrori(5);
		}
	}
	else
		gestisciErrori(5);

	return 0;
}

void menu()
{
	char line[MAX_LENGTH];

	int i=0;
	while(i==0)
	{
		printf("$ ");
		if (!fgets(line, MAX_LENGTH, stdin))
			printf("Errore, prego inserire nuovamente il comando\n");
		else
			i = psystem(line); //esegue quello che c'e' scritto in line
 	}
}

void file(char* nomeFile)
{
	FILE *fd;
	char *res = (char*)calloc(MAX_LENGTH,sizeof(char));
	if(res == NULL)
	{
		printf("Errore di memoria, non posso aprire il file");
		return;
	}

	/* apre il file */
	fd = fopen(nomeFile, "r");
	if(fd == NULL)
	{
		perror("Errore: ");
		exit(1);
	}

	/* legge e stampa ogni riga */
	int i = 0;
	while(i == 0)
	{
		if(fgets(res, 200, fd) == NULL)
		{
			perror("File");
			break;
		}
		i = psystem(res);
	}


	/* chiude il file */
	if(fclose(fd) == EOF)
		perror("Error: ");
}

int main(int argc, char* argv[])
{
	padre = init();
	
	if(padre == NULL)
	{
		gestisciErrori(10);
		exit(2);
	}
	
	if(signal(SIGINT,SIG_IGN) == SIG_ERR)
	{
		gestisciErrori(10);
		exit(3);
	}
		
	if(argc == 1)
		menu();
	else
	{
		file(argv[1]);
		menu();
	}

	return 0;
}
