#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int client(const char* host, int portno, char buffer[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;



    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -2; //error("ERROR opening socket");
    server = gethostbyname(host);
    if (server == NULL) {
        //fprintf(stderr,"ERROR, no such host\n");
        return  -3;// exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
       return -4;// error("ERROR connecting");


    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,sizeof(buffer));
    n = read(sockfd,buffer,sizeof(buffer));
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}