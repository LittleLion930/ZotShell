#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>

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

char input[MAX_LINE];
fgets(input, MAX_LINE, stdin);
input[strcspn(input, "\n")] = 0;

int index = 0;
char *token = strtok(input, " ");
char *input_file = NULL;
char *output_file = NULL;
int redirect_in = 0, redirect_out = 0;

int pipe_flag = 0;

while (token != NULL) {
	if (strcmp(token, "<") == 0) {
		redirect_in = 1;
		token = strtok(NULL, " ");
		input_file = token;
	} else if (strcmp(token, ">") == 0) {
		redirect_out = 1;
		token = strtok(NULL, " ");
		output_file = token;

	} else if (strcmp(token, "|") == 0) {
		pipe_flag = 1;
		args[index] = NULL;
	} else {
		args[index++] = token;
	}

	token = strtok(NULL, " ");
}
args[index] = NULL;

if (pipe_flag) {
	int pipe_fd[2];
	if (pipe(pipe_fd) < 0) {
		perror("Pipe creation failed");
		should_run = 0;
		continue;
	}

	char *args1[MAX_LINE/2 + 1];
	char *args2[MAX_LINE/2 + 1];
	int split_index = 0;

	for (int i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "|") == 0) {
			split_index = i;
			break;
		}
	}

	int j = 0;
	for (int i = split_index + 1; args[i] != NULL; i++, j++) {
		args2[j] = args[i];
	}
	args2[j] = NULL;

	pid_t p1 = fork();
	if (p1 < 0) {
		perror("Fork failed");
		should_run = 0;
		continue;
	} else if (p1 == 0) {
		if (redirect_in) {
			int fd0 = open(input_file, O_RDONLY);
			if (fd0 < 0) { perror("Input redirection failed"); return 1; }
			dup2(fd0, STDIN_FILENO);
			close(fd0);
		}

		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[0]);
		close(pipe_fd[1]);

		execvp(args[0], args);
		perror("Exec failed");
		return 1;
	} else {
		pid_t p2 = fork();
		if (p2 < 0) {
			perror("Fork failed");
			should_run = 0;
			continue;
		} else if (p2 == 0) {
			if (redirect_out) {
				int fd1 = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
				if (fd1 < 0) { perror("Output redirection failed"); return 1; }
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}

			dup2(pipe_fd[0], STDIN_FILENO);
			close(pipe_fd[1]);
			close(pipe_fd[0]);

			execvp(args[index], &args[index + 1]);
			perror("Exec failed");
			return 1;
		} else {
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			waitpid(p1, NULL, 0);
			waitpid(p2, NULL, 0);
		}
	}
} else {

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

		if (redirect_in) {
			int fd0 = open(input_file, O_RDONLY);
			if (fd0 < 0) { perror("Input redirection failed"); return 1; }
			dup2(fd0, STDIN_FILENO);
			close(fd0);
		}

		if (redirect_out) {
			int fd1 = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
			if (fd1 < 0) { perror("Output redirection failed"); return 1; }
			dup2(fd1, STDOUT_FILENO);
			close(fd1);
		}

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
}
}
return 0;
}

