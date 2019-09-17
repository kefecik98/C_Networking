#include <stdio.h>
#include <netdb.h>
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <stdlib.h>

#define MAXBUF 4097

int Open_clientfd(char *hostname, int port){

	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if ((hp = gethostbyname(hostname)) == NULL)
		return -2;

	bzero((char*) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr, (char *)&serveraddr.sin_addr, hp->h_length);
	serveraddr.sin_port = htons(port);

	if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	
	return clientfd;

	
}



int main (int argc, char ** argv){
	int clientfd, port;
	char *host, *filePath, buf[MAXBUF];

	char getStart[] = "GET ";
	char getEnd[] = " HTTP/1.0\r\n\r\n";
	char *requestString;
	int requestStringSize;
	char responseMask[] = "\r\n\r\n";
	char *p;

	char checkResponse[] = "HTTP/1.1 200 OK";
	
	host = argv[1];
	port = atoi(argv[2]);
	filePath = argv[3];

	requestStringSize = strlen(getStart) + strlen(getEnd) + strlen(filePath);
	requestString = malloc(sizeof(char) * requestStringSize);

	strcpy(requestString, getStart);
	strcat(requestString, filePath);
	strcat(requestString, getEnd);

	clientfd = Open_clientfd(host, port);

	if (clientfd < 0){
		printf("Error opening connection \n");
		return EXIT_FAILURE;
	}
	bzero(buf,MAXBUF);
	write(clientfd, requestString, requestStringSize);
	read(clientfd, buf, MAXBUF-1);

	p = strstr(buf, checkResponse);
	if (p == NULL)
	  return EXIT_FAILURE;
	
	p = strstr(buf, responseMask);
	
	fputs(buf, stdout);
	
	close(clientfd);
	free(requestString);
	////////////////////////////////////////////////////////////////////

	requestStringSize = strlen(getStart) + strlen(getEnd) + strlen(p+4);
	requestString = malloc(sizeof(char) * requestStringSize);

	int temp = strlen(p+4);
	*(p+4+temp-1) = '\0'; //getting rid of the \n at the end of the path from response
	strcpy(requestString, getStart);
	strcat(requestString, p+4);
	strcat(requestString, getEnd);
	
	clientfd = Open_clientfd(host, port);

	if (clientfd < 0){
		printf("Error opening connection \n");
		return EXIT_FAILURE;
	}
	bzero(buf,MAXBUF);
	write(clientfd, requestString, requestStringSize);
	read(clientfd, buf, MAXBUF-1);
	fputs(buf, stdout);
	bzero(buf,MAXBUF);

	while(read(clientfd, buf, MAXBUF-1) > 0){
	  fputs(buf,stdout);
	  bzero(buf,MAXBUF);
	}
	free(requestString);
      
	close(clientfd);
	
	return EXIT_SUCCESS;

}
