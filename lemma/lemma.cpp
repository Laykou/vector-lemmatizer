/**
 * Copyright (C) 2015, Ladislav Gallay, Ing. Marian Simko, PhD.
 * Licensed under The MIT License
 *
 * This application is part of my bachelor thesis: Utilizing Vector Models for Processing Text on the Web
 * 
 * Run the program with no arguments to see the help.
 * To compile this program we recommed using g++ as follows:
 * g++ lemma.cpp -o lemma -lm -pthread -Ofast -march=native -Wall -funroll-loops -Wno-unused-result -std=c++11 -std=gnu++11
 *
 * Note: In many cases the wchar_t is used instead of char. Wchar stands for wide-character and character can be represents
 * by more bytes. The algorithm uses diacritis therefore standard char cannot be used. The problem is in strlen and strcpy
 * functions where standard char is counted by the bytes and not by the real letters. the wcslen and wcscpy methods are used instead.
 * Also the scanf/printf methods need the %ls placeholder.
 *
 * Note 2: "Dictionary" is the same as the "reference lexicon", "lexicon", "reference pairs".
 *
 * Note 3: Output is separated by tabs.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>     
#include <math.h>
#include <time.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <bits/types.h>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <wctype.h>

/**
  * Define the order of the special letters so that sorting of strings works correctly. See the wcscompare method.
  * This string should be max. 200 chars long. For more you need to update the main() method.
  */
#define ALPHABET L"aáäbcčdďeéfghiíjklĺľmnňoóôpqrŕsštťuúvwxyýzžAÁÄBCČDĎEÉFGHIÍJKLĹĽMNŇOÓÔPQRŔSŠTŤUÚVWXYÝZŽ"

/**
 * Maximum length of words. This number is used to allocate the strings in memory.
 */
#define max_size 2000

/**
 * Maximum number of closest words that will be evaluated. This number is used to
 * allocate the memory. Use the "word_analogy_iterations" iterations to set the number
 * of words that will be evaluated.
 * Keep in mind that: word_analogy_iterations <= N
 */
#define N 40

 /**
  * Maximum number of words in the input for each test case. Each test can consist of one or
  * more words. You can pass the arguments mentioned in the help and modify the variables at the runtime.
  * First non-argument word is taken as the input word. You can add another words, that will be just outputed.
  * For example, if you put "autom auto 132 -iterations 2", the match_iterations variable will be set to 2
  * and the algorithm will continue with the "autom auto 132" input. The first word "autom" will be lematized
  * and on stdin will be printed the whole input and the results, e.g. "autom auto 132 motorka 0,9745 auto 0,8464".
  * The additional words are just bypass to the output. You can pass the correct lemma for easier evaluation of results.
  */
#define W 20

/**
 * This number is used in allocation the vocabulary memory. Words are read from the model binary file.
 * The max_w variable is original variable from the word2vec tool. The difference between max_size is that
 * max_w should be known value, because the model is constructed on the server side, however max_size expects
 * any input from the client side and therefore should be a bit larger.
 */
#define max_w 50

/**
 * Denotes the maximum number of reference words that will be evaluated for each input word. The IT variable
 * is used to allocate the memory in the beginning, however match_iterations is used in the runtime.
 * Keep in mind that: match_iterations <= IT
 */
#define IT 20

/**
 * When using the socket access instead of the console, this number denotes the maximum lenght of the output buffer.
 * Make this number large enough to be able to hold the whole message that server can produce at once.
 */
#define MESSAGE 100000

/**
  * Range of the INT values of wchar_t symbols for slovak alphabet. The highest is for "Ž"=381. You do not need
  * to understand or change this constant. See the wcscompare for more details.
  */
#define ALPHA 500

using namespace std;


// ----------------------------------------
// Options - you can change these variables
// to modify the default behavior. See the
// help or the description to understand
// what is each variable doing.
// ----------------------------------------

/**
 * Modifies the method of choosing the reference words from the reference lexicon.
 * 0 - Closer words to the input word will be evaluated first (if exist in lexicon). See the
 *      closestCount variable to set how many closest words will be retrieved for input word.
 * 1 - Words from reference lexion are chosen by the longest common suffix.	
 */
int choosingMethod = 1;

/**
 * If the choosing method is set to 0 (closer words), this number denotes how many
 * words will be fetched from the distance() method when retrieving the list of closest words
 * for given input.
 */
int closestCount = 40;

/**
 * If false (0) each input line will have only one-line output. The input will be bypassed to
 * the output followed by the results.
 * If true (1) more debug information is being printed as the algorithm processes the input.
 */
int quiet = 0;

/**
 * True (1) will print the vocabulary trained in the model after it is loaded. It is good to see what is stored in
 * the vocabulary as the model is a binary file. See more in loadModel() method.
 */
int printModel = 0;

/**
 * True (1) will print the dictionary after it is loaded. It is good to see what is stored in
 * the dictionary entry as it is formed of 3 words based on the application mode (normal/categories).
 */
int printDictionary = 0;

/**
 * Count of words that will be fetched from the word_analogy() method for each iteration.
 */
int word_analogy_iterations = N;

/**
 * Number of iterations - number of distinct reference pairs that will be chosen and evaluated by
 * the word_analogy() method.
 */
int match_iterations = 3;

/**
 * Changes the behavior of the application. Changin to true (1) will start the application in server mode. It will open
 * socket connection and listen on port (default: 4332) for incomming connections. Client can send the send message as
 * to the standard input and expects same message as from the standard output.
 * Default behavior is the 'console' mode.
 */
int server = 0;

/**
 * Port on which the server will be listening (if enabled server mode).
 */
int port = 4332;

/**
 * Number of threads in the console mode. Each thread will read one input line, process it and asynchronly output the
 * results. Therefore is good to know, that the input line is part of the output (except additional arguments which are stripped).
 * The lines on the ouput does not need to match the order of the input.
 */
int threads = 1;

/**
 * Denotes the number of top words that will be printed. The results are sorted by their weights and only the first
 * result should be the correct lemma. However for testing purposes is good to see other positions. Useful values is 1 or 5.
 */
int showOutputs = 1;

/**
 * Each word from the word_analogy() method is weighted. There are several weighting methods:
 * 0 - Relative common prefix - prefix length of the input word and the lemma candidate (normalized)
 * 1 - Jaro-winkler distance
 * 2 - Levenshtein distance
 * 3 - Cosine distance only (no weighting performed, only the distance from word_analogy() method is used)
 */
int weighting = 0;

/**
 * Changes the mode of the application. True (1) means that each entry in dictionary has additional category (see
 * the loadDictionary() method). Each input than must consist of at least two words. First is the category that must
 * match on of the categories mentioned in the dictionary and the second is the input word.
 * Standard behavior requires only one word - the first word in the line is the input word.
 */
int categories = 0;

/**
 * Boolean indicates whether to print the reference pairs in the output that were used in word_analogy() method.
 * Number of this pairs is determined by the "match_iterations" variable (-iterations argument).
 *
 * Standard behavior is to print "[input] [results]", e.g. for input "autom auto 10": "autom auto 10 auto 0,974 motorka 0,876".
 * Printing the pairs will output "[input] [pairs] [result]", e.g. for 2 iterations: "autom auto 10 dubom dub chlapom chlap auto 0,974 motorka 0,876".
 */
int printPairs = 0;


// --------------------------------------
// **** DO NOT CHANGE VARIABLES BELOW ***
// * UNLESS YOU KNOW WHAT YOU ARE DOING *
// --------------------------------------

/**
 * Filename for model and dictionary files.
 */
char model_file[max_size], dictionary_file[max_size];

/**
 * Dictionary holds 3 words for each entry: "reversed word, word, lema". For categories the first word is the category name.
 * Use the dic() function to access the pointer of any word. See the documenation for better understanding.
 */
wchar_t *dictionary;

/**
 * Denotes the size of current dictionary. The actual number of words in dictionary is 3-times larger, because
 * every dictionary entry is represented by 3 words (see the doc above).
 */
int dictionarySize;

/**
  * If running in "categories" mode, thise variable holds the first and last index in the dictionary for each category.
  * The dictionary is sorted by categories. Use this variable to quickly iterate only the entries in the specified category.
  */
map<wstring, pair<int, int> > dictionaryCategories;

/**
  * This variable is used as the output buffer. When running in server mode, the reponses are written here and sent
  * via socket in the end of processing. See the MESSAGE for the maximum size of the output buffer. To write to the
  * output buffer use the o() method. If you want to print the output to the console see the print() method.
  */
char *outputBuffer;

/**
  * Original variable from the word2vec tool. Denotes the current number of words in vocabulary - the vocab size.
  */
long long words;

/**
  * Original variable from the word2vec tool. Denotes the list of words. The index will match the index in the M variable,
  * which is the list of vectors for each word.
  */
wchar_t *vocab;

/**
 * Original variable from the word2vec tool. Denotes the size of vector, e.g. 200 or 300. The M variable then contains for
 * each words "size" float numbers - latent vector.
 */
long long size;

/**
 * Original variable from the word2vec tool. List of vectors. The two dimensional array is compressed into one dimension with
 * "size" numbers for each word in the vocab.
 */
float *M;

/**
 * In original examples from word2vec the input words was found in the vocab by simply iterating it. To make things fast
 * the vocabulary words are also contained in this list. The number denotes the index of the word. The complexicity is constant
 * in average instead of linear searching as it was before. See the distance.h and word-analogy.h.
 */
unordered_map<wstring, int> wp;

/**
 * Application supports multi thread processing. Each thread reads the one case from the input and calculates the lemma.
 * This variable holds the list of all running threads.
 */
pthread_t thrs[16];

/**
 * When using multiple threads the standart input should be locked while reading so that one thread reads the whole test-case input.
 * See the runConsole() function.
 */
pthread_mutex_t inputLock;

/**
 * Socket connection ID when listening on port. Used to close the listener on SIGINT signal (when existing the application).
 */
int sockfd;

/**
  * List of order for each wchar_t letter in slovak alphabet. Index is it's integer representation.
  * For example index 97 has value 0 as it means "a" is the first letter. Non-defined letters will have -1. See the wcscompare
  * and the main method for how is this array filled.
  */
int sortAlpha[ALPHA];

/**
 * To make the code a bit cleaner some imporant methods were extracted into separate files.
 */
#include "word-analogy.h"
#include "distance.h"
#include "jaro-winkler.h"
#include "levenshtein.h"

/**
  * Referse the string. Strrev function is not available on Linux.
  * Note that the string is not copied but changed instead. The pointer of p is returned.
  * To generate new string run the 'strrev(wcscpy(new, original));'.
  *
  * @param wchar_t* p String to be reversed.
  * @return wchar_t* Pointer of the word.
  */
wchar_t *strrev(wchar_t *p)
{
	int length = wcslen(p), i, j;
    wchar_t c;

    for (i = 0, j = length - 1; i < j; i++, j--) {
        c = p[i];
        p[i] = p[j];
        p[j] = c;
    }
    return p;
}

/**
  * Returns the pointer to current pointer in the outpu buffer. Use sprintf to write to the output buffer:
  * sprintf(o(), "This is my message with number %d\n", 123);
  * 
  * To print the output use the print() method. If not running in the console mode, nothing will be printed.
  * Because of performance issue we decided to use sprintf() only in the server mode. Following code is used
  * accross the application to determine whether running in console or server mode:
  * 
  * if (server) {
  * 	sprintf(o(), "The number of results is %d\n", resNum);
  * } else {
  *		printf(, "The number of results is %d\n", resNum);
  * }
  *
  * @return char* Pointer to the current position in the output buffer.
  */
char *o()
{
	return outputBuffer + strlen(outputBuffer) * sizeof(char);
}

/**
  * Prints the output buffer to the standart output and empties the buffer.
  * The buffer will be printed to the stdin even when running in server mode.
  *
  * @return void
  */
void forcePrint()
{
	if (strlen(outputBuffer)) {
		printf("%s", outputBuffer);
	}
	outputBuffer[0] = 0;
}

/**
  * Prints the output buffer to the standart output and empties the buffer.
  * This function does nothing when running in server mode. You must implement custom reading of the buffer.
  *
  * @return void
  */
void print()
{
	if (!server) {
		forcePrint();
	}
}

/**
 * This method will flip the values and types of the pair variable. Function is used to sort the map
 * by values instead of keys. E.g. having the <string, int> map, the map will be sorted by integer values,
 * and not by alphabetically by the strings, which is the default behavior.
 */
template<typename A, typename B>
pair<B,A> flip_pair(const pair<A,B> &p)
{
    return pair<B,A>(p.second, p.first);
}

/**
 * Sort the map by the values not by the keys, E.g. having the <string, int> map, the map will be sorted by integer values,
 * and not by alphabetically by the strings, which is the default behavior. The map is then reversed: <int, string> map is
 * outputed. As this can lead to multiple values for the same key, the multimap is returned instead.
 *
 * @param map<A,B> src Source map to be sorted by values.
 * @return multimap<B,A> Multimap sorting the map by values.
 */
template<typename A, typename B>
multimap<B,A> flip_map(const map<A,B> &src)
{
    multimap<B,A> dst;
    transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A,B>);
    return dst;
}

/**
  * Load trained binary file from word2vec tool. The binary reading is faster than reading the text file.
  * If you want to see what words are inside, see the "-print_model" argument in the help.
  *
  * @return int 0 if everyting OK, else 1 (if not enough memory or file not found).
  */
int loadModel(char *filename)
{
	long long a, b;
	float len;
	wchar_t ch;

	// Check the filename
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		if (server) {
			sprintf(o(), "Model: Input file not found\n");
		} else {
			printf("Model: Input file not found\n");
		}
		return 1;
	}

	// Allocate the memory for vocabulary and vector
	fscanf(f, "%lld", &words);
	fscanf(f, "%lld", &size);
	vocab = (wchar_t *)malloc((long long)words * max_w * sizeof(wchar_t));
	M = (float *)malloc((long long)words * (long long)size * sizeof(float));
	if (M == NULL) {
		if (server) {
			sprintf(o(), "Cannot allocate memory: %lld MB  (%lld words, %lld size)\n", (long long)words * size * sizeof(float) / 1048576, words, size);
		} else {
			printf("Cannot allocate memory: %lld MB  (%lld words, %lld size)\n", (long long)words * size * sizeof(float) / 1048576, words, size);
		}
		return 1;
	}

	// wp is used for fast lookup of index in the vocab array
	wp.clear();

	// Original implementation of reading by word2vec tool.
	for (b = 0; b < words; ++b) {
		fscanf(f, "%ls%lc", &vocab[b * max_w], &ch);
		wp[wstring(&vocab[b * max_w])] = b;

		for (a = 0; a < size; ++a)
			fread(&M[a + b * size], sizeof(float), 1, f);
		len = 0;
		for (a = 0; a < size; ++a)
			len += M[a + b * size] * M[a + b * size];
		len = sqrt(len);
		for (a = 0; a < size; ++a)
			M[a + b * size] /= len;
	}
	fclose(f);

	// If requested, the words in model are printed. Can be usefull to see what is inside of binary model.
	if (printModel) {
		printf("%lld\n", words);
		for (a = 0; a<words; ++a) {
			printf("%ls\n", &vocab[a * max_w]);
		}
	}
	return 0;
}

/**
  * Return n-th word in dictionary - the w-th format. Use this function for better compatibility and changes in the future.
  * W can have these values
  * 
  * 0 - reversed input word (e.g. motua)
  * 1 - input word (e.g. autom)
  * 2 - root form (e.g. auto)
  *
  * @param int n Index of the entry in the dictionary.
  * @param int w The exact word format for the entry (as described above).
  * @return wchar_t* Requested string.
  */
wchar_t *dic(int n, int w)
{
	return &dictionary[(3 * n + w) * max_size];
}

/**
  * Compare two strings according to slovac diacritic
  */
int wcscompare (const wchar_t* s1, const wchar_t* s2) {
  while ((*s1 && *s2) && (*s1 == *s2)) s1++,s2++;
  // Test if we have custom order for the letters. If yes determine the order based on this custom order.
  if (*s1<ALPHA && *s2<ALPHA && sortAlpha[*s1] != -1 && sortAlpha[*s2] != -1) return sortAlpha[*s1] - sortAlpha[*s2];
  return *s1 - *s2;
}

/**
 * The entries in dictionary sorted by the first string in dictionary entry.
 * Standart this word is reversed word in dictionary, so the entries are sorted by suffix. Running in categories mode
 * the first word is the category name. Dictionary is sorted by these categories and dictionaryCategories variable is used
 * to determine being/end index for each category. However for next processing it is important to sort the entries in each
 * category by the suffix. This compare method is used in qsort() for each category separately a compares the second word
 * in dictionary entry (e.g. autom) by it's reversed form. Therefore the dictionary is sorted by categories and by suffixes.
 *
 * @param void* a First word to compare.
 * @param void* b Second word to compare.
 * @result int Returns the output of wcscmp() method (less than zero = a is before b).
 */
int dictionaryCategoriesCompare(const void * a, const void * b)
{
	wchar_t *rev1, *rev2, *rev3, *rev4;
	rev1 = (wchar_t*) malloc(max_size * sizeof(wchar_t));
	rev2 = (wchar_t*) malloc(max_size * sizeof(wchar_t));
	rev3 = strrev(wcscpy(rev1, (wchar_t*)a + max_size));
	rev4 = strrev(wcscpy(rev2, (wchar_t*)b + max_size));
	int comp = wcscompare(rev3, rev4);
	free(rev1);
	free(rev2);
	return comp;
}

/**
  * Load dictionary file. The dictionary files expects to have the number of entries on the first line.
  * Each line contains two words "word lemma", e.g. "autom auto".
  * When running in categories mode, each line expects to have three words where first word is the category
  * name, e.g. "ms7 autom auto".
  *
  * @param char* filename Name of the dictionary file to be loaded.
  * @return int 0 if everyting OK, else 1 (if not enough memory or file not found)
  */
int loadDictionary(char *filename)
{
	long long a, b;

	// Check the filename
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		if (server) {
			sprintf(o(), "Dictionary: Input file not found\n");
		} else {
			printf("Dictionary: Input file not found\n");
		}
		return 1;
	}

	// Allocate the memory
	fscanf(f, "%d", &dictionarySize);
	dictionary = (wchar_t *)malloc(3 * dictionarySize * max_size  * sizeof(wchar_t));
	if (dictionary == NULL) {
		if (server) {
			sprintf(o(), "Could not allocate enough memory\n");
		} else {
			printf("Could not allocate enough memory\n");
		}
		return 1;
	}

	// Read the dictionary
	for (a = 0; a < dictionarySize; ++a) {
		if (categories) {
			// For categories first world in dictionary will not be reversed word but the category name.
			// More words can have the same category
			fscanf(f, "%ls%ls%ls", dic(a, 0), dic(a, 1), dic(a, 2));
		} else {
			fscanf(f, "%ls%ls", dic(a, 1), dic(a, 2));
			// Copy the word also to reversed list and reverse it
			strrev(wcscpy(dic(a, 0), dic(a, 1)));
		}
	}

	// Sort dictionary by the first word (reversed word or category).
	// Using bubblesort is not bad, because there are cca. 500 words in dictionary and this is done only once.
	wchar_t tmp[3 * max_size];
	for (a = 0; a < dictionarySize; ++a) {
		for (b = a; b < dictionarySize; ++b) {
			if(wcscompare(dic(a, 0), dic(b, 0)) > 0) {
				memcpy(tmp, dic(a, 0), 3 * max_size * sizeof(wchar_t));
				memcpy(dic(a, 0), dic(b, 0), 3 * max_size * sizeof(wchar_t));
				memcpy(dic(b, 0), tmp, 3 * max_size * sizeof(wchar_t));
			}
		}
	}
	fclose(f);

	// Memorize categories' first/last index and sort words in each category
	// by suffix (see the dictionaryCategoriesCompare() method)
	if(categories) {
		wchar_t *previous = NULL;
		for (a = 0; a < dictionarySize; ++a) {
			if (previous == NULL || wcscmp(previous, dic(a, 0)) != 0) {
				if (previous != NULL) {
					// Sort only the category in the array
					dictionaryCategories[wstring(previous)].second = a;
					int firstItem = dictionaryCategories[wstring(previous)].first;
					qsort(dic(firstItem, 0), a - firstItem, 3 * max_size * sizeof(wchar_t), dictionaryCategoriesCompare);
				}
				dictionaryCategories[wstring(dic(a, 0))] = make_pair(a, -1);
				previous = dic(a, 0);
			}
		}
		dictionaryCategories[wstring(previous)].second = dictionarySize;
		int firstItem = dictionaryCategories[wstring(previous)].first;
		qsort(dic(firstItem, 0), dictionarySize - firstItem, 3 * max_size * sizeof(wchar_t), dictionaryCategoriesCompare);
	}

	// Optionally print the dictionary if requested
	if(printDictionary) {
		printf("%d\n", dictionarySize);
		for (a = 0; a < dictionarySize; ++a) {
			printf("[%lld] %ls %ls %ls\n", a, dic(a, 0), dic(a, 1), dic(a, 2));
		}
	}
	return 0;
}

/**
  * Returns number of common common characters in both strings from the beginning
  *
  * @param wchar_t* w1 First string.
  * @param wchar_t* w2 Second string.
  * @return int Length of the longest common prefix.
  */
int prefixLength(wchar_t *w1, wchar_t *w2)
{
	int i = 0;
	while((w1[i] && w2[i]) && (w1[i] == w2[i])) ++i;
	return i;
}

/**
 * The calculation of weighting function for the W0 method (as mentioned in the bachelor thesis).
 * The longest common prefix is normalized by the relative length of both strings.
 *
 * @return float Weight of the input word.
 */
float relativePrefixWeight(wchar_t *inputWord, wchar_t *word, int lenIn)
{
	return (float) prefixLength(inputWord, word) / (((float)lenIn + (float)wcslen(word)) / 2.0);
}

/**
  * Returns the weight
  * inputWord - the word for which the lemma is being evaluated
  * secondWord - current word to be weighted
  * lenIn - the lenght of the input word (should be precalculated)
  * dist - total distance for all iterations (sum) between input word and second word
  */
float match(wchar_t *inputWord, wchar_t *secondWord, int lenIn, float dist)
{
	if (weighting == 0) { // Relative prefix length + cosine distance
		return (float) relativePrefixWeight(inputWord, secondWord, lenIn) * fabs(dist);
	} else if (weighting == 1) { // Jaro-Winkler distance + cosine distance
		return (float) jaro_winkler_distance(inputWord, secondWord, lenIn) * fabs(dist);
	} else if (weighting == 2) { // Lehvenstein distance + cosine distance
		return (float) levenshtein_distance(inputWord, secondWord, lenIn) * fabs(dist);
	} else if (weighting == 3) { // Distance only
		return dist;
	}
	return 0;
}

/**
  * Position of given argument in argument array
  */
int ArgPos(char *str, int argc, char **argv)
{
	int a;
	for (a = 0; a < argc; ++a) { // ARGV starts from index 1 but the input arguments from 0
		if (!strcmp(str, argv[a])) {
			// The argument is destroyed after usage, so that when processing the input, there are no arguments.
			// See the description of the "#define W" for more details.
			argv[a][0] = 0;
			return a;
		}
	}
	return -1;
}


/**
  * Return index into dictionary for best word to perform word_analogy with.
  * Best suffixes are already precalculated so just find first best unvisited
  */
int findBestDepthWord(int *inputDictionaryDepth, int *inputDictionaryStart, int *inputDictionaryPrefixMem, char *visitedDictionaryWord, int maxSize)
{
	int c;
	while(1) {
		// Skip all depths where there is no suffixLength match in dictionary and input
		while(inputDictionaryStart[*inputDictionaryDepth] == -1) {
			if(*inputDictionaryDepth == 0) {
				return -1;
			}
			--(*inputDictionaryDepth);
		}

		// This was the best word with best suffix match for given lenth
		c = inputDictionaryStart[*inputDictionaryDepth];
		// Go word-by-word in the dictionary but only while the suffixLength is the same as the depth we are in
		while(inputDictionaryPrefixMem[c] == *inputDictionaryDepth && c < maxSize) {
			if(visitedDictionaryWord[c] == 0) {
				// Save next word as the best, so by next searching it will jump directly to this word
				inputDictionaryStart[*inputDictionaryDepth] = c + 1;
				return c;
			}
			++c;
		}

		// On the zero level and last dictionary word - there is nothing more to search for
		if(*inputDictionaryDepth == 0 || c >= maxSize)
			return -1;

		// Nothing found for this suffix length. Go one letter back (one level up)
		--(*inputDictionaryDepth);
	}
}

/**
  * Return index into dictionary for the next closest word that is also in the dictionary.
  */
int findBestClosestWord(int *currentClosestWord, char *visitedDictionaryWord, wchar_t closestw[N][max_size])
{
	while (*currentClosestWord < closestCount) {
		++(*currentClosestWord);
		
		for (int c=0; c<dictionarySize; ++c) {
			// Compare the word in dictionary in non-root form with the closest word
			if (visitedDictionaryWord[c] == 0 && !wcscmp(dic(c, 1), closestw[*currentClosestWord])) {
				if(!quiet){
					if (server) {
						sprintf(o(), "%d-th closest word %ls was found in dictionary: '%ls' '%ls' '%ls'\n", *currentClosestWord, closestw[*currentClosestWord], dic(c, 0), dic(c, 1), dic(c, 2));
					} else {
						printf("%d-th closest word %ls was found in dictionary: '%ls' '%ls' '%ls'\n", *currentClosestWord, closestw[*currentClosestWord], dic(c, 0), dic(c, 1), dic(c, 2));
					}
				}
				return c;
			}
		}
	}
	return -1;
}

/**
  * Find the next best word to be used in the word_analogy function
  *
  * If the choosingMethod = 0, find the closest word using distance. If not in dictionary, use the longest suffix method.
  * If the choosingMethod = 1, find the word only by longest suffix.
  */
int findBestWord(int *inputDictionaryDepth, int *inputDictionaryStart, int *inputDictionaryPrefixMem, char *visitedDictionaryWord, int *currentClosestWord, wchar_t closestw[N][max_size], wchar_t *category)
{
	// Search word in category
	if (categories) {
		try {
			pair<int, int> pos = dictionaryCategories.at(wstring(category));
			return findBestDepthWord(inputDictionaryDepth, inputDictionaryStart, inputDictionaryPrefixMem, visitedDictionaryWord, pos.second);
		} catch (const out_of_range& r) {
			return -1;
		}
		return -1;
	}

	int bestClosestWord = -1;

	if (choosingMethod == 0) {
		bestClosestWord = findBestClosestWord(currentClosestWord, visitedDictionaryWord, closestw);
	}

	if (bestClosestWord == -1) {
		return findBestDepthWord(inputDictionaryDepth, inputDictionaryStart, inputDictionaryPrefixMem, visitedDictionaryWord, dictionarySize);
	} else {
		return bestClosestWord;
	}
}

/**
 * Outputs the help into the output buffer. To see the help in the console launch the print() method.
 *
 * @return void
 */
void help()
{
		sprintf(o(), "Vector Lematizator 1.0 (c) Ladislav Gallay, Ing. Marian Simko, PhD.\n\n");
		sprintf(o(), "Program to lemmatize words based on the trained vector model and reference dictionary.\n");
		sprintf(o(), "This project is part of the bachelor thesis:\n");
		sprintf(o(), "Utilizing Vector Models for Processing Text on the Web @ FIIT STUBA 2015.\n");
    	sprintf(o(), "\nOptions:\n");

    	sprintf(o(), "\t-model <file>\n");
    	sprintf(o(), "\t\tLoad the model from binary <file>. Train the model with word2vec tool.\n");

    	sprintf(o(), "\t-dictionary <file>\n");
    	sprintf(o(), "\t\tLoad the reference pairs from the <file>.\n");

    	sprintf(o(), "\t-print_dictionary\n");
    	sprintf(o(), "\t\tPrint the reference pairs.\n");

    	sprintf(o(), "\t-print_model\n");
    	sprintf(o(), "\t\tPrint the trained model.\n");

    	sprintf(o(), "\t-print_word_analogy_input\n");
    	sprintf(o(), "\t\tPrint the word-analogy input for each testing.\n");

    	sprintf(o(), "\t-print_pairs\n");
    	sprintf(o(), "\t\tSimiliar to print_word_analogy_input but it will print the pairs in the output line. So the output will be '[input words] [pairs] [show results]'.\n");
    	sprintf(o(), "\t\tFor example: 'autom auto ms7 dubom dub chlapom chlap strojom stroj auto 0,456794 motorka 0,234879 vlaky 0,136789 papierom 0,015964'.\n");

    	sprintf(o(), "\t-quiet\n");
    	sprintf(o(), "\t\tPrint results only. No steps or other processing/execution info.\n");

    	sprintf(o(), "\t-categories\n");
    	sprintf(o(), "\t\tEnable categories processing. Dictionary should contain words in format '<category> <word> <lemma>' and input should be in format '<category> <input>'.\n");

    	sprintf(o(), "\t-choosing_method <integer> (default: %d)\n", choosingMethod);
    	sprintf(o(), "\t\tUse <integer> to set the method used for choosing the pairs.\n");
    	sprintf(o(), "\t\t\t0 - Find semantic closest word (using the distance function). If the word is not in dictionary find by the suffix.\n");
    	sprintf(o(), "\t\t\t1 - Find the word in the dictionary only by the longest suffix with the input word.\n");

    	sprintf(o(), "\t-weighting <integer> (default: %d)\n", weighting);
    	sprintf(o(), "\t\tUse <integer> to set the method used for weighting the results.\n");
    	sprintf(o(), "\t\t\t0 - Relative prefix length + cosine distance.\n");
    	sprintf(o(), "\t\t\t1 - Jaro-Winkler distance + cosine distance.\n");
    	sprintf(o(), "\t\t\t2 - Levehnstein distance + cosine distance.\n");
    	sprintf(o(), "\t\t\t3 - Cosine distance only.\n");

    	sprintf(o(), "\t-word_analogy <integer> (default: %d)\n", word_analogy_iterations);
    	sprintf(o(), "\t\tUse <integer> to set the number of words to be found by word_analogy method\n");

    	sprintf(o(), "\t-iterations <integer> (default: %d)\n", match_iterations);
    	sprintf(o(), "\t\tUse <integer> to set the number of iterations for each input word. Can be overwritten by each input\n");

    	sprintf(o(), "\t-closest_count <integer> (default: %d)\n", closestCount);
    	sprintf(o(), "\t\tUse <integer> to set the number of the closest words to investigate when using the distance method/strategy.\n");

    	sprintf(o(), "\t-show_outputs <integer> (default: %d)\n", showOutputs);
    	sprintf(o(), "\t\tUse <integer> to set the number of first N result words to show in  output. They will be in format 'word weight' separated by spaces.\n");

    	sprintf(o(), "\t-server\n");
    	sprintf(o(), "\t\tBy default the app run in console mode but it can run as server app listening on <port>.\n");

    	sprintf(o(), "\t-port <integer> (default: %d)\n", closestCount);
    	sprintf(o(), "\t\tWhen running as server the app will listen on this port.\n");

    	sprintf(o(), "\t-threads <integer> (default: %d)\n", threads);
    	sprintf(o(), "\t\tNumber of threads proocessing the input.\n");
}

/**
  * If the value on n-th position is boolean (1 or 0) it is returned and removed.
  * Else 1 is returned.
  * The reason is that you can pass "-quiet" to indicate the quiet mode or "-quiet 1"/"-quiet 0" to explicitly
  * set the value true/false for the parameter. Other values than 1/0 will not be evalueted as part of the argument.
  * E.g. "-quiet -iterations 3" will handle the word "-iterations" as next argument whether "-quiet 1 -iterations 3"
  * will handle the "1" as the value for the quiet argument.
  */
int resolveBoolean(int n, int argc, char **argv)
{
	if (n < argc) {
		if (argv[n][0] == '1') {
			*argv[n] = 0;
			return 1;
		}
		if (argv[n][0] == '0') {
			*argv[n] = 0;
			return 0;
		}
	}
	return 1;
}

/**
 * Loads the arguments from the list of words. This is evaluated before each input processing.
 * The algorithm options can be changed runtime and can be for each input different. The arguments and their values
 * are then ereased leaving an empty string. Only non-argument words will remain which will by forwarded into
 * the processInput method.
 * The boolean options have optional argument 1/0 to explicitly set the argument. Without any
 * argument the boolean option evaluates to true. See the resolveBoolean() method for more examples.
 * 
 * @param int argc Number of words (arguments).
 * @param char** argc List of arguments to evaluate.
 * @return void
 */
void loadArguments(int argc, char **argv)
{
	int i;
	model_file[0] = 0;
	dictionary_file[0] = 0;
	if((i = ArgPos((char *)"-model", argc, argv)) >= 0) {
		strcpy(model_file, argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-dictionary", argc, argv)) >= 0) {
		strcpy(dictionary_file, argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-print_dictionary", argc, argv)) >= 0) {
		printDictionary = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-print_pairs", argc, argv)) >= 0) {
		printPairs = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-print_model", argc, argv)) >= 0) {
		printModel = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-quiet", argc, argv)) >= 0) {
		quiet = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-categories", argc, argv)) >= 0) {
		categories = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-choosing_method", argc, argv)) >= 0) {
		choosingMethod = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-word_analogy", argc, argv)) >= 0) {
		word_analogy_iterations = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-show_outputs", argc, argv)) >= 0) {
		showOutputs = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-iterations", argc, argv)) >= 0) {
		match_iterations = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-weighting", argc, argv)) >= 0) {
		weighting = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-closest_count", argc, argv)) >= 0) {
		closestCount = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-server", argc, argv)) >= 0) {
		server = resolveBoolean(i + 1, argc, argv);
	}
	if((i = ArgPos((char *)"-port", argc, argv)) >= 0) {
		port = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
	if((i = ArgPos((char *)"-threads", argc, argv)) >= 0) {
		threads = atoi(argv[i + 1]);
		*argv[i + 1] = 0;
	}
}

// Print pairs - word analogy input (into one line separated by tabs)
void printPairsWai(wchar_t *wai[IT][4])
{
	if (printPairs) {
		int i;
		for(i=0; i<match_iterations; ++i) {
			if (server) {
				sprintf(o(), "\t%ls\t%ls", wai[i][0], wai[i][1]);
			} else {
				printf("\t%ls\t%ls", wai[i][0], wai[i][1]);
			}
		}
	}
}

// Print the user input (into one line separated by tabs)
void printInput(wchar_t **input)
{
	int inputI = 0;
	while(input[inputI][0] != 0) {
		if (server) {
			if(inputI != 0) {
				sprintf(o(), "\t");
			}
			sprintf(o(), "%ls", input[inputI]);
		} else {
			if(inputI != 0) {
				printf("\t");
			}
			printf("%ls", input[inputI]);
		}
		++inputI;
	}
}

/**
  * The main core of the algorithm is this method. It evaluats the input word and prints the output.
  *
  * @param wchar_t** input List of inputs. The first or first two are taken. Rest is just bypassed to the output. 
  * @return void
  */
void processInput(wchar_t **input)
{
	long long a, b, d;
	// Word analogy takes 3 words as input. We will remember these words for each iteration.
	wchar_t *wai[IT][4];

	// In categories mode the first word is the category. In standard mode the variable is not used.
	wchar_t *category = input[0];

	// By default the first word is the actual input to lemmatize. In categories mode it is the second word.
	wchar_t *inputWord = categories ? input[1] : input[0];
	int lenIn = wcslen(inputWord);

	// Reverse input word for suffix matching
	wchar_t reversedInputWord[max_size];
	strrev(wcscpy(reversedInputWord, inputWord));

	// Check if input word exists in dictionary. If not exit. The prerequisite is that the word must be in the
	// vector model, otherwise vector operations are possible.
	try {
		wp.at(wstring(inputWord));
	} catch (const out_of_range& r) {
		if(!quiet) {
			if (server) {
				sprintf(o(), "Input out of dictionary word: %ls!\n", inputWord);
			} else {
				printf("Input out of dictionary word: %ls!\n", inputWord);
			}
		}
		if (server) {
			sprintf(o(), "%ls\t0\t%f\n", inputWord, 0.000000);
		} else {
			printf("%ls\t0\t%f\n", inputWord, 0.000000);
		}
		return;
	}

	// For the choosing method R0 the closest words for the input have priority in the
	// reference lexicon. If the R0 is selected, retrieve the closest words based on the vector model (see word2vec and distance()).
	float closestd[N];
	wchar_t closestw[N][max_size];
	if (choosingMethod == 0) {
		wai[0][0] = inputWord;
		distance(vocab, words, size, 1, wai[0], M, closestd, closestw);
	}

	// Find best suffix match precalculation
	int inputDictionaryStart[max_size + 1], inputDictionaryPrefixMem[dictionarySize], inputDictionaryDepth, currentClosestWord;
	char visitedDictionaryWord[dictionarySize];

	memset(inputDictionaryStart, -1, (max_size + 1) * sizeof(int));
	memset(inputDictionaryPrefixMem, 0, dictionarySize * sizeof(int));
	memset(visitedDictionaryWord, 0, dictionarySize * sizeof(char));

	int from = 0, to = dictionarySize;
	if (categories) {
		try {
			// Calculate the suffix only for the category range (which is already sorted)
			pair<int, int> pos = dictionaryCategories.at(wstring(category));
			from = pos.first;
			to = pos.second;
		} catch (const out_of_range& r) {
			// Non-existing category - findBestWord() method will fail and break the algorithm
		}
	}
	
	wchar_t reversedWordTemp[max_size];
	for(a=from; a<to; ++a) {
		// This will calculate suffix, because the first word in dictionary is reversed.
		wchar_t *reversedWord = categories ? strrev(wcscpy(reversedWordTemp, dic(a, 1))) : dic(a, 0);
		inputDictionaryPrefixMem[a] = prefixLength(reversedWord, reversedInputWord);
		// The best word in dictionary for the suffixLength is word at index "a" (if the word isn't already set)
		// Since the dictionary is sorted, there is no better word
		if(inputDictionaryStart[inputDictionaryPrefixMem[a]] == -1) {
			inputDictionaryStart[inputDictionaryPrefixMem[a]] = a;
		}
	}

	// Denotes whether the lemma was found in the dictionary and no more evaluation is needed.
	int solutionFound = 0;

	// List of words outputed from word_analogy() method. These are candidates for correct lemma.
	wchar_t bestw[N][max_size];
	// List of cosine distances for each candidate from 0 (the worst) to 1 (the best).
	float bestd[N];

	// BENCHMARK: There are several lines marked as the BENCHMARK. That code can be used to meassure the execution time of the
	//				findBestWord and word_analogy methods. The tFindBest/tWordAnalogy will hold total time. Divide it by number of
	//				iterations (veriable "a") to see the average duration.
	// BENCHMARK: clock_t begin;
	// BENCHMARK: double tFindBest = 0, tWordAnalogy = 0;

	// List of final unique results with their final weight.
	map<wstring, float> results;

	// We would like to find the match that is as long as the whole input word (which will obviously not happne, but it is a good point to start from)
	inputDictionaryDepth = lenIn;
	currentClosestWord = -1;

	// Clear the list of all chosen reference pairs. This list will be printed in the printPairsWai() method. If there was less iterations than
	// requested, empty words will be printed. Therefore the clearing is important.
	memset(wai, 0, IT * 4 * sizeof(wchar_t *));

	a = 0; // Number of current iteration
	while (a < match_iterations)
	{
		// BENCHMARK: begin = clock();
		if((d = findBestWord(&inputDictionaryDepth, inputDictionaryStart, inputDictionaryPrefixMem, visitedDictionaryWord, &currentClosestWord, closestw, category)) == -1) {
			// All words were already visited. There is no other word to choose. Not all requested iterations were performed, but nevermind.
			break;
		}
		// BENCHMARK: tFindBest += (double)(clock() - begin) / CLOCKS_PER_SEC;

		// If the input word is in the dictionary, the lema is already known. There is no need for other calculation.
		// It is ok to check only the "d" entry, because the longest common suffix has the input word with itself so it should be chosen from
		// the dictionary as the first one (if it is there).
		if (!wcscmp(dic(d, 1), inputWord)) {
			if(!quiet) {
				if (server) {
					sprintf(o(), "Solution found in dictionary: %ls\n", dic(d, 2));
				} else {
					printf("Solution found in dictionary: %ls\n", dic(d, 2));
				}
			}

			printInput(input); // Bypass the input to the output so that it is clear what the original word was
			printPairsWai(wai); // Show the reference pairs used in word_analogy() [only if enabled, see options]

			if (server) {
				sprintf(o(), "\t%ls\t%f\n", dic(d, 2), 1.0);
			} else {
				printf("\t%ls\t%f\n", dic(d, 2), 1.0);
			}
			solutionFound = 1; // Disable further results printing. Will skip to next input case.
			break;
		}

		// Mark current dictionary entry as visited
		visitedDictionaryWord[d] = 1;

		// Prepare the three words to be used in the word_analogy() method. See the comments to the word2vec tool.
		wai[a][0] = dic(d, 1);
		wai[a][1] = dic(d, 2);
		wai[a][2] = inputWord;

		if(!quiet) {
			if (server) {
				sprintf(o(), "WordAnalogy: %ls\t%ls\t%ls\n", wai[a][0], wai[a][1], wai[a][2]);
			} else {
				printf("WordAnalogy: %ls\t%ls\t%ls\n", wai[a][0], wai[a][1], wai[a][2]);
			}
		}

		// BENCHMARK: begin = clock();
		if (!word_analogy(vocab, words, size, 3, wai[a], M, bestd, bestw)) {
			// If some of the words does not exist in the vocabulary, this iteration will be skipped and does not count
			continue;
		}
		// BENCHMARK: tWordAnalogy += (double)(clock() - begin) / CLOCKS_PER_SEC;

		// Analyze the result of word_analogy
		for (b = 0; b < word_analogy_iterations; ++b) {
			if(!quiet) {
				if (server) {
					sprintf(o(), "\t\tMatched: %ls, dist: %f\n", bestw[b], bestd[b]);
				} else {
					printf("\t\tMatched: %ls, dist: %f\n", bestw[b], bestd[b]);
				}
			}

			// For the same word sum together the distances in various iterations
			results[wstring(bestw[b])] += bestd[b];
		}

		++a;
	}

	// For each unique word in the results set calculate the weight based on the strings
	// and total cosine distance.
	for(auto it = results.begin(); it != results.end(); ++it) {
		it->second = match(inputWord, (wchar_t *)it->first.c_str(), lenIn, it->second);
	}

	// If solution has not been found in the dictionary, print it from the results. Otherwise
	// the printing has already been done and can be skipped here.
	if (!solutionFound)
	{
		// Sort the results by their weights.
		multimap<float, wstring> orderedResults = flip_map(results);

		// Print more detailed information about the results
		if(!quiet) {
			if (server) {
				sprintf(o(), "Listing only non-zero results (total: %lu): \n", orderedResults.size());
				sprintf(o(), "\t              Word          Match\n");
			} else {
				printf("Listing only non-zero results (total: %lu): \n", orderedResults.size());
				printf("\t              Word          Match\n");
			}

			for(auto it = orderedResults.rbegin(); it != orderedResults.rend(); ++it) {
				if (it->first > 0) {
					if (server) {
						sprintf(o(), "\t%2lld%15ls%15f\n", a, it->second.c_str(), it->first / (float) a);
					} else {
						printf("\t%2lld%15ls%15f\n", a, it->second.c_str(), it->first / (float) a);
					}
				}
			}

			if (server) {
				sprintf(o(), "Final:\t");
			} else {
				printf("Final:\t");
			}
		}

		printInput(input); // Bypass the input to the output so that it is clear what the original word was
		printPairsWai(wai); // Show the reference pairs used in word_analogy() [only if enabled, see options]

		// Print the top results. Usually the correct lemma should be the first one, however for testing purposes
		// it is possible to print top-k words. See the "showOuputs" variable.
		int shown = 0;
		for(auto it = orderedResults.rbegin(); it != orderedResults.rend(); ++it) {
			if (it->first > 0) {
				if (server) {
					sprintf(o(), "\t%ls\t%f", it->second.c_str(), it->first / (float) a);
				} else {
					printf("\t%ls\t%f", it->second.c_str(), it->first / (float) a);
				}
				if(++shown >= showOutputs) {
					break;
				}
			}
		}

		if (server) {
			sprintf(o(), "\n");
		} else {
			printf("\n");
		}
		if(!quiet) {
			if (server) {
				sprintf(o(), "\n");
			} else {
				printf("\n");
			}
		}
	}
}

/**
  * Split the input by words into the st array. The number of words is in cn value.
  *
  * Try to extract any arguments from the input and the rest is used as the input to the processing.
  * 
  * @param wchar_t* input Input query.
  * @param wchar_t** st List of arguments that can be passwed to the processInput(). Arguments are stripped and already loaded.
  * @return int Number of loaded arguments in the st.
  */
int prepareInput(wchar_t *input, wchar_t **st)
{
	int cn = 0, localCn = 0;
	long long a, b = 0, c = 0;

	while (1) {
		st[cn][b] = input[c], ++b, ++c, st[cn][b] = 0;
		if (input[c] == 0 || input[c] == 13) break;
		if (input[c] == ' ') {
			++cn, b = 0, ++c;
		}
	}
	++cn;

	if (cn > 1) {
		char *chst[W];
		for(a=0;a<cn;++a) {
			chst[a] = (char *) calloc(max_size, sizeof(char));
			wcstombs(chst[a], st[a], wcslen(st[a]));
		}

		loadArguments(cn, chst); // Load some options from the input query

		// Remove empty wstrings from st - that were removed during arguments loading
		for(a=0;a<cn;++a) {
			if (strlen(chst[a])) {
				wcscpy(st[localCn], st[a]);
				++localCn;
			}
			free(chst[a]);
		}
		st[localCn][0] = 0;
	} else {
		localCn = cn;
	}

	return localCn;
}

/**
 * Runs the thread that will read from stdin as long as possible and process the input.
 *
 * @param void* id Thread id.
 * @return void
 */
void *runConsole(void *id)
{
	wchar_t **st; // Input parameters for processing
	wchar_t st1[max_size];
	int running = 1;

	st = (wchar_t **) malloc(W * sizeof(wchar_t *));
	for(int i=0;i<W;++i) {
		st[i] = (wchar_t *) calloc(max_size, sizeof(wchar_t));
	}

	while (running)
	{
		if(!quiet) {
			if (server) {
				sprintf(o(), "Enter the word (EXIT to break), [other args]: ");
			} else {
				printf("Enter the word (EXIT to break), [other args]: ");
			}
			print();
		}

		// Read the input by character
		int a = 0;
		pthread_mutex_lock(&inputLock);
		while (1) {
			st1[a] = fgetwc(stdin);
			if (st1[a] == EOF) {
				pthread_mutex_unlock(&inputLock);
				running = 0; // Stop the processing if end of input or EXIT.
				break;
			}
			if ((st1[a] == '\n') || (a >= max_size - 1)) {
				st1[a] = 0;
				break;
			}
			++a;
		}
		pthread_mutex_unlock(&inputLock);

		// Stop the processing if end of input or EXIT.
		if (!wcscmp(st1, L"EXIT") || !running) break;

		int cn = prepareInput(st1, st);

		// Input was empty
		if (cn == 0) continue;

		processInput(st);
		print();
	}

	// Free the variables
	for(int i=0;i<W;++i) {
		free(st[i]);
	}
	free(st);

	return NULL;
}

void processRequest(int sock)
{
	int n;
	char buffer[MESSAGE];
	wchar_t wbuffer[MESSAGE];
	wchar_t **st; // Input parameters for processing

	bzero(buffer, MESSAGE);
	n = read(sock, buffer, MESSAGE);

	if (n < 0) {
		perror("ERROR reading from socket");
		return;
	}

	st = (wchar_t **) malloc(W * sizeof(wchar_t *));
	for(int i=0;i<W;++i) {
		st[i] = (wchar_t *) calloc(max_size, sizeof(wchar_t));
	}

	mbstowcs(wbuffer, buffer, MESSAGE);
	wbuffer[wcslen(wbuffer)] = 0;
	int cn = prepareInput(wbuffer, st);

	if (cn == 0) {
		sprintf(o(), "Empty input\n");
	} else {
		processInput(st);
	}
	
	n = write(sock, outputBuffer, strlen(outputBuffer) * sizeof(char));
	outputBuffer[0] = 0;

	if (n < 0)	{
		perror("ERROR writing to socket\n");
		return;
	}

	// Free the variables
	for(int i=0;i<W;++i) {
		free(st[i]);
	}
	free(st);
}

/**
 * Close the port when existing on Ctrl-C. This will leave the port to be assigned by another application.
 */
void signal_callback_handler(int signum)
{
   printf("Stop listening.\n");
   close(sockfd);
   exit(signum);
}


/**
 * Run the application in the server mode. Server will open the local socket on given port (see the variable port)
 * and listens for incomming connections. Each connection is handled in separated thread.
 *
 * @return void 
 */
void runServer()
{
	int newsockfd, pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

   	signal(SIGINT, signal_callback_handler);
   	sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	bzero((wchar_t *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
   
	// Now bind the host address using bind() call.
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	printf("Listening on port %d\n", port);

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
   
	while (1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			perror("ERROR on accept");
			continue;
		}

		pid = fork();
		if (pid < 0) {
			perror("ERROR on fork");
			continue;
		}

		if (pid == 0) {
			close(sockfd);
			processRequest(newsockfd);
			close(newsockfd);
			exit(0); // Exits the child process
		} else {
			close(newsockfd);
		}
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	setlocale (LC_ALL, "");

  // Setup sorting array for slovak alphabet including special letters
  wchar_t s[200] = ALPHABET;
  memset(sortAlpha, -1, ALPHA * sizeof(int));
  for(int i = 0; i < (int)wcslen(s); ++i) {
    sortAlpha[(int)s[i]] = i;
  }

	outputBuffer = (char *)malloc(MESSAGE * sizeof(char));

	if (argc == 1) {
		help();
		print();
  	exit(0);
	}

	loadArguments(argc, argv);	

	loadModel(model_file);
	print();

	loadDictionary(dictionary_file);
	print();

	forcePrint();

	if (server) {
		runServer();
	} else {
		// Create multiple threads
		for (int i = 0; i < threads; ++i) {
			int err = pthread_create(&thrs[i], NULL, &runConsole, NULL);
	        if (err != 0) {
	            fprintf(stderr, "Can't create thread %d: [%s]\n", i+1, strerror(err));
	        }
		}
		// Wait for all threads to finish.
		for (int i = 0; i < threads; ++i) {
			pthread_join(thrs[i], NULL);
		}
	}

	return 0;
}
