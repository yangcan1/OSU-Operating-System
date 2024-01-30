#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_SIZE 200000 

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
    exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

// Learned and modified from github: https://github.com/JetLiTheQT/OneTimePads/blob/main/enc_server.c#L192
void encryption(char* ciphertext, char* plaintext, char* key) {
    int length = strlen(plaintext);
    int i;
    for (i = 0; i < length; i++) {
        ciphertext[i] = '\0';
        int p = (plaintext[i] == ' ') ? 26 : plaintext[i] - 'A'; // Plaintext character as integer (0-25)
        int k = (key[i] == ' ') ? 26 : key[i] - 'A'; // key character as integer (0-25)
        int c;

        c = (p+k)%27;

        ciphertext[i] = (c == 26) ? ' ': c+'A';

    }
    ciphertext[length] = '\0';
}

int main(int argc, char *argv[]){
    int connectionSocket, charsWritten, charsRead;
    char buffer[MAX_SIZE] = {'\0'}, ciphertext[MAX_SIZE] = {'\0'}, plaintext[MAX_SIZE] = {'\0'}, key[MAX_SIZE] = {'\0'}, verifier[10] = {'\0'};
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
    }


    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 
    
    // Accept a connection, blocking if one is not available until one connects
    while(1){
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }



        // FORK
        pid_t pid = fork();
        switch (pid) {
            case 0: // Chinld process

                // Get the message from the client and display it
                memset(buffer, '\0', MAX_SIZE);
                // Read the client's message from the socket
                charsRead = recv(connectionSocket, buffer, MAX_SIZE - 1, 0); 
                if (charsRead < 0){
                    error("ERROR reading from socket");
                }
                // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

                // Check if right client connected
                char *token;
                token = strtok(buffer, ",");
                strcpy(verifier, token);
                token = strtok(NULL, ",");
                strcpy(plaintext, token);
                token = strtok(NULL, ",");
                strcpy(key, token);

                if (strcmp(verifier, "ENC") != 0) {
                    fprintf(stderr, "SERVER: ERROR, client is not enc_client\n");
                    int notEnc = send(connectionSocket, "Client error: dec_client cannot use enc_server\n", 48, 0);
                    close(connectionSocket);
                    exit(2);
                }

                memset(ciphertext,'\0', MAX_SIZE);
                encryption(ciphertext, plaintext, key);
                // fprintf(stderr, "\n-- Ciphertext: %s\n", ciphertext);

                // Send the ciphertext back to the client
                // Learned and modified from github: https://github.com/GrantKop/CS344-OS1/blob/main/kopczeng_program5/assignment5/enc_server.c
                charsWritten = 0;
                int bytesSent = 0;
                int messageLength = strlen(ciphertext);

                while (charsWritten < messageLength) {
                    bytesSent = send(connectionSocket, &ciphertext[charsWritten], strlen(ciphertext), 0); 
                    if (bytesSent < 0) {
                        error("SERVER: ERROR writing to socket");
                        exit(1);
                    }
                    charsWritten += bytesSent;
                }


                memset(plaintext,'\0', MAX_SIZE);
                memset(key,'\0', MAX_SIZE);
                memset(verifier,'\0', 10);

                // Close the connection socket for this client
                close(connectionSocket); 
                exit(0);

            default:; // Parent process
                int childStatus;
                wait(&childStatus);
                close(connectionSocket); 
            
        }

    }

    // Close the listening socket
    close(listenSocket); 
    return 0;
}