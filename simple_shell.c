#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>
#include <fcntl.h>
//
// This code is given for illustration purposes. You need not include or follow this
// strictly. Feel free to writer better or bug free code. This example code block does not
// worry about deallocating memory. You need to ensure memory is allocated and deallocated
// properly so that your shell works without leaking memory. //
int getcmd(char *prompt, char *args[], int *background) {
	int length, i = 0; char *token, *loc; char *line = NULL; size_t linecap = 0;
	printf("%s", prompt);
	length = getline(&line, &linecap, stdin);
	if (length <= 0) { 
		exit(-1);
	}
	// Check if background is specified.. 
	if ((loc = index(line, '&')) != NULL) {
		*background = 1;
		*loc = ' '; 
	} else
		*background = 0;

	while ((token = strsep(&line, " \t\n")) != NULL) {
		for (int j = 0; j < strlen(token); j++)
			if (token[j] <= 32) 
				token[j] = '\0';
		if (strlen(token) > 0) 
			args[i++] = token;
		}
		return i; 
	}

struct command{
	char *args[20];
	int count;
	int length;
};

struct BPROCESS{
	char *args[20];
	int pid;
	int length;
	int uid;
};

struct command getHistory(char *id, struct command all_c[10]){
	char *token;
	char *count;
	int selector;
	while((token=strsep(&id, "!"))!=NULL){
		count = token;
	}

	for(int i=0; i<10; i++){
		if(all_c[i].count == atoi(count)){
			selector = i;
			break;
		}
	}

	return all_c[selector];
}

void changeDir(char *arg){
	int success = chdir(arg);
	if(!success){
		printf("cwd -> %s\n", arg);
	}else{
		printf("Dir does not exist or is not dir");
	}
}

void currentDir(){
	size_t size = 100;
	char *buffer = (char *) malloc(size);
	if(getcwd(buffer, size)==buffer){
		 printf("%s\n", buffer);
	}
	free(buffer);
}

void getJobs(struct BPROCESS all_bp[2]){
	// int status;
	// waitpid(all_bp[0].pid, &status, WNOHANG);
	for(int i=0; i<2; i++){
		
		int status;
		waitpid(all_bp[i].pid, &status, WNOHANG);
		if(status!=0 && all_bp[i].pid!=0){
			printf("%d : %d : ", all_bp[i].uid, all_bp[i].pid);
			for(int k=0; k<all_bp[i].length; k++){
				printf("%s ", all_bp[i].args[k]);
			}
			printf("\n");
		}
	}
}

int main(void) {
	char *args[20]; int bg;
	struct command history_command;
	struct command all_commands[10];
	struct BPROCESS all_bprocesses[2];
	struct command commandi;
	struct BPROCESS bprocessi;
	int t_count = -1;
	int bprocess_count = 1;
	int output_file;
	int in_out[2];

	while(1) {
	    bg = 0;
		int cnt = getcmd("\n>> ", args, &bg);
		//Incriment # of commands
		t_count++;
		/* the steps can be..:
		(1) fork a child process using fork()
		(2) the child process will invoke execvp()
		(3) if background is not specified, the parent will wait,
			otherwise parent starts the next command... */

		/*History command access*/
		if(args[0][0]=='!') {
			history_command = getHistory(args[0], all_commands);
			for(int k=0; k<history_command.length; k++){
				args[k] = history_command.args[k];
			}
			cnt = history_command.length;
		}

		if(!cnt){
			continue;
		}

		//Make sure last args entry is null
		args[cnt] = NULL;

		/*Storing last 10 commands for history*/
		commandi.count = t_count;
		commandi.length = cnt;
		for(int i=0; i<cnt; i++){
			commandi.args[i] = args[i];
		}

		/*Circular array with modulus operator*/
		all_commands[t_count%10] = commandi;

		/*Built in commands*/
		if(!strcmp(args[0], "cd")){
			changeDir(args[1]);
			continue;
		}else if(!strcmp(args[0], "pwd")){
			currentDir();
			continue;
		}else if(!strcmp(args[0], "exit")){
			exit(0);
		}else if(!strcmp(args[0], "jobs")){
			getJobs(all_bprocesses);
			continue;
		}

		pid_t process = fork();
		if(process==0) {
			//CHILD

			/*Print history from all commands list*/
			if(!strcmp(args[0], "history")){
				for(int i=t_count; i>t_count-10 && i>-1; i--){
					printf("%d : ", all_commands[i%10].count);

					for(int j=0; j<all_commands[i%10].length; j++){
						printf("%s ", all_commands[i%10].args[j]);
					}

					printf("\n");
				}
			}else{
				/*Check for redirect or pipes*/
				for(int t=0; t<cnt; t++){

					/*REDIRECT*/
					if(!strcmp(args[t], ">")){
						cnt = t;
						close(1);
						output_file = open(args[t+1], O_WRONLY);
						args[cnt] = NULL;

					/*PIPE*/
					}else if(!strcmp(args[t], "|")){
						int mid = t;

						/*Left hand side arguments from 0 to t-1*/
						args[t] = NULL;

						/*Create pipe*/
						pipe(in_out);

						/*Create a child process to run left hand side*/
						int child = fork();
						if(child==0){
							close(1);
							close(in_out[0]);
							dup(in_out[1]);

							execvp(args[0], args);
							close(in_out[1]);
							exit(0);
						}else{
							int status;
							waitpid(child, &status, 0);

							close(0);
							close(in_out[1]);
							dup(in_out[0]);
							char *right[cnt-t];

							/*Right hand side args from t+1 to cnt will be replaced from 0 to cnt*/
							for(int c=t+1; c<cnt; c++){
								right[c-t-1] = args[c];
							}

							right[cnt-t-1] = NULL;
 							execvp(right[0], right);
							close(in_out[0]);
							exit(0);
						}
					}
				}
				execvp(args[0], args);
			}

			exit(0);
			
		}else {
			//PARENT
			if(!bg){
				int status;
				waitpid(process, &status, 0);
			}else{
				// /*New background process to be added to bprocess_list*/
				all_bprocesses[bprocess_count-1].uid = bprocess_count;
				all_bprocesses[bprocess_count-1].pid = process;
				all_bprocesses[bprocess_count-1].length = cnt;
				for(int l=0; l<cnt; l++){
					all_bprocesses[bprocess_count-1].args[l] = args[l];
				}
				all_bprocesses[bprocess_count-1].args[cnt] = NULL;
				// printf("%d\n", all_bprocesses[bprocess_count-1].pid);
				bprocess_count++;
			}
		}
	}
 }