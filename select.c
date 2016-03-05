
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#define max(A,B) ((A)>=(B) ? (A):(B))
extern int errno;

int main(){
	int fd, n, ret;
	socklen_t addrlen;
	fd_set rfds;
	enum {idle,busy} state;
	int maxfd, counter;
	struct hostent *h;
	struct in_addr *a;
	struct sockaddr_in addr;
	char buffer[1024];
	char options[128];

	h = gethostbyname("tejo.tecnico.ulisboa.pt");
	a = (struct in_addr*)h->h_addr_list[0];
	fd = socket(AF_INET,SOCK_DGRAM, 0);
	
	if(fd == -1) exit(1);
	memset((void*)&addr, (int)'\0', sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr = *a;
	addr.sin_port = htons(58000);

	while(1){
		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);
		FD_SET(fileno(stdin), &rfds);
		maxfd = max(fd,fileno(stdin));
		
		/*returns the number of file descriptors ready*/
		counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter <= 0) exit(1);
		
		if(FD_ISSET(fileno(stdin), &rfds)){
			fgets(options, 128, stdin);		
				if(strcmp(options, "list\n") == 0){
					n = sendto(fd, "LST\n", 4, 0, (struct sockaddr*)&addr, sizeof(addr));
					printf("n = %d\n", n);
					if(n == -1) exit(1);
					addrlen = sizeof(addr);
					n = recvfrom(fd, buffer, 1024, 0, (struct sockaddr*)&addr, &addrlen);
					printf("n = %d\n", n);
					write(1, buffer, n);//stdout
					printf("\n");
				}else{
					if(strcmp(options, "exit\n") == 0) exit(1);
					else printf("Not an option\nThe options are: \n1)list\n2)exit\n");
					}
				}

		if(FD_ISSET(fd, &rfds)){ /*fd is ready*/

		}
	}

    exit(0);
}
