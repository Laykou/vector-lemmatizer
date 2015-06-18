//  Find words in lemmaformtag.txt that are also in the provided file

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

char w1[max_w], w2[max_w], w3[max_w];
set<string> vocab;

int main(int argc, char **argv)
{
	srand(time(NULL));

	if (argc != 3) {
		printf("MATCHER 1.0\n");
		printf("Find words in lemmaformtag.txt that are also in the provided file\n\n");
    	printf("Usage:\n");

    	printf("\tapp <file> <lemmaformtag_file>\n");
    	printf("\t\tFor every entry in <lemmaformtag_file> check if it exists in the <file>.\n");
    	exit(0);
	}

	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Input file not found\n");
		exit(1);
	}

	fscanf(f, "%lld", &words);
	for (a = 0; a < words; ++a) {
		fscanf(f, "%s%c", w1, &ch);
		vocab.insert(string(w1));
	}
	fclose(f);

	// qsort(vocab, words, max_w * sizeof(char), myCompare);


	f = fopen(argv[2], "r");
	if (f == NULL) {
		printf("Lemmaformtag file not found\n");
		exit(1);
	}

	while(fscanf(f, "%[^\t\n]\t%[^\t\n]\t%[^\t\n]\n", w1, w2, w3) == 3) {
		if(vocab.find(string(w2)) != vocab.end()) {
			printf("%s\t%s\t%s\n", w1, w2, w3);
		}
	}
	fclose(f);

	return 0;
}
