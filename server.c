#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define RCVSIZE 1024

int main (int argc, char *argv[]) {

  int port;

  fd_set initial_fds;
	FD_ZERO(&initial_fds);

  if (argc < 2){
    perror("Too few arguments given. Format : ./server port");
    return -1;
  }
  else if (argc > 2){
    perror("Too many arguments given. Format : ./server port");
    return -1;
  }
  else {
    port = atoi(argv[1]);
  }

  struct sockaddr_in adresse, client;
  int valid= 1;
  socklen_t alen= sizeof(client);
  char buffer[RCVSIZE];

  //create socket
  int server_desc = socket(AF_INET, SOCK_STREAM, 0);

  //handle error
  if (server_desc < 0) {
    perror("Cannot create socket\n");
    return -1;
  }

  //create select socket
  int select_desc = socket(AF_INET, SOCK_DGRAM, 0);

  //handle error
  if (select_desc < 0){
    perror("Cannot create UDP socket");
    return -1;
  }

  setsockopt(server_desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse.sin_family= AF_INET;
  adresse.sin_port= htons(port);
  adresse.sin_addr.s_addr= htonl(INADDR_ANY);
  pid_t child_pid;

  //initialize sockets
  if (bind(server_desc, (struct sockaddr*) &adresse, sizeof(adresse)) == -1) {
    perror("Bind failed\n");
    close(server_desc);
    return -1;
  }


  //listen to incoming clients
  if (listen(server_desc, 0) < 0) {
    printf("Listen failed\n");
    return -1;
  }

  printf("Listen done\n");

  FD_SET(STDIN_FILENO,&initial_fds);
  FD_SET(server_desc,&initial_fds);

  while (1) {

    printf("Waiting...\n");
    if (select(server_desc+1,&server_desc,NULL,NULL,NULL)<0){
      perror("select issue");
      return -1;
    }


    printf("Accepting\n");
    int client_desc = accept(server_desc, (struct sockaddr*)&client, &alen);
    child_pid = fork();
    printf("PID : %d\n",child_pid);
    printf("Value of accept is:%d\n", client_desc);

    if (child_pid == 0){
      close(server_desc);
      printf("Socket creation : %d\n",server_desc);
      printf("Socket accept : %d\n",client_desc);

      while(1){
        int msgSize = read(client_desc,buffer,RCVSIZE);
        //buffer[strcspn(buffer, "\r\n")] = 0;

        while (msgSize > 0) {

          if (strcmp(buffer, ":quit")==0){
              printf("//end of connection");
              break;
          }
          else{
            write(client_desc,buffer,msgSize);
            printf("%s",buffer);
            memset(buffer,0,RCVSIZE);
            msgSize = read(client_desc,buffer,RCVSIZE);
            //buffer[strcspn(buffer, "\r\n")] = 0;
          }
        }  
      }
    }
    else{
    close(client_desc);
    }
  }

close(server_desc);
return 0;
}
