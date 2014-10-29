#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
  int status;
  pid_t pid_1, pid_2, pid_3, pid_4;
	int fds1[2];
	int fds2[2];
	int fds3[2];

  if (argc != 4) {
    printf("usage: finder DIR STR NUM_FILES\n");
    exit(0);
  }

	pipe(fds1);
	pipe(fds2);
	pipe(fds3);
  pid_1 = fork();
  if (pid_1 == 0) {
    /* First Child */
	char cmdbuf[BSIZE];
	bzero(cmdbuf, BSIZE);
	sprintf(cmdbuf,"%s %s -name \'*\'.[ch]",FIND_EXEC,argv[1]);
	dup2(fds1[1], STDOUT_FILENO);
	close(fds1[0]);
	close(fds2[0]);
	close(fds2[1]);
	close(fds3[0]);
	close(fds3[1]);
	if(execl(BASH_EXEC,BASH_EXEC,"-c",cmdbuf,(char*)0)<0){
		fprintf(stderr, "\nCould not be found%d\n", errno);
		return EXIT_FAILURE;
	}
    exit(0);
  }
	close(fds1[1]);

  pid_2 = fork();
  if (pid_2 == 0) {
    /* Second Child */
	dup2(fds1[0], STDIN_FILENO);
	dup2(fds2[1], STDOUT_FILENO);
	close(fds1[1]);
	close(fds2[0]);
	close(fds3[0]);
	close(fds3[1]);
	if(execl(XARGS_EXEC,XARGS_EXEC,GREP_EXEC,"-c",argv[2],NULL)<0){
		fprintf(stderr, "\nCould not be executed. ERROR%d\n", errno);
		return EXIT_FAILURE;
	}
    exit(0);
  }
	close(fds2[1]);
  pid_3 = fork();
  if (pid_3 == 0) {
    /* Third Child */
	dup2(fds2[0], STDIN_FILENO);
	dup2(fds3[1], STDOUT_FILENO);
	close(fds2[1]);
	close(fds3[0]);
	close(fds1[0]);
	close(fds1[1]);
	if(execl(SORT_EXEC,"sort","-t",":","+1.0","-2.0","--numeric","--reverse",NULL)<0){
		fprintf(stderr, "\nCould not be executed. ERROR%d\n", errno);
		return EXIT_FAILURE;
	}
    exit(0);
  }
	close(fds3[1]);

  pid_4 = fork();
  if (pid_4 == 0) {
    /* Fourth Child */
	dup2(fds3[0],STDIN_FILENO);
	close(fds3[1]);
	close(fds2[0]);
	close(fds2[1]);
	close(fds1[0]);
	close(fds1[1]);
	if(execl(HEAD_EXEC,"head","--lines",argv[3],NULL)<0){
		fprintf(stderr, "\nHead could not execute. ERROR%d\n",errno);
		return EXIT_FAILURE;
	}
    exit(0);
  }

  if ((waitpid(pid_1, &status, 0)) == -1) {
    fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_2, &status, 0)) == -1) {
    fprintf(stderr, "Process 2 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_3, &status, 0)) == -1) {
    fprintf(stderr, "Process 3 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }
  if ((waitpid(pid_4, &status, 0)) == -1) {
    fprintf(stderr, "Process 4 encountered an error. ERROR%d", errno);
    return EXIT_FAILURE;
  }

  return 0;
}
