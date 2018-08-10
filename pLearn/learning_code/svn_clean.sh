#!/bin/bash

#remove extranous files from results
for filename in */*.log; do
    echo "$filename"
    svn rm "$filename"
done

for filename in results/simulation*; do
    svn rm "$filename"
done

    
