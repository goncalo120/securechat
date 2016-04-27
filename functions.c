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
#include "functions.h"

/* set colours */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE    "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD_ON    "\x1b[1m"

/*
 * Function: check the command received
 * 
 * Parameters: 3 chars corresponding to the input command 
 * 
 * Return:	0 if it is supposed to register a user
 * 			1 if it is supposed to unregister a user
 * 			2 if it is supposed to do a query either to the surname server or other firstname servers
 * 			-1 otherwise
 * 
 */
int choose_command(char * cmd){
	
	if(strcmp("REG", cmd) == 0) return 0;
	if(strcmp("UNR", cmd) == 0) return 1;
	if(strcmp("QRY", cmd) == 0) return 2;
	return -1;
}

/*
 * Function: get the user inside the list
 * 
 * Parameters: pointer to the list of registered users, the firstname of said user
 * 
 * Return:	the user structure stored in the list
 * 
 */
user get_localization(list * lista, char * firstname){
	
	list * aux;
	user util;
	aux = lista;
	while(aux->prox!=NULL && strcmp((aux->prox)->valor.name, firstname) != 0){
			aux = aux->prox;
	}
	util = (aux->prox)->valor;
	return util;
}

/*
 * Function: get a user	read from the buffer
 * 
 * Parameters: the buffer, the length of the buffer
 * 
 * Return:	both name and surname of the user
 * 
 */
server new_server(char * buffer, int nread){
	char * surname;
	char * ip;
	char * port;
	server valor;
	int i;
	int k;
	
	k = 0;

	surname = malloc(sizeof(char)*20);
	memset(surname,0,20);
	ip = malloc(sizeof(char)*20);
	memset(ip,0,20);
	port = malloc(sizeof(char)*20);
	memset(port,0,20);
	
	for (i=5; buffer[i]!=';';i++){
		surname[k]=buffer[i];
		k = k + 1;
	}
	k=0;
	for(i=i+1;buffer[i]!=';';i++){
		ip[k]=buffer[i];
		k = k + 1;
	}
	k=0;
	for(i=i+1;i<nread;i++){
		port[k]=buffer[i];
		k = k + 1;
	}
	valor.surname = surname;
	valor.ip = ip;
	valor.port = port;

	return valor;
}

/*
 * Function: get the address of the wanted firstname server
 * 
 * Parameters: the ip of the said server, the port of the said server
 * 
 * Return:	the said address
 * 
 */
struct sockaddr_in snp_qry(char * ip, char * port){

	struct sockaddr_in addr_snp;
	int port_i;
	
	memset((void*)&addr_snp, (int)'\0', sizeof(addr_snp));
	
	port_i = atoi(port);
	printf("%d\n",port_i);
	addr_snp.sin_family = AF_INET;
	inet_aton(ip, &addr_snp.sin_addr);
	addr_snp.sin_port = htons(port_i);
	
return addr_snp;
}

/*
 * Function: register a new user in the list (REG)
 * 
 * Parameters: pointer to the list of registered users, the buffer, the length of the buffer, the surname of the firstname server
 * 
 * Return:	the string associated to the action:
 * 			User added - if the user was registered
 * 			NOK User already registered - if the user was already in the list
 * 			NOK IP/PORT BAD FORMAT - if the IP/port were not valid
 * 			NOK Wrong Server - if for some reason the surname do not match the surname associated with the snp
 * 
 */
char* register_new(list * start, char* buffer, int n, char* surname){
	
	user User;
	int a, b, c, d;
	
	User = novo_user(buffer, n);
	
	/* check if the ip is valid -> ex: 111.111.111.111 */
	if(sscanf(User.ip,"%d.%d.%d.%d",&a,&b,&c,&d)!=4){
		return "NOK IP BAD FORMAT";
	}
	/* check if the port is valid -> ex: 1111 */
	if(sscanf(User.port,"%d",&a)!=1){
		return "NOK PORT BAD FORMAT";
	}	
	
	/* check surname */
	if(strcmp(surname, User.surname) == 0){
		if(getuser(start, User.name, User.surname) == 0){
			/* if names are not on the list */
			insere_lista(start, User);
		}else{
			/* if names are alread taken */
			return "NOK User already registered";
		}		
	}else{
		/* surname does not match this snp */
			return "NOK Wrong Server";
	}
	
	write(1, "User added\n", strlen("User added\n"));
	return "OK ";
}

/*
 * Function: delete a registered from the list (UNR)
 * 
 * Parameters: pointer to the list of registered users, the buffer, the length of the buffer, the surname of the firstname server
 * 
 * Return:	the string associated to the action:
 * 			User removed - if the user was deleted
 * 			NOK User doesnt exist - if the user is not found in the list
 * 			NOK Wrong Server - if for some reason the surname do not match the surname associated with the snp
 * 
 */
char* u_unregister(list* start, char *buffer, int n, char* surname){
	
	completename Complete;
	
	Complete = encontra_nomes(buffer, n);

	/* check surname */			
	if(strcmp(surname, Complete.last) == 0){
		if(getuser(start, Complete.first, Complete.last) != 0){
			/* if names are on the list remove the user */
			remove_lista(start, Complete.first, Complete.last);
		}else{
			/* if names are not on the list the user cannot be removed */
			return "NOK User doesnt exist";
		}
	}else{
		/* surname does not match this snp */
		return "NOK Wrong Server";
	}
	write(1, "User removed\n", strlen("User removed\n"));
	return "OK ";
}

/*
 * Function: register the surname associated with the snp on the sa (SREG)
 * 
 * Parameters: the host entity of the surname server, the port of the surname server, the surname of the snp, the IP of the snp 
 *			and the port of the snp 
 * 
 * Return:	0 - if the registration is successful
 * 			-1 - otherwise
 */
int init_sa(struct hostent* h, int surport, char* surname, char* snpip, char* snpport){
	
	int fd,n,addrlen;
	struct sockaddr_in addr;
	char buffer[128];
	struct in_addr *a;
	int send_len;
	char *bufferIn;
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */
	
	/* Register surname in the sa (SREG) */
	send_len = (strlen("SREG ")+strlen(surname)+strlen(snpip)+strlen(snpport)+3);
	bufferIn = (char*)malloc(sizeof(char)*send_len);
	sprintf(bufferIn, "SREG %s;%s;%s\n", surname, snpip, snpport);
	
	a=(struct in_addr*)h->h_addr_list[0];
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd==-1) {
		free(bufferIn);	
		close(fd);
		return -1;
	}

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr=*a;
	addr.sin_port=htons(surport);

	n=sendto(fd,bufferIn,send_len,0,(struct sockaddr*)&addr,sizeof(addr));
	if(n==-1) {
		free(bufferIn);	
		close(fd);
		return -1;
	}
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd, &sock);
	sel = select(fd+1, &sock, 0, 0, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something went wrong" ANSI_COLOR_RESET "\n");
		free(bufferIn);	
		close(fd);
		return -1;
	}
	
	addrlen = sizeof(addr);
	n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,(socklen_t*)&addrlen);
	if(n==-1) {
		free(bufferIn);	
		close(fd);
		return -1;
	}
	
	free(bufferIn);	
	close(fd);
	write(1, "echo_sreg: ", strlen("echo_sreg: "));
	write(1, buffer, n);
	printf("\n");
	return 0;
}

/*
 * Function: delete the surname associated with this snp from the surname server (SUNR)
 * 
 * Parameters: the host entity of the surname server, the port of the surname server and the surname of the snp
 * 
 * Return:	0 - if the unregistration is successful
 * 			-1 - otherwise
 */
int free_sa(struct hostent* h, int aport, char* surname){
	
	int fd,n,addrlen;
	struct sockaddr_in addr;
	char buffer[512];
	struct in_addr *a;
	int send_len;
	char *bufferIn;
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */
	
	a=(struct in_addr*)h->h_addr_list[0];
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd==-1) {
		return -1;
	}

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr=*a;
	addr.sin_port=htons(aport);
	
	send_len = (strlen("SUNR ")+strlen(surname)+1);
	bufferIn = (char*)malloc(sizeof(char)*send_len);
	sprintf(bufferIn, "SUNR %s\n", surname);
	
	n = sendto(fd, bufferIn, strlen(bufferIn), 0, (struct sockaddr*)&addr, sizeof(addr));
	if(n == -1){
		free(bufferIn);	
		close(fd);
		exit(1);
	}	
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd, &sock);
	sel = select(fd+1, &sock, 0, 0, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something went wrong" ANSI_COLOR_RESET "\n");
		free(bufferIn);	
		close(fd);
		return -1;
	}
	
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 512, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);

	free(bufferIn);	
	close(fd);
	write(1, "echo_exit: ", strlen("echo_exit: "));
	write(1, buffer, n);
	printf("\n");

	return 0;
}

