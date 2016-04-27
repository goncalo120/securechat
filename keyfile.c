/* 
* @Author: goncalo
* @Date:   2016-03-31 19:05:41
* @Last Modified by:   goncalo
* @Last Modified time: 2016-04-03 10:56:30
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv) {

	int i;
	int array[256];
	char* fileName;
	FILE *f;
	int r;

	if(argc < 2){

		printf("choose a name for the file\n");
		exit(1);

	}

	fileName = (char*)malloc(sizeof(char)*(strlen(argv[1])+5));
	sprintf(fileName, "%s.txt", argv[1]);

	printf("fileName = %s\n", fileName);

	f = fopen(fileName, "w");

	srand(time(NULL));

	

	for (i = 0; i < 256; i++) {   
	    array[i] = i;
	}

	for ( i = 0; i < 256; i++) {    
	    int temp = array[i];
	    int randomIndex = rand() % 256;

	    array[i]           = array[randomIndex];
	    array[randomIndex] = temp;
	}


	for ( i = 0; i < 256; i++) {  
	    fprintf(f, "%d\n", array[i]);
	}

	free(fileName);
	fclose(f);

    exit(0);
}