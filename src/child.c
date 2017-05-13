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

void readLine (int idPipeScrittura,char *str) {
	/* Read a single '\0'-terminated line into str from fd */
	/* Return 0 when the end-of-input is reached and 1 otherwise */
	int n;

	do { /* Read characters until '\0' or end-of-input */
		n = read (idPipeScrittura, str, 1); /* Read one character */
	} while (n > 0 && *str++ != '\0');
}

int readPipe(char* str)
{
	int fd = open(pipeScrittura, O_RDONLY); /* Open it for reading */
	
	if(returnErrno() != 0)
		return -1;
		
	readLine(fd, str);
	close(fd);
	
	return 0;
}

void writePipe(char* nomePipe, char* str) {
	int fd;

	do {
		fd = open(nomePipe, O_WRONLY);
		//returnErrno(); /* Open it for reading */
	} while (fd == -1);

	write (fd, str, strlen(str)); /* Write message down pipe */
	close(fd);
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
	
	snprintf(pipeLettura, PIPELEN, "%s%s", "assets/Lettura_", argv[1]);
	snprintf(pipeScrittura, PIPELEN, "%s%s", "assets/Scrittura_", argv[1]);

	/* Prepare message */
	sprintf(message, "Processo <%s> avviato", argv[1]);
	writePipe(pipeLettura, message); /* Write message down pipe */

	do {
		if(readPipe(status)!= 0)
			status = "NO";
		

		if(strlen(status) > 4)
			if(generaFiglio(status) != 0)
			{
				strcpy(message,"IMPOSSIBILE AVVIARE IL FIGLIO");
				writePipe(pipeLettura, message);
			}

	} while(strcmp(status, "EXIT") != 0);

	sprintf(message, "Processo <%s> terminato", argv[1]);
	writePipe(pipeLettura, message);

	unlink(pipeLettura);
	unlink(pipeScrittura);

	exit(0);
}
