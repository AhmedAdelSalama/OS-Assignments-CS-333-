#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <unistd.h>
#include <limits.h>

/*Maximum possible number of characters a command can handle*/
const int MAX_LENGTH = 2047;
/*log file*/
FILE *file;

/*Signal handler when a child terminated*/
void sigchld_handler();
/*A function to split one command line to a list of strings of arguments*/
void split(char *line, char **args);
/*checks to execute the command and return immediately or block the command until finishing*/
int execInBackground(char *line);

int main(){
    file = fopen("logger.log", "a+");
    signal(SIGCHLD, sigchld_handler);

    if (file == NULL) {
        fprintf(stderr, "Failed to open log file..\n");
        exit(1);
    }
    while (1){
        char *line = malloc(MAX_LENGTH);

        //printf("%s@%s:%s$ ",hostname, username, cwd);
        printf("Shell > ");
        fgets(line, MAX_LENGTH, stdin);
        /* check if the command is "exit" then terminate*/
        char exitCheck[5];
        strcpy(exitCheck, line);
        if(strcmp(exitCheck, "exit\n")==0){
            exit(0);
        }
        /*string proceesing: splitting the line into number of arguments*/
        char *args[100];
        split(line, args);

        /*fork a child process*/
        pid_t parent = getpid();
        pid_t pid = fork();

        if(pid<0){
            fprintf(stderr, "Fork Failed\n");
            fprintf(file, "Fork Failed\n");
            return 1;
        }else if(pid == 0){//child process

            int res = execvp(args[0], args);
            if(res = -1){
                printf("command not found\n");
                fprintf(file, "command not found\n");
                _exit(EXIT_FAILURE);
            }
            //execvp("bin/"+*args[0]+'/', args);
            exit(0);

        }else{//parent process
            if(!execInBackground(line)){
                int status;
                wait(&status);
                //printf("Child process was terminated\n");
                //fprintf(file, "Child process was terminated\n");
            }
        }
        free(line);
    }
    fclose(file);
   return 0;
}
/*Signal handler when a child terminated*/
void sigchld_handler(){
    fprintf(file, "Child process was terminated\n");
}
/*A function to split one command line to a list of strings of arguments*/
void split(char *line, char **args){
    int  i =0,j=0;
    int lineMaxLength = strlen(line);
    int wordMaxSize =1000;
    while(i<lineMaxLength && line[i]!='\n' && line[i]!='\0'){
        char *word = malloc(wordMaxSize);
        int count = 0;
        /*sparating arguments*/
        while(j<wordMaxSize && i< lineMaxLength &&line[i]!=' '&&line[i]!='\n' && line[i]!='\0'){
            /*check if this "&" for background execution or a normal character (escaped or beteween quotes)*/
            if(line[i]=='&'){
                if(i>0 && line[i-1]!='\\'){
                    break;
                }
            }
            /*ignore "\" if it is used to escape a character*/
            if(line[i]=='\\'){
                i++;
                continue;
            }
            /*If there is a word/sentece between quotes, it's one argument --> consider it a word*/
            if((line[i]=='\''||line[i]=='\"') && (i>0 && line[i-1]!='\\')){
                char quotMark = line[i++];
                while(line[i] != quotMark){
                    word[count] = line[i];
                    count++;
                    i++;
                }
                i++;
                continue;
            }
            word[count] = line[i];
            count++;
            i++;
        }
        args[j] = word;
        ++i;
        ++j;
    }
    args[j] = NULL;
}

/*checks to execute the command and return immediately or block the command until finishing*/
int execInBackground(char *line){
    int lineMaxLength = strlen(line);
    for(int i = lineMaxLength-1; i>=0; --i){
        /* if & is not escaped(ex: \&) or preceded \"not escaped" (ex: aa\\&)*/
        if(line[i]=='&'){
            if(i>0 && (line[i-1]!='\\'||(line[i-1]=='\\'&&line[i-2]=='\\'))){
                return 1;
            }
            return 0;
        }
        if(line[i] != '\n' && line[i] !=' ')
            return 0;
    }
    return 0;
}
