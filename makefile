CPP = g++
#The -Ofast might not work with older versions of gcc; in that case, use -O2
CFLAGS = -lm -pthread -Ofast -march=native -Wall -funroll-loops -Wno-unused-result

all: lemma_tool extract_both extract_not_in_lemma extract_not_in_prim

lemma_tool : lemma/lemma.cpp
	$(CPP) lemma/lemma.cpp -o bin/lemma $(CFLAGS)
extract_both : utilities/extract_both.cpp
	$(CPP) utilities/extract_both.cpp -o bin/extract_both $(CFLAGS)
extract_not_in_lemma : utilities/extract_not_in_lemma.cpp
	$(CPP) utilities/extract_not_in_lemma.cpp -o bin/extract_not_in_lemma $(CFLAGS)
extract_not_in_prim : utilities/extract_not_in_prim.cpp
	$(CPP) utilities/extract_not_in_prim.cpp -o bin/extract_not_in_prim $(CFLAGS)

clean:
	rm -rf bin/lemma utilities/extract_both utilities/extract_not_in_lemma utilities/extract_not_in_prim
