#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LENGTH 2048

// globla variable
int background = 1;

struct commands {
    // A structure that store commands, arguments, and files.
    char* cmd[513];
    char* inputFile;
    char* outputFile;
    int background; // 1: background 0: foreground.
    int num_args;
};

struct commands processLine() {
    // Parse the passed line and store them into struct commands.
    char* line = malloc(MAX_LENGTH * sizeof(char));
    size_t buffersize = MAX_LENGTH;
    struct commands command;

    // Initialize struct.
    for (int i = 0; i < 512; i ++) 
        command.cmd[i] = NULL;
    command.num_args = 0;
    command.background = 0;
    command.inputFile = NULL;
    command.outputFile = NULL;



    // Print if childs are ternimated.
    int exitStatus;
    pid_t processID = waitpid(0, &exitStatus, WNOHANG);
    while (processID > 0) {
        if(WIFEXITED(exitStatus)){
            printf("Child exited normally with status %d\n", WEXITSTATUS(exitStatus));
        } else{
            printf("Child with pid %d exited abnormally due to signal %d\n", processID, WTERMSIG(exitStatus));
        }
    processID = waitpid(0, &exitStatus, WNOHANG);
    }
    
    printf(": ");
    fflush(stdout);
    getline(&line, &buffersize, stdin);

    // Remove Newline
    int newLine = 0;
    for (int i = 0; i < buffersize && !newLine; i++) {
        if (line[i] == '\n') {
            line[i] = '\0';
            newLine = 1;
        }
    }

    // Blank input
    if (strcmp(line, "") == 0) {
        // fprintf(stdout, "this is a comment line starting with space!\n");
        command.cmd[0] = "";
        return command;
    } 

    // $$ Expansion of Variable
    char Pid[32];
    sprintf(Pid, "%d", getpid());
    char* arr = malloc(sizeof(char) * MAX_LENGTH);
    int j = 0;

    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '$' && line[i+1] == '$') {

            for (; j < i; j++) {
                strncat(arr, &line[j], 1);
            }
            j += 2;
            i += 1;
            strcat(arr, Pid);

        }
    }

    for (; j < strlen(line); j++) {
        strncat(arr, &line[j], 1);
    }

    // Parse arguments into command.cmd
    char* saveptr;
    char* token = strtok_r(arr, " ", &saveptr);

    for (int i = 0; token != NULL; i++) {
        command.cmd[i] = strdup(token);
        
        token = strtok_r(NULL, " ", &saveptr);
    }

    return command;
}

void printStatus(int status) {
    if (WIFEXITED(status)) 
        printf("exit value %d\n", WEXITSTATUS(status));     
    else if (status == 1) 
        printf("terminated by signal %d\n", WTERMSIG(status));
    }

void execution (struct commands command, int* exitStatus, struct sigaction sig) {
    // Execute the commands in command.cmd[0], and take the arguments.
    pid_t spawnpid = -5;
    int inFD, outFD, newFD;
    spawnpid = fork();

    switch (spawnpid)
    {
    case -1:
        perror("fork() failed!");
        exit(1);
        break;
    case 0:
        // chile process will execute it
        // Input file redirection
        if (command.inputFile) {
            inFD = open(command.inputFile, O_RDONLY);
            if (inFD == -1) {
                perror("Cannot open file!\n");
                *exitStatus = 1;
            } 
            // Redirect file descriptor
            else {
                if (dup2(inFD, 0) == -1) {
                    perror("Cannot redirect file!\n");
                }
            }
        }

        //Output file redirection
        if (command.outputFile) {
            outFD = open(command.outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (outFD == -1) {
                perror("Cannot open file!\n");
                *exitStatus = 1;
            } 
            // Redirect file descriptor
            else {
                if (dup2(outFD, 1) == -1) {
                    perror("Cannot redirect file!\n");
                }
            }
        }

        // execute it
        if (execvp(command.cmd[0], (char* const*)command.cmd)) {
				printf("%s: no such file or directory\n", command.cmd[0]);
				fflush(stdout);
				exit(1);
			}

        //  prevent sharing
        fcntl(inFD, F_SETFD, FD_CLOEXEC);
        fcntl(outFD, F_SETFD, FD_CLOEXEC);  

        break;
    default:
        // parent process will execute it
        if (!command.background) {
            // when it is foreground mode
            waitpid(spawnpid, exitStatus, 0);
            
        } else{
            // Background mode
            printf("Background Process ID is %d\n", spawnpid);
            fflush(stdout);
        }

        break;
    }

    return;
}

void sigfunc(int signal) {
    // signal handlling function for ctrl z
    if (background == 1) {
        char* message = "Entering foreground-only mode (& is now ignored)\n";
        write(1, message, 49);
        fflush(stdout);
        background = 0;
        }

    else {
        // If it's 0, set it to 1 and display a message reentrantly
        char* message = "Exiting foreground-only mode\n";
        write (1, message, 29);
        fflush(stdout);
        background = 1;
    }
}

int main(void){
    // signal ^C initialization
    struct sigaction saSIGINT = {0};
    saSIGINT.sa_handler = SIG_IGN;
    sigfillset(&saSIGINT.sa_mask);
    saSIGINT.sa_flags = 0;
    sigaction(SIGINT, &saSIGINT, NULL);

    // signal ^Z initialization
    struct sigaction saSIGTSTP = {0};
    saSIGTSTP.sa_handler = sigfunc;
    sigfillset(&saSIGTSTP.sa_mask);
    saSIGTSTP.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &saSIGTSTP, NULL);


    int exitStatus = 0;
    do {
        struct commands command = processLine();
        
        // Set background status
        for (int i = 0; command.cmd[i]; i++) {
            if (!strcmp(command.cmd[i], "&")) {
                if (command.cmd[i + 1] == NULL) {
                    command.background = 1;
                    command.cmd[i] = NULL;
                }
            }   
        }

        // > output file
        for (int i = 0; command.cmd[i]; i++) {
            if (!strcmp(command.cmd[i], ">")) {
                command.outputFile = strdup(command.cmd[i + 1]);
                command.cmd[i+1] = NULL;
                command.cmd[i] = NULL;
            }
        }

        // < input file
        for (int i = 0; command.cmd[i]; i++) {
            if (!strcmp(command.cmd[i], "<")) {
                command.inputFile = strdup(command.cmd[i + 1]);
                command.cmd[i+1] = NULL;
                command.cmd[i] = NULL;
            }
        }

        // comment 
        if (!strcmp(command.cmd[0], "") || command.cmd[0][0] == '#') {
            // printf("comment line\n");
            continue;
        }

        // exit 
        else if (!strcmp(command.cmd[0], "exit")) {
            // printf("this is exit!\n");
            exit(0);
        }

        // cd 
        else if (!strcmp(command.cmd[0], "cd")) {
            // printf("this is a cd! \n");
            if (command.cmd[1]) {
                if (chdir(command.cmd[1]) != 0)
                    perror("chdir() to failed");
            }
            // no arg after cd
            else {
                if (chdir(getenv("HOME")) != 0) 
                    perror("chdir() to /HOME failed");
            }
        }

        // status 
        else if (!strcmp(command.cmd[0], "status")) {
            // printf("this is a status! \n");
            printStatus(exitStatus);
        }

        // cmd goes to exec ()
        else{
            execution(command, &exitStatus, saSIGINT);
        }

    } while (1);

    return 0;
}