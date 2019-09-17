#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


#define LISTENQ 10
#define MAXBUF 512

void encrypt(char* arr, int size, int shift){

  int i;
  int temp;
  for(i=0; i<size; i++){
    if ((((int) arr[i]) <= 122) && (((int) arr[i]) >= 97)){
      temp = ((int) arr[i] - 97 - shift)%26;
      if (temp < 0)
	arr[i] = (char) (122+temp+1);
      else
	arr[i] = (char) (97+temp);
    }
    else if ((((int) arr[i]) <= 90) && (((int) arr[i]) >= 65)){
      temp = ((int) arr[i] - 65 - shift)%26;
      if (temp < 0)
	arr[i] = (char) (90+temp+1);
      else
	arr[i] = (char) (65+temp);
    }
  }
} 

int open_listenfd(int port)  
{ 
  int listenfd, optval=1; 
  struct sockaddr_in serveraddr; 
   
  /* Create a socket descriptor */ 
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; 
  
  /* Eliminates "Address already in use" error from bind. */ 
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  
		 (const void *)&optval , sizeof(int)) < 0) 
    return -1; 
 
  /* Listenfd will be an endpoint for all requests to port 
     on any IP address for this host */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET;  
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  serveraddr.sin_port = htons((unsigned short)port);  
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
 
  /* Make it a listening socket ready to accept 
     connection requests */ 
  if (listen(listenfd, LISTENQ) < 0) 
    return -1; 
 
  return listenfd; 
} 


int main(int argc, char **argv) {

  char okResponse[] = "HTTP/1.0 200 OK\r\n\r\n";
  char notFoundResponse[] = "HTTP/1.0 404 Not Found\r\n\r\n";
  char forbiddenResponse[] = "HTTP/1.0 403 Forbidden\r\n\r\n";
  
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  struct hostent *hp;
  char *haddrp;

  char buf[MAXBUF];

  char *path;//for parsing the get strings
  char *shft;
    
  char *filePath;
  int pathLength;
  int shift;

  FILE *fp;
  char c;

  pid_t childpid;
  
  port = atoi(argv[1]); /* the server listens on a port passed 
			   on the command line */
   
  listenfd = open_listenfd(port);
  
  while(1){
    
    
    clientlen = sizeof(clientaddr); 
    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

    if((childpid = fork()) == 0){
      close(listenfd);

      
      hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
		       sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      haddrp = inet_ntoa(clientaddr.sin_addr);
      
      
      bzero(buf,MAXBUF);
      read(connfd, buf, MAXBUF-1);
  
      path = strstr(buf+4, " ");
      shft = strstr(path, " ");

      *path = '\0';
      *shft = '\0';
  
      shift = atoi(path+1); // shift amount 
      pathLength = strlen(buf+4); // length of the path file
      filePath = malloc(sizeof(char) * pathLength);
      strcpy(filePath, buf+4); // relative path to the file
    
      fp = fopen(filePath, "r");

      if (fp == NULL){
	if (errno == EACCES)
	  write(connfd, forbiddenResponse, strlen(forbiddenResponse)* sizeof(char));
	else
	  write(connfd, notFoundResponse, strlen(notFoundResponse)* sizeof(char));
	
	close(connfd);
	continue;
      }
  
      bzero(buf, MAXBUF);

      while(fread(buf, sizeof(char), MAXBUF-1, fp) == (MAXBUF-1)){
      
	encrypt(buf, MAXBUF-1, shift);
	write(connfd, buf, MAXBUF-1);
	bzero(buf, MAXBUF);
      }

      encrypt(buf, MAXBUF-1, shift);
      write(connfd, buf, MAXBUF-1);

  
      fclose(fp);
      exit(0);
    }
    close(connfd);
  }
}

