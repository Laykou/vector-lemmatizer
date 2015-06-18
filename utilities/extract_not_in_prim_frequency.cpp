//  Find words in frequencies.txt that are not in the provided file

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     /* atoi */
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <set>
#include <string>
#include <iostream>

#define max_w 50

using namespace std;

// File for input reading
FILE *f;

// Input filenames
char ch;
long long words;
int a;

char w1[max_w], w2[max_w], w3[max_w], w4[max_w];
set<string> vocab;

int main(int argc, char **argv)
{
	srand(time(NULL));

	if (argc != 3) {
		printf("MATCHER 1.0\n");
		printf("Find words in frequencies that are not in the provided file.\n");
		printf("Frequencies are in format: <freq_num>[tab]<lemma>[tab]<word>[tab]<grammar_categ>.\n\n");
    	printf("Usage:\n");

    	printf("\tapp <file> <frequency_file>\n");
    	printf("\t\tFor every entry in <frequency_file> check if it exists in the <file>.\n");
    	exit(0);
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Input file not found\n");
		exit(1);
	}

	while(fscanf(f, "%s", w1) == 1) {
		vocab.insert(string(w1));
	}
	fclose(f);

	// qsort(vocab, words, max_w * sizeof(char), myCompare);


	f = fopen(argv[2], "r");
	if (f == NULL) {
		printf("Frequency file not found\n");
		exit(1);
	}

	while(fscanf(f, "%s%s%s%s", w1, w2, w3, w4) == 4) {
		if(vocab.find(string(w3)) == vocab.end()) {
			printf("%s\t%s\t%s\t%s\n", w1, w2, w3, w4);
		}
	}
	fclose(f);

	return 0;
}
