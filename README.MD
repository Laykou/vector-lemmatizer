# Vector Lematizer

This project solves the problem of lemmatization utilizing [word2vec](https://code.google.com/p/word2vec/) tool. It was shown that the word vectors capture many linguistic regularities including morpholical relations.

## Installation

- Download this repo
- Run the `make` command. This will add the binary files into the `bin/` folder.
- Download models into `models/` folder.
	- You can train your own models using [word2vec](https://code.google.com/p/word2vec/) tool.

## Usage

Run the `bin/lemma` to see the help:

```
WORD2VEC Lematizator 1.0

Options:
Parameters for training:
	-model <file>
		Use binary data from <file> to build the model. Train the model with word2vec tool.
	-dictionary <file>
		Use text data from <file> to build the dictionary.
	-print_dictionary
		Print the dictionary and exit.
	-print_model
		Print trained model.
	-print_word_analogy_input
		Print the word-analogy input for each testing.
	-quiet
		Print results only. No steps or other processing/execution info.
	-choosing_method <integer> (default: 0)
		Use <integer> to set the method used for choosing the pairs.
			0 - Find semantic closest word (using the distance function). If the word is not in dictionary find by the suffix.
			1 - Find the word in the dictionary only by the longest suffix with the input word.
	-word_analogy <integer> (default: 40)
		Use <integer> to set the number of words to be found by word_analogy method
	-iterations <integer> (default: 20)
		Use <integer> to set the number of iterations for each input word. Can be overwritten by each input
	-closest_count <integer> (default: 40)
		Use <integer> to set the number of the closest words to investigate when using the distance method/strategy.
	-server
		By default the app run in console mode but it can run as server app listening on <port>.
	-port <integer> (default: 40)
		When running as server the app will listen on this port.

```

The easiest way how to start is to run

```
bin/lemma -model mymodel.bin -dictionary dictionaries/dictionary.dict
```

It takes a while to load the model into memory but then are you ready to type your own queries. You can change the arguments any time by passing them into the *stdin* followed by the word you want to lemmatize.

Due to copyright the mymodel.bin is not part of this project. You need to traing your own vector model using `word2vec` tool.

The application is able to communicate via local sockects (by default on port **4332**). Run the lemma with flag **-server**.

```
bin/lemma -model models/mymodel.bin -dictionary dictionaries/dictionary.dict -server
```

Than you can run the PHP server and use the web interface. The web is set to communicate with the server on the default port **4332**.

## Structure

```
bin - binary files (compile them with `make` command)
dictionaries - contains *.dict files
img - images used in this wiki
lemma - the lemmatizer
utilities - small tools performing some tasks during development
tests - files for testing
web - web app

```