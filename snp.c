/* 
* @Author: goncalo
* @Date:   2016-03-02 10:19:34
* @Last Modified by:   goncalo
* @Last Modified time: 2016-03-02 16:33:09
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv){

	int fd, n, length;
	socklen_t addrlen;
	struct hostent *h;
	struct in_addr *a;
	struct sockaddr_in addr;
	char buffer[1024];
	char options[128];
	char *surname, *ip, *snpport, *bufferIn;


	if(argc < 7) exit(1);
	//if(argc > 11) exit(1);

	surname = (char*)malloc(sizeof(char)*strlen(argv[2]));
	surname = argv[2];

	ip = (char*)malloc(sizeof(char)*strlen(argv[4]));
	ip = argv[4];

	snpport = (char*)malloc(sizeof(char)*strlen(argv[6]));
	snpport = argv[6];

	
	h = gethostbyname("tejo.tecnico.ulisboa.pt");
	a = (struct in_addr*)h->h_addr_list[0];
	fd = socket(AF_INET,SOCK_DGRAM, 0);//UDP socket
	
	if(fd == -1) exit(1);//error
	
	memset((void*)&addr, (int)'\0', sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr = *a;
	addr.sin_port = htons(58000);

	strcpy(bufferIn, "SREG ");
	strcat(bufferIn, surname);
	strcat(bufferIn, ";");
	strcat(bufferIn, ip);
	strcat(bufferIn, ";");
	strcat(bufferIn, snpport);
	strcat(bufferIn, "\n");

	
	n = sendto(fd, bufferIn, strlen(bufferIn), 0, (struct sockaddr*)&addr, sizeof(addr));
	if(n == -1) exit(1);//error

	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, &addrlen);

	write(1, buffer, n);//stdout
	printf("\n");


	while(1){

		fgets(options, 128, stdin);
		
		if(strcmp(options, "list\n") == 0){
			n = sendto(fd, "LST\n", 7, 0, (struct sockaddr*)&addr, sizeof(addr));
			if(n == -1) exit(1);//error

			addrlen = sizeof(addr);
			n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, &addrlen);
			
			write(1, buffer, n);//stdout
			printf("\n");
		}else{
			if(strcmp(options, "exit\n") == 0) break;
			else printf("Not an option\nThe options are: \n1)list\n2)exit\n");
		}

	}

	free(surname);
	free(ip);
	free(snpport);

    exit(0);
}