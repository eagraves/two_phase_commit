/*
 *  server.c
 *
 *  Modified by Evan Graves on 5/23/13
 *  
 *  Two Phase Commit Program
 *
 */

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Socket-related libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_LINE_SIZE 50
#define LISTEN_QUEUE 5
#define NUM_THREADS 2  //Increase this number to allow more connections

int success;
int num_clients = 0;
int ready_clients = 0;

void print_usage(){
  fprintf(stderr, "Usage: ./server [port]\n");
  fprintf(stderr, "Example: ./server 4845\n\n");
}
 
void check_args(int argc, char *argv[]){
  if( argc != 2 ) {
    print_usage();
	exit(1);
  }
}


// Thread function
void *serverFunction(void* args){
 printf("A client connected\n");
	int recvFD;
	// Parse the arguments:
	recvFD = (int) args;
	// Send a connection ack to client and start session

	num_clients++;
	if(num_clients < 2)
		 printf("Waiting for second client\n");
	while(num_clients < 2)
	{}


	while(1){
	// Send a Vote Request
	// If ’exit’ command, break the while loop

   	char line[MAX_LINE_SIZE];   //Received String
   	char message[MAX_LINE_SIZE];   //message String
	int offset = 0; //Desired offset from client
	int len;
  	char vote[] = "Vote";
	char abort[] = "Abort";
	char commit[] = "Commit";

	while (1){
  	printf("Press \"V\" when ready to vote \n");
  	fgets(line, MAX_LINE_SIZE + 1, stdin);
	if(line[0] == 'V') break;
	printf("Improper format \n");
	}


  	len = strlen(vote);
  	success = send(recvFD, vote, len, 0);
	//Error checking
	if (success == -1) {perror("Failed to send data \n"); exit(1);}



    	// Receive data
	//If TCP, use recv() function
	int num_bytes_received = 0;
	num_bytes_received = recv(recvFD, line, MAX_LINE_SIZE-1, 0);
	if(num_bytes_received == -1) {perror("Failed to properly receive data \n"); exit(1);}

	
	//Check the client's response
	if(line[0] == 'Y')
	{
		ready_clients++;
	}

	if(line[0] == 'N')
	{
	ready_clients = -10;
	//Send the decision back to the client
  	len = strlen(abort);
  	success = send(recvFD, abort, len, 0);
	//Error checking
//	if (success == -1) {perror("Failed to send data"); exit(1);}

   	 // Close the socket associated to this client
  	printf("A client disconnected\n");
	num_clients--;
    	success = close(recvFD);
	    if(success == -1) {perror("Failed to close socket"); exit(1);}
	pthread_exit(NULL);
	}



while (ready_clients < 2)
	{
	if(ready_clients < -5 )
		{
		//Send the decision back to the client
  		len = strlen(abort);
  		success = send(recvFD, abort, len, 0);
		//Error checking
		if (success == -1) {perror("Failed to send data"); exit(1);}

		// Close the socket associated to this client
  		printf("A client disconnected\n");
		num_clients--;
    		success = close(recvFD);
	    	if(success == -1) {perror("Failed to close socket"); exit(1);}
		pthread_exit(NULL);
		}	
	}

	//Send the decision back to the client
	//If this point has been reached, it is a commit
  	len = strlen(commit);
  	success = send(recvFD, commit, len, 0);
	//Error checking
	if (success == -1) {perror("Failed to send data"); exit(1);}
  
	}
   	 // Close the socket associated to this client
  printf("A client disconnected\n");
	num_clients--;
    	success = close(recvFD);
	    if(success == -1) {perror("Failed to close socket"); exit(1);}
	pthread_exit(NULL);
}



/////////////////////
// MAIN /////////////
int main(int argc, char *argv[])
{
  int created_threads = 0;
  char space[] = " ";
  int success; 
  struct addrinfo hints;
  struct addrinfo *serverInfo;
  
  // Check arguments and exit if they are not well constructed
  check_args(argc, argv);
  

  // Fill the 'hints' structure
  memset( &hints, 0, sizeof(hints) );
  hints.ai_flags    = AI_PASSIVE;  // Use the local IP address
  hints.ai_family   = AF_INET;     // Use IPv4 address
  hints.ai_socktype = SOCK_STREAM; // TCP sockets


  // Get the address information of the server ('NULL' uses the local address )
  getaddrinfo( NULL , argv[1], &hints, &serverInfo );
  
  // Make a socket (identified with a socket file descriptor)
  int socketFD = socket( serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol );
	//Error checking
	if (socketFD == -1) {perror("Failed to create socket"); exit(1);}

  // Bind to the specified port (in getaddrinfo())
  success = bind( socketFD, serverInfo->ai_addr, serverInfo->ai_addrlen );
	//Error checking
	if (success == -1) {perror("Failed to bind to port"); exit(1);}
  
  // This is only used to be able to reuse the same port
  int yes = 1;
  setsockopt( socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) );
  

	//Create an array of threads
  pthread_t threads[NUM_THREADS];
  // Start to listen (only for TCP)
  success = listen(socketFD, LISTEN_QUEUE);
	//Error checking
	if (success == -1) {perror("Falied to begin listening"); exit(1);}


  printf("Listening in port %s... \n", argv[1]);
  fflush(stdout);
  
  // Free the address info struct
  freeaddrinfo(serverInfo);
  

  int recvFD;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(socketFD, &fds);
  while (1) {
	//If there is nothing in the listen queue, sleep
	int numOfFD = select(socketFD +1, &fds, NULL, NULL, 0);
   	if( select(FD_SETSIZE, &fds, NULL, NULL, 0) < 1)
	{
		sleep(1);
		continue;
	}

  // Accept a connection (only for TCP) and create a socket for each one
  	struct sockaddr_storage clientAddr;
  	socklen_t clientAddr_size = sizeof(clientAddr);

  	recvFD = accept(socketFD, (struct sockaddr *)&clientAddr, &clientAddr_size);
	if(recvFD == -1 ) {perror("Failed to accept a connection"); exit(1);}

pthread_create( &threads[created_threads % NUM_THREADS], NULL, serverFunction, (void *)recvFD );
created_threads++;
  }

  // Close the socket that was used to listen (TCP only)
  success = close(socketFD);
	if(success == -1) {perror("Failed to close socket"); exit(1);}
  
  return EXIT_SUCCESS;
}
