#!/bin/bash
files=`ls model_z*`
# echo $files

for f in $files; do
    diff -q   $f  output/$f
done
