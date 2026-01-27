#!/bin/bash
# compile_run.sh

# Compile the code
g++ -std=c++17 -O2 -Wall tfs.cpp -o tfs

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "Compilation successful! Running the program..."
    ./tfs
else
    echo "Compilation failed. Please check for errors."
fi
