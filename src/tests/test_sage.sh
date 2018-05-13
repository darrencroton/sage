#!/bin/bash
cwd=`pwd`
echo cwd is ${cwd}
datadir=src/auxdata/trees/mini-millennium
cd $datadir
if [ ! -f trees_063.7 ]; then
    curl -s https://data-portal.hpc.swin.edu.au/dataset/7bab038b-1d1f-4e79-8cfc-ea171dd1492f/resource/7ff28a50-c401-4a07-9041-13524cbac5c9/download/mini-millennium.tar | tar xvf -
else
    rm -f model_z*
fi
cd $cwd
./sage $datadir/mini-millennium.par
cd $datadir
files=`ls model_z*`
cd $cwd
npassed=0
nfiles=0
nfailed=0
for f in $files; do
    ((nfiles++))
    python src/tests/sagediff.py $datadir/$f $datadir/output/$f 
    if [[ $? == 0 ]]; then 
        ((npassed++))
    else
        ((nfailed++))
    fi
done
echo "Passed: $npassed"
echo "Failed: $nfailed"
exit $nfailed
