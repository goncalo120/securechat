#define _POSIX_C_SOURCE 1
#define __USE_MISC 
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include "functions.h"

#define max(A,B) ((A)>=(B) ? (A):(B))

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


extern int errno;

/*
 * Function: send query to the surname server (SQRY)
 * 
 * Parameters: the host entity of the surname server, the port of the latter and the surname associated with server which location is wanted to know 
 * 
 * Return:	the surname server reply to the query (SRPL)
 * 
 */
char* do_qry_sa(struct hostent* h, int surport, char* surname) {
	
	int fd, n, addrlen, msg;
	struct sockaddr_in addr;
	struct in_addr *a;
	int send_len;
	char* bufferIn;
	char* mensagem;
	char buffer[512];
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */

	send_len = strlen("SQRY ") + strlen(surname);
	bufferIn = malloc(send_len*sizeof(char) + 1);
	
	a = (struct in_addr*)h->h_addr_list[0];
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd == -1) {
		return "";
	}

	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr=*a;
	addr.sin_port=htons(surport);

	if((msg = sprintf(bufferIn, "SQRY %s", surname)) != (send_len)){
		free(bufferIn);
		close(fd);
		return "";
	}	
	/* Sending it to the surname Server */	
	n = sendto(fd, bufferIn, send_len, 0, (struct sockaddr*)&addr, sizeof(addr));
	if(n == -1) {
		free(bufferIn);
		close(fd);
		return "";
	}
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd, &sock);
	sel = select(fd+1, &sock, NULL, NULL, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something is wrong" ANSI_COLOR_RESET "\n");
		free(bufferIn);	
		close(fd);
		return "";
	}
	
	/* Receiving to check if OK */
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 512, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
	if(n == -1) {
		free(bufferIn);
		close(fd);
		return "";
	}

	mensagem = malloc(n);

	sprintf(mensagem,"%.*s", n, buffer);
	
	free(bufferIn);	
	close(fd);
	return mensagem;
}

/*
 * Function: send query to other firstname servers (QRY)
 * 
 * Parameters: the ip of the firstname server, the port of the latter and the fullname of the user which his location is wanted to know 
 * 
 * Return:	the firstname server reply to the query (RPL)
 * 
 */
char* do_qry_snp(char* snpip, char* snpport, char* firstname, char* surname) {
	
	int fd, n, addrlen;
	struct in_addr* a;
	struct sockaddr_in addr;
	int port;
	int send_len;
	char* bufferIn;
	char buffer[512];
	char* mensagem;
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */

	send_len = strlen("QRY ") + strlen(firstname) + strlen(".") + strlen(surname);
	bufferIn = malloc(send_len*sizeof(char)+1);
	
	if(sprintf(bufferIn, "QRY %s.%s", firstname, surname) != (send_len)){
		free(bufferIn);
		return "";
	}

	a = (struct in_addr *)malloc(sizeof(struct in_addr));
	if (inet_aton(snpip, a) == 0){
        printf("the ip address is wrong\n");
        return "";
    }
    fd = socket(AF_INET,SOCK_DGRAM, 0);
    if(fd == -1) exit(1);
	memset((void*)&addr, (int)'\0', sizeof(addr));

	sscanf(snpport, "%d", &port);
	addr.sin_family = AF_INET;
	addr.sin_addr = *a;
	addr.sin_port = htons(port);

	/* Sending it to the name Server */	
	n=sendto(fd, bufferIn, send_len, 0, (struct sockaddr*)&addr, sizeof(addr));
	if(n==-1) {
		free(bufferIn);
		close(fd);
		return "";
	}
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd, &sock);
	sel = select(fd+1, &sock, NULL, NULL, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something is wrong" ANSI_COLOR_RESET "\n");
		free(bufferIn);	
		close(fd);
		return "";
	}
	
	/* Receiving to check if OK */
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 512, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
	if(n == -1) {
		free(bufferIn);
		close(fd);
		return "";
	}

	mensagem = malloc(n);

	sprintf(mensagem, "%.*s", n, buffer);

	free(bufferIn);	
	close(fd);
	
	return mensagem;
}

/*
 * Function: get the reply from either the surname server or other firstname servers
 * 
 * Parameters: pointer to the list of registered users, the buffer, the length of the buffer, the host entity of the surname server, the port of the latter
 * 			 and the surname which location is wanted to know 
 * 
 * Return:	the reply to the query made by the user of the schat application
 * 
 */
 char* do_qry(list* start, char* buffer, int n, struct hostent *h, int surport, char* surname) {

	char *bufferIn;
	int send_len;
	user User;
	completename Complete;
	server Server;
	
	/* Stored wanted User */
	Complete = encontra_nomes(buffer, n);

	if(strcmp(surname, Complete.last) == 0){
		/* The wanted User is in this snp */
		if(getuser(start, Complete.first, Complete.last) != 0){

			User = get_localization(start, Complete.first);
			/* The +3 is there for 2 semicolon and one dot */
			send_len = (strlen("RPL ")+strlen(User.name)+strlen(User.surname)+strlen(User.ip)+strlen(User.port)+3);
			bufferIn = (char*)malloc(sizeof(char)*send_len);
			sprintf(bufferIn, "RPL %s.%s;%s;%s", User.name, User.surname, User.ip, User.port);

		}else{
			/* User requested has a valid surname for this server but not an existent first name */
			return "NOK User not registered therefore his location is impossible to reach";
		}
	}else{
		bufferIn = do_qry_sa(h, surport, Complete.last);

		if(strcmp(bufferIn,"SRPL") == 0){
			 return "NOK Server not found in the sa";
		}

		Server = new_server(bufferIn, strlen(bufferIn));
		
		bufferIn = do_qry_snp(Server.ip, Server.port, Complete.first, Complete.last);

		if(strcmp(bufferIn,"") == 0){
			 write(1, "User not found\n", strlen("User not found\n"));
			 return "NOK user not found";
		}
		
		/* return the reply*/
		write(1, "User found: ", strlen("User found: "));
		write(1, bufferIn, strlen(bufferIn));
		write(1, "\n", 1);
	}
	return bufferIn;
}



/* main function for snp */
int main(int argc, char** argv){
	int n, port_p, port_a, ret;
	int fd_chat;
	socklen_t addrlen;
	fd_set rfds;
	int maxfd, counter;
	struct hostent *h;
	struct sockaddr_in addr_chat;
	char opt, input[10];
	char buffer[512];
	char options[128];
	char *cmd;
	char *msg;
	char *surname, *snpip, *snpport;
	char *saip, *saport;
	list *start,*temp;
	start = (list *)malloc(sizeof(list)); 
	
	temp = start;
	temp -> prox = NULL;
	temp -> ant = NULL;
	
	surname = NULL;
	snpip = NULL;
	snpport = NULL;
	saip = NULL;
	saport = NULL;
	
	
	/* usage of getopt function which deals with the input (argv)*/
	strcpy(input, "n:s:q:i:p:");
	while((opt = getopt(argc, argv, input))!=-1){
		if(opt == '?'){
			if(optopt == 'n' || optopt == 's' || optopt == 'q' || optopt == 'i' || optopt == 'p'){
				write(1, "Required arg for option -", strlen("Required arg for option -"));
				printf("%c", optopt);
				write(1, "\n", 1);
			}
			else if(isprint(optopt)) {
				write(1,"Unknown option -", strlen("Unknown option -"));
				printf("%c", optopt);
				write(1, "\n", 1);
			}
			else{
				printf("Unknown option character `\\x%x`.\n",optopt);
			}
			return -1;
		}
		if(opt == 'n'){
			surname = optarg;
		}
		if(opt == 's'){
			snpip = optarg;			
		}
		if(opt == 'q'){
			snpport = optarg;			
		}
		if(opt == 'i'){
			saip = optarg;			
		}
		if(opt == 'p'){
			saport = optarg;
		}		
	}

   	/* check if surname, ip and port were given from argv otherwise close */
	if(surname == NULL || snpip == NULL || snpport == NULL){
		exit(1);
	}
	
	/* since it is not mandatory to give the ip of the sa: */ 
	/* we need to check if it was given or if we should use the default (tejo) */
	if(saip == NULL){
		/* connect to the sa, tejo in this case */
		h = gethostbyname("tejo.tecnico.ulisboa.pt");
		if(h == NULL){
			write(1, "Unable to reach sa (tejo)\n", strlen("Unable to reach sa (tejo)\n"));
			exit(1);
		}
	}else{
		/* connect to the sa, other than tejo in this case */
		inet_pton(AF_INET, saip, &temp);
		h = gethostbyaddr(&temp,sizeof(temp),AF_INET);
		if(h == NULL){
			write(1, "Unable to reach sa\n", strlen("Unable to reach sa\n"));
			exit(1);
		}
	}
	
	/* since it is not mandatory to give the port of the sa: */ 
	/* we need to check if it was given or if we should use the default (58000) */
	if(saport == NULL){
		/* no port given */
		port_a = 58000;
	}else{
		/* get port given by input */
		port_a = atoi(saport);
	}

	/* get port given by input for this snp */
	if(sscanf(snpport, "%d\n", &port_p) != 1) {
		write(1, "Not a valid port\n", strlen("Not a valid port\n"));
		exit(1);
	}
	
	/* initializes the socket used to communicate with schats */	
	fd_chat = socket(AF_INET,SOCK_DGRAM, 0);
	if(fd_chat == -1) exit(1);
	
	sscanf(snpport,"%d\n",&port_p);
	memset((void*)&addr_chat, (int)'\0', sizeof(addr_chat));
	addr_chat.sin_family = AF_INET;
	addr_chat.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_chat.sin_port = htons(port_p);

	ret = bind(fd_chat, (struct sockaddr*)&addr_chat, sizeof(addr_chat));
	if(ret == -1) exit(1);
	
	/* register the snp server in the sa serverc*/
	init_sa(h, port_a, surname, snpip, snpport);


	while(1){
		FD_ZERO(&rfds);
		FD_SET(fd_chat,&rfds);
		FD_SET(fileno(stdin), &rfds);
		maxfd = max(fd_chat,fileno(stdin));
		
		/* returns the number of file descriptors ready */
		counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter <= 0) exit(1);
		
		if(FD_ISSET(fileno(stdin), &rfds)){
			fgets(options, 128, stdin);		
			if(strcmp(options, "list\n") == 0){
				/* The typed command is LIST */
				imprime_lista(start->prox);
			}else{
				if(strcmp(options, "exit\n") == 0){
					/* The typed command is EXIT */
					free_sa( h, 58000, surname);
					break;
				}
				else write(1, "Not an option\nThe options are: \n1)list\n2)exit\n", strlen("Not an option\nThe options are: \n1)list\n2)exit\n"));
			}
		}

		if(FD_ISSET(fd_chat, &rfds)){ /*fd_chat is ready*/
			memset(buffer,0,strlen(buffer));
			addrlen = sizeof(addr_chat);
			n = recvfrom(fd_chat, buffer, 128, 0, (struct sockaddr*)&addr_chat, &addrlen);
			if(n == -1) exit(1);
			/* Received command */
			buffer[n] = '\0';
			cmd = (char*) malloc(sizeof(char)*4);
			strncpy(cmd,buffer, 3);
			cmd[3] = '\0';

			if(choose_command(cmd)==0){
				/* The received command is to register a user (REG) */
				msg = register_new(start, buffer, n, surname);
				n = sendto(fd_chat, msg, strlen(msg), 0, (struct sockaddr*)&addr_chat, addrlen);
				if(n==-1) {
					write(1, "NOK message not sent. Error replying!", strlen("NOK message not sent. Error replying!"));
				}
			}
			if(choose_command(cmd) == 1){
				/* The received command is to delete a registered user (UNR) */
				msg = u_unregister(start, buffer, n, surname);
				n = sendto(fd_chat, msg, strlen(msg), 0, (struct sockaddr*)&addr_chat, addrlen);
				if(n==-1) {
					write(1, "NOK message not sent. Error replying!", strlen("NOK message not sent. Error replying!"));
				}
			}
			/* The received command is to query a user (QRY) */
			if(choose_command(cmd) == 2){
				msg = do_qry(start, buffer, n, h, 58000, surname);
				n = sendto(fd_chat, msg, strlen(msg), 0, (struct sockaddr*)&addr_chat, addrlen);		
				if(n==-1) {
					write(1, "NOK message not sent. Error replying!", strlen("NOK message not sent. Error replying!"));
				}
			}
			free(cmd);

		}
		
	}
	close(fd_chat);
	free(start);
    exit(0);
}
