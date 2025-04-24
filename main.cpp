#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE 80  /* The maximum length command */

int main(int argc, char* argv[])
{ 
char *args[MAX_LINE/2 + 1]; /* command line arguments */
int should_run = 1; /* flag to determine when to exit program */
char *history[MAX_LINE/2 +1];
history[0]="NULL";
for(int i=0;i<argc-1;i++) {
	args[i]=argv[i+1];
}
args[argc-1]=NULL;

while (should_run) {
printf("osh&#x003E;");
fflush(stdout);

/**
* After reading user input, the steps are:
* (1) fork a child process using fork()
* (2) the child process will invoke execvp()
* (3) parent will invoke wait() unless command included &
*/



int status=0;
pid_t p = fork();
if(p<0) {
	perror("fork fail\n");
	should_run=0;
	//exit(1);
} else if(p==0) {
	if(args[0]=="!!") {
		if(strcmp(history[0],"NULL")) {
			printf("No commands in history\n");
		} else {
			status = execvp(history[0], history);
		}
	} else {
		status = execvp(args[0], args);
		for(int i=0;i<sizeof(history);i++) {
			history[i]=args[i];
		}
		args[sizeof(history)]=NULL;
	}
	if(status==-1) {
		printf("exec fail\n");
		//exit(1);
		should_run=0;
	}
	
} else {
	int count=0;
	for(int i =0; i<MAX_LINE/2 +1; i++) {
		if(args[i]=="&") {
			count++;
		}
	}
	if(count=0) {
		p=wait(NULL);
	}
}

return 0;
}
}
