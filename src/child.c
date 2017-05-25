#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MINLEN 10
#define PIPELEN 25
#define MAXLEN 100
char* pipeLettura;
char* pipeScrittura;

int returnErrno()
{
	if(errno != 0)
	{
		strerror(errno);
		errno = 0;
		return -1;
	}

	return 0;
}

int readLine (int idPipeLettura,char *str) {
	int n;

	do {  //Read characters until '\0' or end-of-input
		n = read(idPipeLettura, str, 1); //Read one character
	} while (n > 0 && *str++ != '\0'); //Controllo che non ci siano errori

	return returnErrno();
}

int readPipe(char *str) {
	//char* str = (char*)calloc(MAXLEN,sizeof(char));

	int fd = open(pipeScrittura, O_RDONLY); /* Open it for reading */ /* Open it for reading */

	if(errno != 0)
		return 10;

	int ris = readLine(fd, str); /* Display received messages */

	/* chiude il file */
	if(close(fd) == EOF)
	{
		perror("Error");
		return 10;
	}

	return ris;
}

int writePipe(char* nomePipe, char* str) {
	int fd;

	fd = open(nomePipe, O_WRONLY); /* Open it for reading */
	if(returnErrno() != 0)
		return 10;

	int n = write (fd, str, strlen(str)); /* Write message down pipe */

	if(n < 0)
		return 10;

	if(close(fd) == EOF)
	{
		perror("Error");
		return 10;
	}

	return 0;
}

int generaFiglio(char* status)
{
	char* pch = (char*)calloc(MAXLEN,sizeof(char));
	char* name = (char*)calloc(MAXLEN,sizeof(char));
	char* id = (char*)calloc(MINLEN,sizeof(char));

	if(returnErrno() != 0)
		return -1;

	strtok (status," ");
	name = strtok (NULL," ");
	id = strtok (NULL,".");

	signal(SIGCHLD, SIG_IGN);

	int p = fork();
	if(p == 0)
		execl("build/child", name, id, (char*) NULL);
	else if(p < 0)
		return -1;

	sprintf(pch, "Clonazione avvenuta: proceso <%s> generato", id);
	writePipe(pipeLettura, pch);

	return 0;
}

int main (int argc, char* argv[])
{
	char message [MAXLEN];
	char* status = (char*)calloc(MAXLEN, sizeof(char));
	pipeLettura = (char*)calloc(PIPELEN, sizeof(char));
	pipeScrittura = (char*)calloc(PIPELEN, sizeof(char));

	if(status == NULL || pipeLettura == NULL || pipeScrittura == NULL)
	{
		strcpy(message,"ERRORI IN FASE DI AVVIO");
		writePipe(pipeLettura, message);
		return 0;
	}

	snprintf(pipeLettura, PIPELEN, "%s%s", "pipe/Lettura_", argv[1]);
	snprintf(pipeScrittura, PIPELEN, "%s%s", "pipe/Scrittura_", argv[1]);

	/* Prepare message */
	sprintf(message, "Processo <%s> avviato,%d", argv[1],getpid());
	if(writePipe(pipeLettura, message) != 0) /* Write message down pipe */
		return 0;

	do {
		if(readPipe(status)!= 0)
			return 0;

		if(strlen(status) > 4)
		{
			if(generaFiglio(status) != 0)
			{
				strcpy(message,"IMPOSSIBILE AVVIARE IL FIGLIO");
				if(writePipe(pipeLettura, message) != 0)
					return 0;
			}
		}
	} while(strcmp(status, "EXIT") != 0);

	sprintf(message, "Processo <%s> terminato", argv[1]);
	if(writePipe(pipeLettura, message) != 0)
		return 0;

	unlink(pipeLettura);
	unlink(pipeScrittura);
	
	exit(0);
}
