#!/bin/bash

# Run this command with the type of the testing: "./run categories" all "./run all"

# All commands that will be fired to test the dictionaries

# 1000 vs 10000
# all, case, categories, gender, number.case, number

TYPE=$1

# Iterations
it=3
# Show outputs - number of first lemma candidates to output
so=5

# type is retrieved from the attribute.
types=( "all" "case" "categories" "gender" "number.case" "number" )

# for TYPE in "${types[@]}"; do
for inputSize in 1000; do
for ch in 1; do
for it in 1 3 5; do
for we in 0 1 2 3; do

[[ $TYPE != "all" ]] && attr="-categories" || attr=""

echo "Running: top.${inputSize}.nouns.${TYPE}.ch.${ch}.it.${it}.so.${so}.we.${we}.out"
echo "../../lemma -model ../../models/prim-6.1-public-all.shuffled.300cbow.bin -dictionary ../../dictionaries/dictionary.genders.plus.all.${TYPE}.dict -quiet -choosing_method ${ch} -iterations ${it} -show_outputs ${so} -weighting ${we} ${attr} < top.${inputSize}.nouns.${TYPE}.in > top.${inputSize}.nouns.${TYPE}.ch.${ch}.it.${it}.so.${so}.we.${we}.out"

# done
done
done
done
done
