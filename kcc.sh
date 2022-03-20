#!/bin/sh

# build kcc
echo "build kcc"
make kcc
echo "kcc build done\n"

# sh kcc.sh -o <output-file> <input-file>
if [ $# -ne 3 ] || [ "$1" != "-o" ]; then
    echo "\e[31merror:\e[m illegal format"
    echo "usage: \"sh kcc.sh -o <output-file> <input-file>\""
    exit 1
fi

# preprocess & compile
gcc -o- -E -P -C "$3" | ./kcc -o "$2.s" - 

if [ $? -ne 0 ]; then
    echo "\e[31merror:\e[m proprocess or compile error"
    exit 1
fi

# assemble
gcc -o "$2" "$2.s"

if [ $? -ne 0 ]; then
    echo "\e[31merror:\e[m assemble error"
    exit 1
fi

echo "OK"
