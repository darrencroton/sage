#!/bin/bash
cwd=`pwd`
datadir=../auxdata/trees/mini-millennium
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"/$datadir
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
cd "$cwd"
exit $nfailed
