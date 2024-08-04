#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#define DIR_STACK_SZ 16

char *dir_stack[DIR_STACK_SZ];
int dir_stack_top = -1;
char *pwd;

void print_prompt(){
	fprintf(stdout, "gshell:%s $ ", dir_stack[dir_stack_top]);
	fflush(stdout);
}

int handle_cd(char *args){
	char *to_dir;

	to_dir = args;
	if(to_dir == NULL){
		to_dir = getenv("HOME");
	}

	if(chdir(to_dir) != 0){
		fprintf(stderr, "cd: %s: No such file or directory\n", args);
		return -1;
	}

	free(dir_stack[dir_stack_top]);
	pwd = getcwd(NULL, 0);
	if(pwd == NULL){
		perror("getcwd");
		return -1;
	}
	dir_stack[dir_stack_top] = pwd;

	return 0;
}

int handle_dirs(char *args){
	int i;
	for (i = dir_stack_top; i >= 0; i--){
		fprintf(stdout, "%s\n", dir_stack[i]);
	}
	return 0;
}

int handle_pushd(char *args){
	char *cur_dir;
	if (dir_stack_top >= DIR_STACK_SZ -1){
		fprintf(stderr, "pushd: Directory stack full\n");
		return -1;
	}

	cur_dir = getcwd(NULL, 0);
	if(cur_dir == NULL){
		perror("getcwd");
		return -1;
	}

	if(chdir(args) != 0){
		fprintf(stderr, "pushd: %s: No such file or directory\n", args);
		free(cur_dir);
		return -1;
	}

	dir_stack[++dir_stack_top] = cur_dir;
	pwd = getcwd(NULL, 0);
	if(pwd == NULL){
		perror("getcwd");
		return -1;
	}
	dir_stack[dir_stack_top] = pwd;

	return 0;
}

int handle_popd(char *args){
	if(dir_stack_top <= 0){
		fprintf(stderr, "popd: Dircetory stack is empty\n");
		return -1;
	}

	char *prev_dir = dir_stack[dir_stack_top];
	dir_stack[dir_stack_top--] = NULL;
	
	if(chdir(prev_dir) != 0){
		fprintf(stderr, "popd: %s: No such file or directory\n", prev_dir);
		return -1;
	}

	free(prev_dir);

	return 0;
}

void handle_external(char* cmd, char *args){
	pid_t pid;
	char *arg_list[3];
	int exec_status;

	pid = fork();
	if (pid == 0){
		arg_list[0] = cmd;
		arg_list[1] = args;
		arg_list[2] = NULL;

		execv(cmd, arg_list);
		fprintf(stdout, "Failed to execute %s\n", cmd);
		exit(EXIT_FAILURE);
	} else if(pid > 0){
		waitpid(pid, &exec_status, 0);
	} else if(pid == -1) {
		perror("fork");
	}
}

int main(void){
	int ret = -1;
	char input_line[1024];
	char *cmd;
	char *args;

	int i;
	//initialise the dir stack memory
	for(i = 0; i < DIR_STACK_SZ; i++){
		dir_stack[i] = NULL;
	}

	pwd = getcwd(NULL, 0);
	if(pwd == NULL){
		perror("getcwd");
		return -1;
	}
	dir_stack[++dir_stack_top] = pwd;

	//shell main loop
	while(1){
		print_prompt();

		if(fgets(input_line, 1024, stdin) == NULL){
			continue;
		}

		//extract the cmd and args from the input line
		input_line[strcspn(input_line, "\n")] = 0;
		cmd = strtok(input_line, " ");
		if(cmd == NULL){
			continue;
		}
		args = strtok(NULL, " ");

		if(strcmp(cmd, "exit") == 0){
			fputs("exit\n", stdout);
			exit(0);
		} else if(strcmp(cmd, "cd") == 0){
			ret = handle_cd(args);
		} else if(strcmp(cmd, "pushd") == 0){  
                        ret = handle_pushd(args);
                } else if(strcmp(cmd, "popd") == 0){  
                        ret = handle_popd(args);
                } else if(strcmp(cmd, "dirs") == 0){  
                        ret = handle_dirs(args);
                } else {
			handle_external(cmd, args);
		}
	}

	// freeup the dir stack memory
	for(i = 0; i<=dir_stack_top; i++){
		free(dir_stack[i]);
	}

	return ret;
}
