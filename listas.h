#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* User Structure*/
typedef struct user{
	char* name;
	char* surname;
	char* ip;
	char* port;
}user;

/* List Structure*/
typedef struct list{
	user valor;
	struct list * prox;
	struct list * ant;
}list;

/* Complete name Structure*/
typedef struct completename{
	char * first;
	char * last;
}completename;


list * AllocatesList();
void print_user(user valor);
void imprime_lista(list * lista);
void insere_lista(list *lista , user n);
void remove_lista(list *lista , char * name, char * surname);
completename encontra_nomes(char * buffer, int nread);
int getuser(list * lista, char * nome, char * surname);
user novo_user(char * buffer, int nread);
int lista_vazia(list * lista);

