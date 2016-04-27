/* 
* @Author: goncalo
* @Date:   2016-03-07 18:25:17
* @Last Modified by:   goncalo
* @Last Modified time: 2016-04-03 11:28:58
*/

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
#include <time.h>

#include "listas.h"

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


/*
 * Function: print the help menu 
 * 
 * Parameters: no parameters
 * 
 * Return:	no value
 * 
 */
void ask_help(){
	printf(ANSI_COLOR_WHITE "**************************************************\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "WELCOME TO THE HELP MENU" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "\n\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "join: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to be registered on the directory\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "leave: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to be removed from the directory\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "find name.surname: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to search for the user with complete name 'name.surname'\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "connect name.surname keyfile: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to establish a connection with the user with complete name 'name.surname' using a secret key stored in 'keyfile'\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "message string: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to send the message 'string' to the connected user\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "disconnect: " ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "to exit the current connection\n" ANSI_COLOR_RESET);
	printf(ANSI_BOLD_ON "exit: " ANSI_COLOR_RESET);	
	printf(ANSI_COLOR_WHITE "to exit the application\n" ANSI_COLOR_RESET);
	printf(ANSI_COLOR_WHITE "**************************************************\n" ANSI_COLOR_RESET);
	return;
}

/*
 * Function: select the user command 
 * 
 * Parameters: the input command from the apllication
 * 
 * Return:	0 - if the user wants to join in
 * 			1 - if the user wants to leave
 * 			2 - if the user wants to send a QRY to find someone
 * 			3 - if the user wants to connect to someone
 * 			4 - if the user wants to send a message to someone connected to
 * 			5 - if the user wants to disconnect from someone
 * 			6 - if the user wants to exit the application
 * 			7 - if the user wants to check the help menu
 * 
 */
int choose_command(char * cmd){
	
	if(strcmp("join\n", cmd) == 0) return 0;
	if(strcmp("leave\n", cmd) == 0) return 1;
	if(strcmp("find", cmd) == 0) return 2;
	if(strcmp("connect", cmd) == 0) return 3;
	if(strcmp("message", cmd) == 0) return 4;
	if(strcmp("disconnect\n", cmd) == 0) return 5;
	if(strcmp("exit\n", cmd) == 0) return 6;
	if(strcmp("help\n", cmd) == 0) return 7;
	
	return -1;
}

/*
 * Function: create the user structure to whom the user wants to connect
 * 
 * Parameters: the buffer
 * 
 * Return:	the user struct extracted from the buffer
 * 
 */
user new_user(char* buffer){

	user connect;
	char* temp;

	temp = strtok(buffer, ".");

	connect.name = (char *)malloc(sizeof(char)*strlen(temp));
	connect.name = temp;

	temp = strtok(NULL, ";");

	connect.surname = (char *)malloc(sizeof(char)*strlen(temp));
	connect.surname = temp;

	temp = strtok(NULL, ";");

	connect.ip = (char *)malloc(sizeof(char)*strlen(temp));
	connect.ip = temp;

	temp = strtok(NULL, "\n");

	connect.port = (char *)malloc(sizeof(char)*strlen(temp));
	connect.port = temp;

	return connect;

}

/*
 * Function: send query to the snp to find someone (QRY)
 * 
 * Parameters: the string with the user "name.surname", the number of files descriptors and the struct needed to realize the sendto
 * 
 * Return:	the reply from the snp (RPL)
 * 
 */
char* find(char* temp, int fd_snp, struct sockaddr_in addr_snp){

	char* bufferOut, *buffer, a[20], b[20];
	int str_len, n;
	socklen_t addrlen;
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */
	
	/* check if the name is valid -> ex: name.surname */
	if(sscanf(temp,"%[^'.']%s", a, b)!=2){
		return "NOK NAME BAD FORMAT";
	}	

	str_len = strlen("QRY ") + strlen(temp);
	bufferOut = (char *)malloc(sizeof(char)*str_len);
	sprintf(bufferOut, "QRY %s", temp);

	buffer = (char*)malloc(sizeof(char)*128);

	n = sendto(fd_snp, bufferOut, strlen(bufferOut), 0, (struct sockaddr*)&addr_snp, sizeof(addr_snp));
	if(n == -1) exit(1);
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd_snp, &sock);
	sel = select(fd_snp+1, &sock, 0, 0, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something went wrong" ANSI_COLOR_RESET "\n");
		close(fd_snp);
		exit(1);
	}
	
	addrlen = sizeof(addr_snp);
	n = recvfrom(fd_snp, buffer, 128, 0, (struct sockaddr*)&addr_snp, &addrlen);
	buffer[n] = '\0';

	free(bufferOut);

	return buffer;

}

/*
 * Function: retrieve a user
 * 
 * Parameters: the complete name "name.surname", the IP and port of a user
 * 
 * Return:	the said user struct
 * 
 */
user chatUser(char* nameSurname, char* ip, char* port){

	int i = 0;
	int k = 0;
	user User;

	while(nameSurname[i] != '.')
		i++;

	User.name = (char *)malloc(sizeof(char)*(i+1));

	k = i+1;
	i -= 1;

	while(i >= 0){
		User.name[i] = nameSurname[i];
		i -= 1;
	}

	User.surname = (char *)malloc(sizeof(char)*(strlen(nameSurname)-k));

	i = 0;

	while(k < strlen(nameSurname)){
		User.surname[i] = nameSurname[k];
		k++;
		i++;
	}

	User.ip = ip;
	User.port = port;

	return User;

}

/*
 * Function: implement the command leave 
 * 
 * Parameters: the complete name "name.surname", the number of file descriptors, the snp address struct and the flag corresponding to the join state
 * 
 * Return:	no vale
 * 
 */
void leave(char* nameSurname, int fd_snp, struct sockaddr_in addr_snp, int *f_join){

	int n, str_len;
	char* bufferIn;
	char *buffer, *temp;
	socklen_t addrlen;
	int sel;
	fd_set sock;
	struct timeval tv;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */

	buffer = (char *) malloc(sizeof(char)*128);

	str_len = strlen("UNR ") + strlen(nameSurname);
	bufferIn = (char *)malloc(sizeof(char)*str_len);
	sprintf(bufferIn, "UNR %s", nameSurname);

	n = sendto(fd_snp, bufferIn, strlen(bufferIn), 0, (struct sockaddr*)&addr_snp, sizeof(addr_snp));
	if(n == -1) exit(1);
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd_snp, &sock);
	sel = select(fd_snp+1, &sock, 0, 0, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something went wrong" ANSI_COLOR_RESET "\n");
		close(fd_snp);
		exit(1);
	}
	
	addrlen = sizeof(addr_snp);
	n = recvfrom(fd_snp, buffer, 128, 0, (struct sockaddr*)&addr_snp, &addrlen);

	write(1, "rcv_leave = ", strlen("rcv_leave = "));
	write(1, buffer, n);
	printf("\n");

	free(bufferIn);
	
	temp = (char *)malloc(sizeof(char)*3);
	strncpy(temp, buffer, 3);
	
	if(strcmp("OK ", temp) == 0){
		
		(*f_join) = 0;		

	}
	if(strcmp("NOK", temp) == 0){

		printf("NOK %s\n", temp);					
	}

	free(buffer);
	free(temp);

}

/*
 * Function: implement the command join
 * 
 * Parameters: the complete name "name.surname", the tcp IP and port of the user, the number of file descriptors, the snp address struct and the flag corresponding to the join state
 * 
 * Return:	no vale
 * 
 */
void join(char* nameSurname, char* tcpip, char*tcpport, int fd_snp, struct sockaddr_in addr_snp, int *f_join ){

	int n, str_len;
	char* bufferIn;
	char *buffer, *temp,*temp2;
	socklen_t addrlen;
	int sel;
	fd_set sock;
	struct timeval tv;
	user User;
	
	tv.tv_sec = 5;         /* seconds */
	tv.tv_usec = 0;        /* microseconds */
	
	buffer = (char *) malloc(sizeof(char)*128);

	buffer = find(nameSurname, fd_snp, addr_snp);
	temp2 = strtok(buffer," ");

	if(strcmp("RPL", temp2) == 0){

		User = new_user(temp2 = strtok(NULL,"\n"));

		if(strcmp(User.port, tcpport) == 0 && strcmp(User.ip, tcpip) == 0){
			write(1, "rcv_join = OK", strlen("rcv_join = OK"));
			(*f_join) = 1;
			return;
		}

	}

	str_len = strlen("REG ") + strlen(nameSurname)+strlen(tcpport)+strlen(tcpip)+2;
	bufferIn = (char *)malloc(sizeof(char)*str_len);
	sprintf(bufferIn, "REG %s;%s;%s", nameSurname, tcpip, tcpport);

	n = sendto(fd_snp, bufferIn, strlen(bufferIn), 0, (struct sockaddr*)&addr_snp, sizeof(addr_snp));
	if(n == -1) exit(1);
	
	/* Timeout check */
	FD_ZERO(&sock);
	FD_SET(fd_snp, &sock);
	sel = select(fd_snp+1, &sock, 0, 0, &tv);
	if(sel == 0){
		printf(ANSI_COLOR_RED "Time out! Something went wrong" ANSI_COLOR_RESET "\n");
		close(fd_snp);
		exit(1);
	}
	
	addrlen = sizeof(addr_snp);
	n = recvfrom(fd_snp, buffer, 1024, 0, (struct sockaddr*)&addr_snp, &addrlen);

	write(1, "rcv_join = ", strlen("rcv_join = "));
	write(1, buffer, n);
	printf("\n");

	free(bufferIn);

	temp = (char *)malloc(sizeof(char)*3);
	strncpy(temp, buffer, 3);

	if(strcmp("OK ", temp) == 0){

		(*f_join) = 1;

	}

	if(strcmp("NOK", temp) == 0){
		printf(ANSI_COLOR_YELLOW "Impossible to register without a valid IP Address or Port" ANSI_COLOR_RESET "\n");
	}

	free(buffer);
	free(temp);
	
}

/*temp = strtok(NULL, "\n")*/


/*
 * Function: implement the command connect 
 * 
 * Parameters: the string containing the user attributes, the number of file descriptors, the string containing the file name
 * 			and the complete name of the user wanting to connect
 * 
 * Return:	1 - if the authentication was successful 
 * 			0 - otherwise
 * 
 */
int make_connect(char* temp, int* fd_tcp, char* fileName, char* nameSurname){

	struct in_addr *tcp;
	struct sockaddr_in addr_tcp;
	char byte[4], line[4], buffer[128], temp2[128];
	int i, r, j, port_s, n;
	FILE *f;
	user user_c;
	i = 0; r = 0;
	

	user_c = new_user(temp);

	printf(ANSI_COLOR_GREEN "You are connecting to %s.%s.\n", user_c.name, user_c.surname);
	printf(ANSI_COLOR_RESET "Autentication in progress...");
	printf("\n");

	(*fd_tcp) = socket(AF_INET,SOCK_STREAM,0);
	if((*fd_tcp) == -1)exit(1);

	tcp = (struct in_addr *)malloc(sizeof(struct in_addr));
	if (inet_aton(user_c.ip, tcp) == 0){
			printf("the ip address is wrong\n");
		exit(-1);
	}

	sscanf(user_c.port, "%d", &port_s);

	memset((void*)&addr_tcp,(int)'\0',sizeof(addr_tcp));
	addr_tcp.sin_family = AF_INET;
	addr_tcp.sin_addr = *tcp;
	addr_tcp.sin_port = htons(port_s);

	n = connect((*fd_tcp),(struct sockaddr*)&addr_tcp,sizeof(addr_tcp));
	if(n == -1)	exit(1);

	n = read((*fd_tcp), buffer, 128);
	if(n == -1)	exit(1);
	buffer[n] = '\0';
	if(strcmp("BUSY",buffer) == 0 ){
		printf("The other user is %s\n", buffer);
		close((*fd_tcp));
		return 0;
	}

	n = write((*fd_tcp), nameSurname, strlen(nameSurname));
	if(n == -1){
		printf("User is busy!\n");
		close((*fd_tcp));
		return 0;
	}

	/*Select aqui!!*/

	n = read((*fd_tcp), byte, 4);

	sscanf(byte,"%d", &i);

	f = fopen(fileName, "r");
	if(f == NULL){
		printf("NOK There is no keyfile for that user!\n");
		close((*fd_tcp));
		return 0;
		
	}

	j = 0;
	while(j != i){
		fgets(line,5,f);
		line[strlen(line)-1] = '\0';
		j++;
	}

	fgets(line,5,f);
	line[strlen(line)-1] = '\0';	

	fclose(f);

	sleep(1);

	/*Envia a linha do byte*/	
	n = write((*fd_tcp), line, 4);
	if(n == -1) exit(1);

	n = read((*fd_tcp), buffer, 128);
	buffer[n] = '\0';

	sprintf(temp2, "AUTH %d", i);
	printf("%s\n", temp2);

	if(strcmp(temp2, buffer) != 0){
		printf(ANSI_COLOR_RED "Autentication failed! Please check your keyfile for this user.");
		printf(ANSI_COLOR_RESET "\n");
		close((*fd_tcp));
		return 0;
	}

	/*Cria random*/
	r = rand() % 255;
	sprintf(byte, "%d", r);

	/*Envia byte*/
	n = write((*fd_tcp), byte, strlen(byte));
	if(n == -1) exit(1);

	/*Le linha do byte*/
	n = read((*fd_tcp), line, 4);
	if(n == -1) exit(1);

	/*Faz a Autenticação*/

	f = fopen(fileName, "r");
	if(f == NULL){
		printf("NOK There is no keyfile for that user!\n");
		close((*fd_tcp));
		return 0;
		
	}

	j = 0;
	while(j != r){
		fgets(byte,5,f);
		byte[strlen(byte)-1] = '\0';
		j++;
	} 

	fgets(byte,5,f);
	byte[strlen(byte)-1] = '\0';

	fclose(f);

	if(strcmp(byte, line) != 0){
		printf(ANSI_COLOR_RED "Autentication failed! Please check your keyfile for this user.");
		printf(ANSI_COLOR_RESET "\n");
		n = write((*fd_tcp), "NOT AUTH2", strlen("NOT AUTH2"));
		if(n == -1) exit(1);
		close((*fd_tcp));
		return 0;
	}

	sprintf(buffer, "AUTH %d", r);
	printf("%s\n", buffer);

	n = write((*fd_tcp), buffer, strlen(buffer));
	if(n == -1) exit(1);

	printf(ANSI_COLOR_GREEN "Autentication completed! You are now connected");
	printf(ANSI_COLOR_RESET "\n");
	
	return 1;

}

/*
 * Function: implement the response to the request of connection
 * 
 * Parameters:  descriptor of the socket that is waiting for a connection after a listen, address whom we are receiving the connection, the file descriptor
 * 
 * Return:	1 - if the authentication was successful 
 * 			0 - otherwise
 * 
 */
int receive_connect(int* fd_rec, struct sockaddr_in addr_rec, int* fd_tcp){

	socklen_t addrlen;
	char byte[4], line[4], buffer[128], temp[128];
	char* fileName;
	int i, r, j, n;
	FILE *f;
	i = 0; r = 0;

	addrlen = sizeof(addr_rec);
	if(((*fd_tcp) = accept((*fd_rec),(struct sockaddr*)&addr_rec,&addrlen)) == -1)
			exit(1);

	n = write((*fd_tcp), "Are you there?", strlen("Are you there?"));
	if(n == -1) exit(1);

	n = read((*fd_tcp),buffer,128);
	if(n == -1) exit(1);
	buffer[n] = '\0';

	printf(ANSI_COLOR_GREEN "You are receiving a call from %s.\n", buffer);
	printf(ANSI_COLOR_RESET "Autentication in progress...\n");

	fileName = (char*)malloc(sizeof(char)*(strlen(buffer)+5));
	sprintf(fileName, "%s.txt", buffer);

	f = fopen(fileName, "r");

	r = rand() % 255;
	sprintf(byte, "%d", r);

	/* Send byte */
	n = write((*fd_tcp), byte, 4);
	if(n == -1) exit(1);

	/* Receive line of the byte */
	n = read((*fd_tcp),line,4);
	if(n == -1) exit(1);

	/* Authenticates */

	f = fopen(fileName, "r");

	if(f == NULL){
		printf("NOK There is no keyfile for that user!\n");
		close((*fd_tcp));
		return 0;
	}

	j = 0;
	while(j != r){
		fgets(byte,5,f);
		byte[strlen(byte)-1] = '\0';
		j++;
	}

	fgets(byte,5,f);
	byte[strlen(byte)-1] = '\0';

	if(strcmp(byte, line) != 0){
		printf(ANSI_COLOR_RED "Autentication failed! Please check your keyfile for this user.");
		printf(ANSI_COLOR_RESET "\n");
		n = write((*fd_tcp), "NOT AUTH1", strlen("NOT AUTH1"));
		if(n == -1) exit(1);
		close((*fd_tcp));
		return 0;
	}

	sprintf(buffer, "AUTH %d", r);
	printf("%s\n", buffer);
	/* receive byte */

	n = write((*fd_tcp), buffer, strlen(buffer));
	if(n == -1) exit(1);

	n = read((*fd_tcp), byte, 20);
	if(n == -1) exit(1);

	byte[n] = '\0';

	sscanf(byte,"%d", &i);

	f = fopen(fileName, "r");

	if(f == NULL){
		printf("NOK There is no keyfile for that user!\n");
		close((*fd_tcp));
		return 0;
	}

	j = 0;
	while(j != i){
		fgets(line,5,f);
		line[strlen(line)-1] = '\0';
		j++;
	}

	fgets(line,5,f);
	line[strlen(line)-1] = '\0';

	/* Send line */

	n = write((*fd_tcp), line, 4);
	if(n == -1) exit(1);

	n = read((*fd_tcp), buffer, 128);
	buffer[n] = '\0';

	sprintf(temp, "AUTH %d", i);
	printf("%s\n", temp);

	if(strcmp(temp, buffer) != 0){
		printf(ANSI_COLOR_RED "Autentication failed! Please check your keyfile for this user.");
		printf(ANSI_COLOR_RESET "\n");
		close((*fd_tcp));
		return 0;
	}

	printf(ANSI_COLOR_GREEN "Autentication completed! You are now connected");
	printf(ANSI_COLOR_RESET "\n");
	free(fileName);
	return 1;

}


int main(int argc, char** argv) {

	int n, f_join, f_connect, nleft;
	int port_snp, port_tcp;
	int fd_snp, fd_tcp, fd_rec, fd_g;
	char *snpip, *snpport, *nameSurname;
	char* tcpport, *tcpip;
	struct in_addr *snp;
	struct sockaddr_in addr_snp, addr_tcp, addr_rec;
	socklen_t addrlen;
	char garbage;
	char opt, input[10];
	char buffer[128];
	char *ptr, *fileName;
	char *temp, *temp2, *cmd;
	fd_set rfds;
	int maxfd, counter;
	user User;
	
	
	void (*old_handler)(int);/*interrupt handler*/

	if((old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR) exit(1);/*error*/
	
	nameSurname = NULL;
	tcpip = NULL;
	tcpport = NULL;
	snpip = NULL;
	snpport = NULL;
	
	/* usage of getopt function which deals with the input (argv)*/
	strcpy(input, "n:i:p:s:q:");
	while((opt = getopt(argc, argv, input))!=-1){
		if(opt == '?'){
			if(optopt == 'n' || optopt == 'i' || optopt == 'p' || optopt == 's' || optopt == 'q'){
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
			nameSurname = optarg;
		}
		if(opt == 'i'){
			tcpip = optarg;			
		}
		if(opt == 'p'){
			tcpport = optarg;			
		}
		if(opt == 's'){
			snpip = optarg;			
		}
		if(opt == 'q'){
			snpport = optarg;
		}		
	}
	
	/* check if name.surname, ip, scport, snpip and snpport were given from argv otherwise close */
	if(nameSurname == NULL || tcpip == NULL || tcpport == NULL|| snpip == NULL || snpport == NULL){
		exit(1);
	}

	User = chatUser(nameSurname, tcpip, tcpport);

	printf("name = %s\n", User.name);
	printf("surname = %s\n", User.surname);

	/*SNP*/
	snp = (struct in_addr *)malloc(sizeof(struct in_addr));
	if (inet_aton(snpip, snp) == 0){
        printf("the ip address is wrong\n");
        return -1;
    }
    fd_snp = socket(AF_INET,SOCK_DGRAM, 0);
    if(fd_snp == -1) exit(1);
	memset((void*)&addr_snp, (int)'\0', sizeof(addr_snp));

	sscanf(snpport, "%d", &port_snp);
	addr_snp.sin_family = AF_INET;
	addr_snp.sin_addr = *snp;
	addr_snp.sin_port = htons(port_snp);
	
	
	if(sscanf(tcpport, "%d%c", &port_tcp, &garbage)==2){
		printf(ANSI_COLOR_YELLOW "Impossible to enter without a valid Port" ANSI_COLOR_RESET "\n");
	}

	/*TCP socket*/	
	sscanf(tcpport, "%d", &port_tcp);


	if((fd_rec = socket(AF_INET,SOCK_STREAM,0))==-1)exit(1);

	memset((void*)&addr_tcp,(int)'\0',sizeof(addr_rec));
	addr_rec.sin_family=AF_INET;
	addr_rec.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_rec.sin_port=htons(port_tcp);

	if(bind(fd_rec,(struct sockaddr*)&addr_rec,sizeof(addr_rec))==-1)
		exit(1);

	f_join = 0;
	f_connect = 0;

	srand(time(NULL));

	if(listen(fd_rec,5) == -1) exit(1);
	
	printf(ANSI_COLOR_MAGENTA "**********************\n*WELCOME TO THE SCHAT*\n**********************" ANSI_COLOR_RESET "\n");
	printf(ANSI_COLOR_GREEN "Type 'help' to show the help menu" ANSI_COLOR_RESET "\n");

	while(1){

		/*select*/
		FD_ZERO(&rfds);
		FD_SET(fileno(stdin), &rfds);
		FD_SET(fd_rec, &rfds);
		if(f_connect == 1){
			FD_SET(fd_tcp, &rfds);
		}
		

		maxfd = max(fileno(stdin), max(fd_tcp,fd_rec));
		
		/*returns the number of file descriptors ready*/
		counter = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter <= 0) exit(1);	

		if(FD_ISSET(fileno(stdin), &rfds)){

			fgets(buffer, 128, stdin);

			cmd = strtok(buffer, " ");
			
			/*join*/
			if(choose_command(cmd) == 0){

				if(f_join != 1 && f_connect != 1){

					join(nameSurname, tcpip, tcpport, fd_snp, addr_snp, &f_join);

				}else{
					if(f_connect == 1){
						printf("NOK You're already connected\n");
					}
					if(f_join == 1){
						printf("NOK You're already joined in\n");
					}
				}

			}else{

			/*leave*/
				if(choose_command(cmd) == 1){
					
					if(f_join == 1 && f_connect != 1){

						leave(nameSurname, fd_snp, addr_snp, &f_join);

					}else{

						if(f_connect == 1){
							printf("NOK You're still connected to another user. Please disconnect\n");
						}
						if(f_join != 1){
							printf("NOK You have to join first\n");
						}

					}
				}else{

					/*find*/
					if(choose_command(cmd) == 2){

						if(f_join == 1){

							temp = strtok(NULL, "\n");

							if(temp == NULL){
								printf("NOK Please write the name of the person you are looking for!\n");
								continue;
							}

							temp = find(temp, fd_snp, addr_snp);
							write(1, "rcv_find = ", strlen("rcv_find = "));
							write(1, temp, strlen(temp));
							printf("\n");
							

						}else{
							printf("NOK You have to join in first\n");
						}


					}else{

						/*connect*/
						if(choose_command(cmd) == 3){

							temp = strtok(NULL, " ");
							if(strcmp(nameSurname, temp) == 0){
								printf("NOK You can't connect to yourself!\n");
								continue;
							}

							temp2 = strtok(NULL, "\n");
							if(temp2 == NULL){
								printf("NOK please insert a name for a valid keyfile!\n");
								continue;
							}
							
							strcat(temp2, ".txt");

							fileName = (char*)malloc(sizeof(char)*(strlen(temp)+5));
							sprintf(fileName, "%s.txt", temp);

							temp = find(temp, fd_snp, addr_snp);

							if(strcmp(fileName, temp2) != 0){
								printf("NOK The keyfile must be named after the person you want to connect.\n");
								free(fileName);
								continue;
							}
							
							temp = strtok(temp, " ");

							if(strcmp(temp, "RPL") == 0){

								f_connect = make_connect(temp = strtok(NULL, "\n"), &fd_tcp, fileName, nameSurname);
								free(fileName);
								continue;

							}else{
								free(fileName);
								printf("NOK %s\n", temp = strtok(NULL, "\n"));
							}

						}else{

							/*message*/
							if(choose_command(cmd) == 4){

								if(f_connect == 1 && f_join == 1){
									temp = strtok(NULL, "\n");
									nleft = strlen(temp);
									
									if(nleft > 128){
										printf("NOK please write a shorter message\n");
										continue;
									}

									ptr = &temp[0];

									while(nleft > 0){
										n = write(fd_tcp,ptr,nleft);
										if(n <= 0) exit(1);
										nleft -= n;
										ptr += n;
									}
								}else{
									if(f_connect != 1){
										printf("NOK You're not connected\n");
									}
									printf("f_join1 = %d\n", f_join);
									if(f_join != 1){
										printf("NOK You're not joined in\n");
									}
								}

							}else{

							/*disconnect*/
								if(choose_command(cmd) == 5){

									if(f_connect == 1 && f_join == 1){

										printf("You disconnected\n");
										close(fd_tcp);
										f_connect = 0;

									}else{
										if(f_connect != 1){
											printf("NOK You're not connected\n");
										}
										if(f_join != 1){
											printf("NOK You're not joined in\n");
										}
									}
									
								}else{

								/*exit*/
									if(choose_command(cmd) == 6){

										if(f_join == 1 && f_connect != 1){

											leave(nameSurname, fd_snp, addr_snp, &f_join);
											break;
										}
										if(f_connect == 1 && f_join == 1){
											printf("NOK You're still connected to a user. Please disconnect first\n");
											continue;
										}
										break;

									}else{
									/*help*/
										if(choose_command(cmd) == 7){
											ask_help();

										}else{

											printf("Incorrect command! Write help for help!\n");
										}

									}
								}
							}
						}
					}
				}
			}

		}

		/*receives tcp connection*/
		if(FD_ISSET(fd_rec, &rfds)){

			if(f_connect == 0 && f_join == 1){

				f_connect = receive_connect(&fd_rec, addr_rec, &fd_tcp);
				continue;

 			}else{
					
				if(f_connect != 0 && f_join == 1){
					printf("You're receiving another call! The other user will call later\n");
					addrlen = sizeof(addr_rec);
					if((fd_g = accept(fd_rec,(struct sockaddr*)&addr_rec,&addrlen)) == -1)
						exit(1);
					n = write(fd_g, "BUSY", strlen("BUSY"));
					if(n == -1) exit(1);
					close(fd_g);

				}
				if(f_join != 1){
					printf("NOK You need to join in to connect.\n");
				}
			}


		}

		/*already connected, receives message*/
		if(f_connect == 1){

			if(FD_ISSET(fd_tcp, &rfds)){

				n = read(fd_tcp,buffer,128);
				if(n == -1) exit(1);

				if(n == 0){
					printf("The other user disconnected\n");
					close(fd_tcp);
					f_connect = 0;

				}else{

					write(1, "message: ", strlen("message: "));

					write(1, buffer, n);

					printf("\n");

				}
	
			}
		}

	}

	exit(0);
}
