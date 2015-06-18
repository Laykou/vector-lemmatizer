# This file contains scripts to prepare testcases from the list of all annotated words.
# Extracts from which the testcases are prepared cannot be part of this CD because of copyright protection.

# Files
# prim-6.1-public-all-word-frequency.txt - contains list of words in Prim corpora and their frequencies
# lemmaformtag.txt - Annotated data from JULS

# Find lemmaformtag tokens for all frequency words
../utilities/extract_both_frequency prim-6.1-public-all-word-frequency.txt ../lemmaformtag.txt

# Not in dictionary
../utilities/extract_not_in_prim_frequency <(awk '{print $1}' < ../dictionaries/nouns.adjectives/dictionary.genders.plus.manual.all.dict) extract.prim.word.frequency.lemmaformtag.all.txt > extract.prim.word.frequency.lemmaformtag.all.not.in.nouns.adjectives.dictionary.txt




# Extract only first -1000 lowercased nouns from the extract - output in format: <category> <word> <lemma> <frequency>.

### Extract top 1000 nouns only
dictionary="extract.prim.word.frequency.lemmaformtag.not.in.categories.dictionary.txt" # Extract lines from this file
count=1000 # Number of generated input cases
folder="../../tests/scenario/frequencies/top.1000" # Folter to put the input files in
pattern='SS...$' # Filter only Substantives
outputSpec1='top' # Beginning of the input filename
outputSpec2='nouns' # Specify the output type (middle of the input filename)
method="cat" # cat command means that the input will be bypassed to output

### Extract top 1000 nouns and ajectives only
dictionary="extract.prim.word.frequency.lemmaformtag.all.not.in.nouns.adjectives.dictionary.txt" # Extract lines from this file
count=1000 # Number of generated input cases
folder="../../tests/scenario/frequencies/random.1000.nouns.adjectives" # Folter to put the input files in
pattern="'A[AF]...x\$|SS...\$'" # Filter only substantives and adjectives
outputSpec1='random'
outputSpec2='nouns.adjectives'
method="sort -R" # Randomize rows

### Extract top 1000 nouns and ajectives only
dictionary="extract.prim.word.frequency.lemmaformtag.not.in.categories.dictionary.txt" # Extract lines from this file
count=1000 # Number of generated input cases
folder="../../tests/scenario/frequencies/random.1000.nouns" # Folter to put the input files in
pattern="'SS...\$'" # Filter only substantives and adjectives
outputSpec1='random'
outputSpec2='nouns'
method="sort -R" # Randomize rows

cat ${dictionary} | egrep 'A[AF]...x$|SS...$' | grep -v '5$' | grep -v '5.$' | awk '{if(substr($2,1,1) >= "a" && substr($2,1,1) <= "z" && $2!=$3) print $1"\t"$2"\t"$3"\t"$4}' | ${method} | head -${count} > temp

# Category: Number
cat temp | awk '{print substr($4,4,1)" "$3" "$2" "$1}' > ${folder}/${outputSpec1}.${count}.${outputSpec2}.number.in
# Category: Categories
cat temp | awk '{print substr($4,3,3)" "$3" "$2" "$1}' | sed 's/^i/m/g' > ${folder}/${outputSpec1}.${count}.${outputSpec2}.categories.in
# Category: None (All)
cat temp | awk '{print $3" "$2" "$1}' > ${folder}/${outputSpec1}.${count}.${outputSpec2}.all.in
# Category: Gender
cat temp | awk '{print substr($4,3,1)" "$3" "$2" "$1}' | sed 's/^i/m/g' > ${folder}/${outputSpec1}.${count}.${outputSpec2}.gender.in
# Category: Case
cat temp | awk '{print substr($4,5,1)" "$3" "$2" "$1}' | head -${count} > ${folder}/${outputSpec1}.${count}.${outputSpec2}.case.in
# Category: Number.case
cat temp | awk '{print substr($4,4,2)" "$3" "$2" "$1}' | head -${count} > ${folder}/${outputSpec1}.${count}.${outputSpec2}.number.case.in
