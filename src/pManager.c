#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "Node.c"

//Per aumentare la sicurezza del programma, nella libreria andrà implementata la funzione restituisci_radice
//Che mi ritornerà la radice già creata, da lì io potrò fare solo le operazioni sulla radice

/*
 * da fare:
 * progress in tutte le azioni che richiedono più di una stampa
*/

Node *padre;
#define MAX_LENGTH 1024

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void file(char*);

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
			printf(ANSI_COLOR_GREEN "Nodo non trovato!\n" ANSI_COLOR_RESET);
			break;
		case 2:
			printf(ANSI_COLOR_GREEN "Inserisci un nome piu' breve!\n" ANSI_COLOR_RESET);
			break;
		case 3:
			printf(ANSI_COLOR_RED "Nome non consentito!\n" ANSI_COLOR_RESET);
			break;
		case 4:
			printf(ANSI_COLOR_RED "Nome non valido, rimuovi il carattere _\n" ANSI_COLOR_RESET);
			break;
		case 5:
			printf(ANSI_COLOR_GREEN "Comando non trovato, usa " ANSI_COLOR_RESET "phelp " ANSI_COLOR_GREEN "per la lista dei comandi!\n" ANSI_COLOR_RESET);
			break;
		case 6:
			printf(ANSI_COLOR_RED "Il processo non puo' terminare poiche' ha figli!\n" ANSI_COLOR_RESET);
			break;
		case 7:
			printf(ANSI_COLOR_RED "Esiste già un figlio con questo nome\n" ANSI_COLOR_RESET);
			break;
		case 8:
			printf(ANSI_COLOR_RED "Impossibile creare il processo\n" ANSI_COLOR_RESET);
			break;
		case 9:
			perror("Error");
			errno = 0;
			printf(ANSI_COLOR_RED "Comando non eseguito\n" ANSI_COLOR_RESET);
			break;
		case 10:
			sleep(2);
			printf(ANSI_COLOR_RED "Procedo alla terminazione di tutti i processi:\n");
			sleep(2);
			errorquit(padre);
			printf(ANSI_COLOR_MAGENTA "FATAL ERROR, prova di nuovo, sarai più fortuanto\n" ANSI_COLOR_RESET);
			system("rm -f assets/Lettura_*; rm -f assets/Scrittura_*; rm src/tmp;");
			exit(10);
			break;
		case 11:
			printf(ANSI_COLOR_GREEN "Argomento non valido\n" ANSI_COLOR_RESET);
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
					plist(padre, ch);
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
			else if(strcmp(comando, "pexport") == 0)
			{
				int ris = pexport(padre);
				if(ris != 0)
					gestisciErrori(ris);
				else
					printf("Export del file avvenuto correttamente, trovi il file in src/log.txt\n");
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
			else if(strcmp(attributo, "pmanager") == 0 || strcmp(attributo, "pManager") == 0)
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
						int ris = pspawn(padre, attributo, 0);
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
			else if(strcmp(comando, "ptree") == 0)
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
							ptree(tmp, 1, ch);
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
			else if(strcmp(comando, "plist") == 0)
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
							plist(tmp, ch);
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
			else if(strcmp(comando, "pfile") == 0)
			{
				file(attributo);
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
	char *line = (char*)calloc(MAXLEN, sizeof(char));

	int i = 0;
	while(i == 0)
	{
		printf("$ ");
		if (fgets(line, MAX_LENGTH, stdin) == NULL)
		{
			printf("\nErrore, prego inserire nuovamente il comando\n");
			fseek(stdin,0,SEEK_END);
		}
		else if(line[0] == '\n')
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
			if(errno != 0)
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
	int fd = open("src/tmp", O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);

	if (errno == EEXIST)
	{
		printf("pManager gia' in esecuzione... terminare l'esecuzione in modo corretto!\nIn caso di necessita' terminare il processo pManager, eliminare il file src/tmp e riavviare pManager.\n");
		exit(3);
	}

	padre = init();

	if(padre == NULL)
	{
		gestisciErrori(10);
		exit(2);
	}

	if(signal(SIGINT, SIG_IGN) == SIG_ERR)
	{
		gestisciErrori(10);
		exit(3);
	}
	if(signal(SIGTSTP, SIG_IGN) == SIG_ERR)
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

	system("rm -f src/tmp;");

	return 0;
}
