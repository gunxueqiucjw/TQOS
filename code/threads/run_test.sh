l="100 101 102 103 104"
for run in $l:
do
    echo $run
    ./nachos -q $run | grep "SUCCESS"
done