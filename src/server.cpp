/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "centralinterface.h"
//#include "NearProccess.h"

using namespace std;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int server(int portno)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return -1;//error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        return -2; //error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    if (newsockfd < 0)
        return -3;//error("ERROR on accept");
    bzero(buffer,256);
    n = read(newsockfd,buffer,255);
    if (n < 0) return -4;//error("ERROR reading from socket");
    addmsgtoCentral(decrypt(buffer));
    n = write(newsockfd,"I got your message",18);
    if (n < 0) return -5; //error("ERROR writing to socket");
    //close(newsockfd);
    //close(sockfd);
    return 0;
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