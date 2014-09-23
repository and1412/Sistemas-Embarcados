#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "experimento1.h" 
#include <signal.h>

void sigintHandler()
{
		printf("\n");
		switch (execution){
			case 0:
				printf("\r READING AND BINARIZE IMAGE - %d COMPLETED...",percentage);
				break;
			case 1:
				printf("\r WRITING IMAGE TO FILE - %d COMPLETED...",percentage);
				break;
			case 2:
				printf("\r GET MATRIX RECURRENCE - %d COMPLETED...",percentage);
				break;
			case 3:
				printf("\r SORT FREQUENCE VECTOR - %d COMPLETED...",percentage);
				break;
			case 4:
				printf("\r SIGN IMAGE - %d COMPLETED...",percentage);
				break;
		}
        fflush(stdout);
}

int main(int argc, char *argv[]){

	signal(SIGINT, sigintHandler); 
	char *filename;
	int result;
	PGM *pgm = malloc(1 * sizeof(PGM));
	
	if(argc != 3){
		fprintf(stderr, "\033[22;31mOnly two arguments are allowed: %s <FILENAME> <DEBUG FLAG 0 or 1>\n\033[0m", argv[0]);
		exit(-1);
	}
	debug = atoi(argv[2]);
	filename = argv[1];
	pgm = readPGM(filename, pgm);
 	char *output = "check.pgm";
 	writePGM(output,pgm,1);
 	result = prepareImage(pgm);
 	if (result){
 		printf("\n[SUCCESS] - Image was signed.\n");
 	}
 	else
 		fprintf(stderr,"\n[ERROR] - Image wasn't signed.\n");

	return 0;
}
