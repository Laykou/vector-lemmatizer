//  Find words in not lemmaformtag.txt that are in the provided file

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
long long words, words2;
int a, b, found;

char w1[max_w], w2[max_w], w3[max_w];
set<string> vocab;

int main(int argc, char **argv)
{
	srand(time(NULL));

	if (argc != 3) {
		printf("MATCHER NOT LEMMA 1.0\n");
		printf("Find words in not lemmaformtag.txt that are in the provided file\n\n");
    	printf("Usage:\n");

    	printf("\tapp <file> <lemmaformtag_file>\n");
    	printf("\t\tFor every entry in <file> check if it exists in the <lemmaformtag>. If not output it.\n");
    	exit(0);
	}

	

	f = fopen(argv[2], "r");
	if (f == NULL) {
		printf("Lemmaformtag file not found\n");
		exit(1);
	}

	while(fscanf(f, "%[^\t\n]\t%[^\t\n]\t%[^\t\n]\n", w1, w2, w3) == 3) {
		vocab.insert(string(w2));
	}

	fclose(f);

	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Input file not found\n");
		exit(1);
	}

	fscanf(f, "%lld", &words2);
	for (b = 0; b < words2; ++b) {
		fscanf(f, "%s%c", w1, &ch);
		
		if(vocab.find(string(w1)) == vocab.end()) {
			printf("%s\n", w1);
		}
	}

	fclose(f);

	return 0;
}
