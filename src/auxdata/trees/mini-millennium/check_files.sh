#!/bin/bash
files=`ls model_z*`
npassed=0
nfiles=0
nfailed=0
for f in $files; do
    ((nfiles++))
    diff -q   $f  output/$f 
    if [[ $? == 0 ]]; then 
        ((npassed++))
    else
        ((nfailed++))
    fi
done
echo "Passed: $npassed (bit-wise identical)"
echo "Failed: $nfailed"

exit $nfailed
