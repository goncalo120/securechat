#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "listas.h"

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
 * Function: alocates a list composed of two structures (previous and next) initialized at NULL
 * 
 * Parameters: no parameters
 * 
 * Return:	the list
 * 
 */
list * AllocatesList(){
	
        list *start,*temp;
        start = (list *)malloc(sizeof(list)); 
        temp = start;
        temp -> prox = NULL;
        temp -> ant = NULL;
return start;

}

/*
 * Function: prints a given user
 * 
 * Parameters: the user
 * 
 * Return:	no value
 * 
 */
void print_user(user valor){
	printf("--------\n");
	printf(ANSI_BOLD_ON "\x1b[1m" "Name:" ANSI_COLOR_RESET);
    printf("%s\n", valor.name);
	printf(ANSI_BOLD_ON "Surname:" ANSI_COLOR_RESET);
    printf("%s\n", valor.surname);
    printf(ANSI_BOLD_ON "IP:" ANSI_COLOR_RESET);
    printf("%s\n", valor.ip);
    printf(ANSI_BOLD_ON "TCP port:" ANSI_COLOR_RESET);
    printf("%s\n", valor.port);  
    printf("\n");
}

/*
 * Function: print users and therefore printing the list of users
 * 
 * Parameters: the list of users
 * 
 * Return:	no value
 * 
 */
void imprime_lista(list * lista){

        list * aux;
        aux = lista;
		if(aux==NULL){
			printf(ANSI_COLOR_YELLOW "The List of Users is Empty" ANSI_COLOR_RESET "\n");
			return;
        }
        
        if(lista==NULL){
			return;
        }
        while(lista!=NULL){
			print_user(lista->valor);
			lista = lista->prox;
		}

}

/*
 * Function: insert a given user in a list
 * 
 * Parameters: the list of users and the user 
 * 
 * Return:	no value
 * 
 */
void insere_lista(list *lista , user n){
 
	while(lista->prox!=NULL){
		lista = lista -> prox;
	}
        
	/* Allocate memory for the new node and put data in it.*/
	lista->prox = (list *)malloc(sizeof(list));
	(lista->prox)->ant = lista;
	lista = lista->prox;
	lista->valor = n;
	lista->prox = NULL;
}

/*
 * Function: remove a user from the list given his name
 * 
 * Parameters: the pointer to the list of users and the user firstname and surname 
 * 
 * Return:	no value
 * 
 */
void remove_lista(list *lista , char * name, char * surname){
 
	list * temp;
	
	while(lista->prox!=NULL && strcmp((lista->prox)->valor.name, name) != 0){
			lista = lista->prox;
	}
	if (lista->prox==NULL){
		return;
	}
	temp=lista->prox;
	lista->prox=temp->prox;
	temp->ant=lista;
	free(temp);
	return;
}

/*
 * Function: return a user knowing his name 
 * 
 * Parameters: the pointer to the list of registered users and the user firstname and surname 
 * 
 * Return:	0 - if the user is not found in the list
 * 			1 - if the user is found
 * 
 */
int getuser(list * lista, char * nome, char * surname){

		list * aux;
        aux =  lista-> prox; /* First node is dummy node */
        
        /* Iterate through the list and search for the name */
        while(aux!=NULL){
			if(strcmp(aux->valor.name, nome) == 0 && strcmp(aux->valor.surname, surname) == 0){
				return 1;
			}
                aux = aux -> prox;/* Search in the next node */
        }
        /* Name is not found */
        return 0;
}

/*
 * Function: return a name read from the buffer
 * 
 * Parameters: the buffer and its length
 * 
 * Return:	the complete name of the user written in the buffer
 * 
 */
completename encontra_nomes(char * buffer, int nread){

	char* temp;
	completename complete;

	temp = strtok(buffer, " ");
	temp = strtok(NULL, ".");
	
	complete.first = temp;

	temp = strtok(NULL, "\n");

	complete.last = temp;

	return complete;

}


/*
 * Function: return a user read from the buffer
 * 
 * Parameters: the buffer and its length
 * 
 * Return:	the user struct of the user written in the buffer
 * 
 */
user novo_user(char * buffer, int nread){
	char * name;
	char * surname;
	char * ip;
	char * port;
	user valor;
	int i;
	int k;
	
	k = 0;

	name = malloc(sizeof(char)*20);
	memset(name,0,20);
	surname = malloc(sizeof(char)*20);
	memset(surname,0,20);
	ip = malloc(sizeof(char)*20);
	memset(ip,0,20);
	port = malloc(sizeof(char)*20);
	memset(port,0,20);
	
	for (i=4; buffer[i]!='.';i++){
		name[k]=buffer[i];
		k = k + 1;
	}
	k=0;
	for (i=i+1; buffer[i]!=';';i++){
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
	valor.name = name;
	valor.surname = surname;
	valor.ip = ip;
	valor.port = port;

	return valor;
}

/*
 * Function: Function that checks if the list is empty or not 
 * 
 * Parameters: the pointer to the list of registered users
 * 
 * Return:	0 - if the list has users
 * 			1 - if the list is empty
 * 
 */
int lista_vazia(list * lista){
    if(lista == NULL){
        return 1;
    }else{
        return 0;
    }

}
