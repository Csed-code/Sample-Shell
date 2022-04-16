#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SIZE 1024

int file_exists;


void proc_exit()
{
    int wstat;
    pid_t	pid;
    while (true) {
        pid = wait3 (&wstat, WNOHANG, (struct rusage *)NULL );
        if (pid == 0)
            return;

        else if (pid == -1)
            return;
        else
        {
            char *message = "a child process is terminated\n";
            write(file_exists,message , strlen(message));
            return;
        }
    }

}

void GetStr(char **args)
{

    for(int i=0 ; 1 ; i++)
    {
        //store location in memory
        if(i)
            *args = (char*)realloc((*args) , i + 1);
        else
            *args = (char*)malloc(i + 1);
        //take character
        (*args)[i]=getchar();
        if((*args)[i] == '\n')
        {
            (*args)[i]= '\0';
            break;
        }
    }
}

void get_args(char* str,char** args,int num){
    int i = 0;
    //split command line to argument
    args[i++] = strtok(str, " ");
    for(int j =0;j<=num;j++) {
        args[i++] = strtok(NULL, " ");
    }
}

int Spaces(char* str){
    int numOfSpace = 0;
    //count number of spaces in command line
    while(*str != '\0'){
        if(*str == ' ')
            numOfSpace++;
        str++;
    }
    return numOfSpace;
}
void  execute_command(char* command,char* args[],int num) {
    char **try = (char **)malloc(SIZE);
    if (num == 0) {
        //check if arguments have "$" or not
        char *ret = strstr(args[0], "$");
        if (ret != NULL) {
            //evaluating environment variable
            args[0] = strtok(args[0], "$");
            args[0] = getenv(args[0]);
        }
        command = args[0];
        try[0] = command;
    }
    else{
        char* temp = NULL;
        int j=1;
        try[0] = command;
        for (int i =1; i <= num; i++) {
            //check if arguments have "$" or not
            char *ret = strstr(args[i], "$");
            if (ret != NULL) {
                //split environment variable
                temp = strtok(getenv(strtok(args[i], "$"))," ");
                while (temp != NULL)
                {
                    try[j++] = temp;
                    temp = strtok(NULL, " ");
                }
            }
            else
                try[j++] =args[i];
        }
    }
    bool isForeground = (strcmp(args[num],"&"));
    //fork a child process
    int id = fork();
    if (!id) {
        execvp(command, try);
        printf("Error\n");
        exit(1);
    } else if(isForeground) {
        //wait the child terminate
        waitpid(id, 0, WUNTRACED);
    }
}

void addSpaces(char *data) {
    int len = strlen(data);
    memset( data+len, ' ', 1 );
}

void  execute_shell_bultin(char* args[],int num) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || strcmp(args[1], "~") == 0)
            // home directory
            chdir(getenv("HOME"));
        else
            // change the directory
            chdir(args[1]);
    } else if (strcmp(args[0], "echo") == 0) {

        for (int i = 1; i <= num; i++) {
            args[i] = strtok(args[i], "\"");
            //check if arguments have "$" or not
            char *ret = strstr(args[i], "$");
            if (ret != NULL) {
                //evaluating environment variable
                args[i] = getenv(strtok(args[i], "$"));
            }
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "export") == 0) {
        char *key = NULL;
        char *data = NULL;
        //store data and key of variable
        key = strtok(args[1], "=");
        data = strtok(strtok(NULL, "="), "\"");
        if(num>1)
            for(int i = 2;i<=num;i++)
                addSpaces(data);
        // store data without double quotations
        data = strtok(data, "\"");
        setenv(key, data, 1);
    }
}



int main() {
    signal (SIGCHLD, proc_exit);
    file_exists = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    char *command = NULL;
    do {
        GetStr(&command);
        if(strcmp(command, "exit") == 0) exit(0);
        int numOfSpaces = Spaces(command); // Get total number of args.
        char **args = (char **)malloc(SIZE);
        args[0] = command;
        if(numOfSpaces!=0) {
            args[numOfSpaces + 1] = NULL; //Arguments array must terminate with a null pointer.
            get_args(command, args,numOfSpaces); // Now we fill the arguments array.
        }
        bool expression = (strcmp(args[0], "cd") != 0 && strcmp(args[0], "echo") != 0 &&
                           strcmp(args[0], "export") != 0);

        if (!expression)
            execute_shell_bultin(args,numOfSpaces);
        else {

            execute_command(command, args, numOfSpaces);
        }
        free(command);
        free(args);
    } while (true);

}


