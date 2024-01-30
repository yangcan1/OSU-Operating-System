#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define MAX_SIZE 200000 

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0],hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[MAX_SIZE] = {'\0'}, ciphertext[MAX_SIZE] = {'\0'}, plaintext[MAX_SIZE] = {'\0'}, key[MAX_SIZE] = {'\0'}, verifier[10] = {'\0'};
    strcpy(verifier, "ENC,");

    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
        exit(0); 
    } 

    // get plaintext from FILE pointer
    FILE *f_plaintext = fopen(argv[1], "r");
    if (f_plaintext == NULL) {
        error("ERROR Failed to open file 1");
    } else {
        fgets(plaintext, MAX_SIZE, f_plaintext);
    }
    fclose(f_plaintext);
    plaintext[strcspn(plaintext, "\n")] = '\0';

    // get keytext from FILE pointer
    FILE *f_key = fopen(argv[2], "r");
    if (f_key == NULL) {
        error("ERROR Failed to open file 2");
    } else {
        fgets(key, MAX_SIZE, f_key);
    }
    fclose(f_key);
    key[strcspn(key, "\n")] = '\0';

    // Complare if the length is same
    int len_plaintext = strlen(plaintext), len_key = strlen(key);
    if (len_plaintext > len_key) {
        error("ERROR key is shorter than plaintext");
    }

    // Make sure plaintext has no bad characters
    int i;
    for (i = 0; i < len_plaintext; i++) {
        if (!((plaintext[i] >= 'A' && plaintext[i] <= 'Z') || plaintext[i] == ' ')) {
            error("ERROR There is bad characters");
        }
    }

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));

    sprintf(buffer, "%s%s,%s", verifier, plaintext, key);
    

    // Send message to server
    // Write to the server
    // Learned and modified from github: https://github.com/GrantKop/CS344-OS1/blob/main/kopczeng_program5/assignment5/enc_server.c
    charsWritten = 0;
    int bytesSent = 0;
    int messageLength = strlen(buffer);
    
    while (charsWritten < messageLength) {
        bytesSent = send(socketFD, buffer, strlen(buffer), 0); 
        if (bytesSent < 0) {
            error("CLIENT: ERROR writing to socket");
            exit(1);
        }
        charsWritten += bytesSent;
    }

    // Get return message from server
    // Clear out the buffer again for reuse

    int bytesReceived = 0;
    charsRead = 0;
    char temp_buffer[MAX_SIZE];
    while (charsRead < strlen(plaintext)) {
        memset(temp_buffer, '\0', sizeof(temp_buffer));
        bytesReceived = recv(socketFD, temp_buffer, 1000, 0);

        charsRead += bytesReceived;  

        if (bytesReceived == 0) {
            // Done reading
            break;
        }

        if (charsRead < 0) {
            error("CLIENT: Error reading from socket");
            break;
        }
        strcat(ciphertext, temp_buffer);
    }

    // print out ciphertext
    ciphertext[strlen(plaintext)] = '\0';
    printf("%s\n", ciphertext);



    // Close the socket
    close(socketFD); 
    return 0;
}