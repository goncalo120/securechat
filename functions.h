#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include "listas.h"

/* Server Structure*/
typedef struct server{
	char* surname;
	char* ip;
	char* port;
}server;

int choose_command(char * buffer);
user get_localization(list * lista, char * firstname);
server new_server(char * buffer, int nread);
struct sockaddr_in snp_qry(char * ip, char * port);
char* register_new(list * start, char* buffer, int n, char* surname);
char* u_unregister(list* start, char *buffer, int n, char* surname);
int init_sa(struct hostent* h, int surport, char* surname, char* snpip, char* snpport);
int free_sa(struct hostent* h, int aport, char* surname);

