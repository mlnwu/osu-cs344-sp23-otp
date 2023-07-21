#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>

#define MAX 900000

// Error function used for reporting issues
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in *address, int portNumber)
{

  // Clear out the address struct
  memset((char *)address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

// Encrypts message and returns
char *encrypt(char *buffer)
{
  // Allocate memory for cipher_text, plain_text, and key
  char *cipher_text = calloc((MAX), sizeof(char));
  memset(cipher_text, '\0', MAX);
  char *plain_text = calloc((MAX), sizeof(char));
  memset(plain_text, '\0', MAX);
  char *key = calloc((MAX), sizeof(char));
  memset(key, '\0', MAX);

  // Tokenize the buffer
  plain_text = strtok(buffer, "\n"); // Get the first line
  strcat(cipher_text, plain_text);   // Copy the first line to line

  strcat(cipher_text, "\n"); // Add a newline character to line

  plain_text = strtok(NULL, "\n"); // Get the second line
  key = strtok(NULL, "\n");        // Get the third line

  char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int plain_index;
  int key_index;

  // Encrypt the message
  for (int i = 0; i < strlen(plain_text); ++i)
  {
    // Find the index of the current character in plain_text in chars[]
    for (plain_index = 0; plain_text[i] != chars[plain_index]; ++plain_index)
    {
    }

    // Find the index of the current character in key in chars[]
    for (key_index = 0; key[i] != chars[key_index]; ++key_index)
    {
    }

    // Calculate the sum and wrap around if necessary
    int sum = (plain_index + key_index) % 27;

    // Append the encrypted character to cipher text
    strncat(cipher_text, &chars[sum], 1);
  }

  strcat(cipher_text, "\neom\n"); // Add "\neom\n" to cipher text

  return cipher_text; // Return the encrypted message
}

int main(int argc, char *argv[])
{
  int connectionSocket, charsRead;
  char buffer[MAX];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2)
  {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0)
  {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
           (struct sockaddr *)&serverAddress,
           sizeof(serverAddress)) < 0)
  {
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while (1)
  {
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket,
                              (struct sockaddr *)&clientAddress,
                              &sizeOfClientInfo);
    if (connectionSocket < 0)
    {
      error("ERROR on accept");
    }

    pid_t pid;
    char inbuffer[MAX];
    pid = fork();
    switch (pid)
    {
    case -1:
      error("Fork failed\n");
      exit(1);
      break;

    case 0:
      // Get the message from the client and display it
      memset(buffer, '\0', sizeof(buffer));
      memset(inbuffer, '\0', sizeof(inbuffer));

      while (!strstr(buffer, "eom\n"))
      {
        // Read the client's message from the socket
        charsRead = recv(connectionSocket, inbuffer, sizeof(inbuffer), 0);
        if (charsRead < 0)
        {
          error("ERROR reading from socket");
        }
        strcat(buffer, inbuffer);
      }

      if (buffer[0] == 'E')
      {
        strcpy(buffer, encrypt(buffer));
      }
      else
      {
        strcpy(buffer, "B\neom\n");
      }

      int size = strlen(buffer);
      int charsSent = 0;
      while (charsSent < size)
      {
        // Send a Success message back to the client
        charsRead = send(connectionSocket, buffer + charsSent, size - charsSent, 0);
        if (charsRead < 0)
        {
          error("CLIENT: ERROR writing to socket.\n");
        }
        charsSent += charsRead;
      }

      memset(buffer, '\0', sizeof(buffer));
      // Close the connection socket for this client
      close(connectionSocket);
      exit(0);

    default:
      wait(NULL);
      break;
    }
  }
  // Close the listening socket
  close(listenSocket);
  return 0;
}