#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX 900000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(1); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
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
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

// Validates characters in plaintext and key files
void validate_chars(char *plaintext, char *key) {
  char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int charslen = strlen(chars);
  FILE *fp;
  char ch;

  fp = fopen(plaintext, "r");
  if (fp == NULL) {
    error("CLIENT: File not found\n");
    return;
  }

  while ((ch = fgetc(fp)) != EOF && ch != '\n') {
    int i;
    for (i = 0; i < charslen; ++i) {
      if (ch == chars[i]) {
        break;
      }
    }
    if (i == charslen) {
      fclose(fp);
      error("CLIENT: Invalid character in plaintext\n");
      return;
    }
  }

  fclose(fp);

  fp = fopen(key, "r");
  if (fp == NULL) {
    error("CLIENT: File Not Found\n");
    return;
  }

  while ((ch = fgetc(fp)) != EOF && ch != '\n') {
    int i;
    for (i = 0; i < charslen; ++i) {
      if (ch == chars[i]) {
        break;
      }
    }
    if (i == charslen) {
      fclose(fp);
      error("CLIENT: Invalid character in key\n");
      return;
    }
  }

  fclose(fp);
}

// Creates message with plaintext and key 
void create_msg(char *buffer, const char *plaintext, const char *key) {
  FILE *fp;
  char line[MAX];

  strcpy(buffer, "E\n");

  fp = fopen(plaintext, "r");
  fgets(line, sizeof(line), fp);
  fclose(fp);

  strcat(buffer, line);
  strcat(buffer, "\n");

  memset(line, '\0', sizeof(line));

  fp = fopen(key, "r");
  fgets(line, sizeof(line), fp);
  fclose(fp);

  strcat(buffer, line);
  strcat(buffer, "\n");
  strcat(buffer, "eom\n");
}

int main(int argc, char *argv[]){
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[MAX];
  // Check usage & args
  if (argc < 3) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(1); 
  } 

  // Get file lengths
  int plain_text = open(argv[1], O_RDONLY);
  int plain_length = lseek(plain_text, 0 , SEEK_END);
  int key_text = open(argv[2], O_RDONLY);
  int key_length = lseek(key_text, 0, SEEK_END);

  // Check if key is too short
  if(plain_length > key_length){
    error("Error: Key is too short");
  }

  // Check for bad characters
  validate_chars(argv[1], argv[2]);
  
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

  // Create message with plaintext and key and prepare to send
  create_msg(buffer, argv[1], argv[2]);

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));

  while(!strstr(buffer, "eom\n")) {
    char inbuffer[MAX];
    // Get return message from server
    // Clear out the buffer again for reuse
    memset(inbuffer, '\0', sizeof(inbuffer));

    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, inbuffer, sizeof(inbuffer), 0); 
    if (charsRead < 0){
        error("CLIENT: ERROR reading from socket");
    }
    strcat(buffer, inbuffer);
  }
  // Check if message is from enc
  char *from = strtok(buffer, "\n");
  if(from[0] == 'B'){
    error("Error: Message not from enc\n");
  }
  // Add newline to end of message
  else{
    fprintf(stdout, "%s\n", strtok(NULL, "\n"));
  }

  // Close the socket
  close(socketFD); 
  return 0;
}