#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h>
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

int main(void) {
	char *args[20]; int bg;
	struct command history_command;
	int t_count = -1;
	struct command all_commands[10];
	
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

		
		if(args[0][0]=='!') {
			history_command = getHistory(args[0], all_commands);
			for(int k=0; k<history_command.length; k++){
				args[k] = history_command.args[k];
			}
			cnt = history_command.length;
		}

		//Make sure last args entry is null
		args[cnt] = NULL;
		struct command commandi;

		commandi.count = t_count;
		commandi.length = cnt;
		for(int i=0; i<cnt; i++){
			commandi.args[i] = args[i];
		}

		all_commands[t_count%10] = commandi;

		pid_t process = fork();
		if(process==0) {
			//child
			if(!strcmp(args[0], "history")){
				for(int i=t_count; i>t_count-10 && i>-1; i--){
					printf("%d : ", all_commands[i%10].count);

					for(int j=0; j<all_commands[i%10].length; j++){
						printf("%s ", all_commands[i%10].args[j]);
					}

					printf("\n");
				}
			}else {
				execvp(args[0], args);
				exit(0);
			}
			
		}else {
			//parent
			if(!bg){
				int status;
				waitpid(process, &status, 0);
			}
		}
	}
 }