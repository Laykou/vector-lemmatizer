//  Find words frequency analyzer that are also in lemma form tag file.

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
#include <map>

#define max_w 700

using namespace std;

// File for input reading
FILE *f;

// Input filenames
char ch;
long long words;
int freq;

char w1[max_w], w2[max_w], w3[max_w];
map<string,string>::iterator it;
map<string, string> list;

int main(int argc, char **argv)
{
	srand(time(NULL));

	if (argc != 3) {
		printf("MATCHER 1.0\n");
		printf("Find words frequency analyzis that are also in the lemmaformtag\n\n");
    	printf("Usage:\n");

    	printf("\tapp <file> <lemmaformtag_file>\n");
    	printf("\t\tFor every entry in <file> check if it exists in the <lemmaformtag_file> and return both rows concated.\n");
    	exit(0);
	}

	f = fopen(argv[2], "r");
	if (f == NULL) {
		printf("Lemmaformtag file not found\n");
		exit(1);
	}

	while(fscanf(f, "%[^\t\n]\t%[^\t\n]\t%[^\t\n]\n", w1, w2, w3) == 3) {
		list[string(w2)] = string(w1) + "\t" + string(w2) + "\t" + string(w3);
	}
	fclose(f);

	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Input file not found\n");
		exit(1);
	}

	while(fscanf(f, "%s%d", w1, &freq) == 2) {
		it = list.find(string(w1));  
		if(it != list.end()) {  
			printf("%d\t%s\n", freq, it->second.c_str());
		}
	}
	fclose(f);

	return 0;
}
