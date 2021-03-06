/*
 *  client.c
 *
 *  Modified by Evan Graves on 3/3/13
 * 
 *  Client Program
 *  Two phase commit program
 *
 */


// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Socket-related libraries
#include <sys/types.h>
#include <sys/socket.h> 
#include <netdb.h>

#define MAX_LINE_SIZE 50

void print_usage(){
  fprintf(stderr, "Usage: ./client [hostname] [port]\n");
  fprintf(stderr, "Example: ./client 192.168.0.2 4845\n\n");
}
 
void check_args(int argc, char *argv[]){
  if( argc != 3 ) {
    print_usage();
	exit(1);
  }
}

int main(int argc, char *argv[])
{

  int success; //For error checking
  struct addrinfo hints;
  struct addrinfo *serverInfo;
  
  // Check arguments and exit if they are not well constructed
  check_args(argc, argv);
  
  // Fill the 'hints' structure
  memset( &hints, 0, sizeof(hints) );
  hints.ai_family   = AF_INET;     // Use IPv4 address
  hints.ai_socktype = SOCK_STREAM; // TCP sockets

  
  // Get the address information of the server.
  // argv[1] is the address, argv[2] is the port
  getaddrinfo(argv[1], argv[2], &hints, &serverInfo);
  
  // Create a socket to communicate with the server
  int socketFD = socket( serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol );
	//Error checking
	if (socketFD == -1) {perror("Failed to create socket\n"); exit(1);}
  

  // Connect to the server (only for TCP)
printf("Waiting for coordinator request...\n");
   success = connect(socketFD, serverInfo->ai_addr, serverInfo->ai_addrlen);
	//Error checking
	if (success == -1) {perror("Failed to connect to server\n"); exit(1);}

	//Instruct how to quit
 // printf("Note: to quit client, type \"quit\" with no numbers\n");
  while(1){
  // Ask for data to send
  char line[MAX_LINE_SIZE];
  char text[MAX_LINE_SIZE];
  char quit[] = "quit";
  int encrypVal = -1;
  int arguments = 0;
  int len;
  char yes[] = "Y";
  char no[] = "N";

 //printf("Waiting for coordinator request...\n");
//RECEIVE DATA FROM SERVER HERE
	int num_bytes_received = 0;
  	char processed[MAX_LINE_SIZE];
	num_bytes_received = recv(socketFD, processed, MAX_LINE_SIZE-1, 0);
	if(num_bytes_received == -1) {perror("Failed to properly receive data\n"); exit(1);}


	while (1){
  	printf("Vote Initiated. Commit? \"Yes\" or \"No\" \n");
  	fgets(line, MAX_LINE_SIZE + 1, stdin);
  	len = strlen(line);
	if(line[0] == 'Y' || line[0] == 'y' || line[0] == 'N' || line[0] == 'n') 	break;
	printf("Improper format\n");
	}


  // Send the users decision:
  printf("Sending response to '%s' using port '%s'... \n", argv[1], argv[2]);

if(line[0] == 'Y' || line[0] == 'y')
  	{
	success = send(socketFD, yes, len, 0); // TCP style.
	}

if(line[0] == 'N' || line[0] == 'n')
  	{
	success = send(socketFD, no, len, 0); // TCP style.
	printf("No commitement made. Exiting.\n");
	}
	//Error checking
	if (success == -1) {perror("Failed to send data"); exit(1);}
 

//Wait for coordinator response
	num_bytes_received = recv(socketFD, processed, MAX_LINE_SIZE-1, 0);
	if(num_bytes_received == -1) {perror("Failed to properly receive data\n"); exit(1);}

//And react
	if(processed[0] == 'C')
	printf("Commit Received!\n");

	else
	printf("Abort Received!\n");
break;

} //End of while loop to accpet data


  // Free the address info struct and close the socket
  freeaddrinfo(serverInfo);
  success = close(socketFD);
	//Error checking
	if (success == -1) {perror("Failed to close socketFD\n"); exit(1);}
  
  return EXIT_SUCCESS;
}
