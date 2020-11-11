#!/bin/bash
function run_test {
    n_errors=0

    echo "parse threads: $1"
    for i in $(seq 1 $n_runs);
    do
        ./main.out $1
        if [ $? -ne 0 ]
        then
            # echo "error occurred"
            ((n_errors+=1))
        fi
    done

    echo "errors: $n_errors"
    echo
}

n_runs=5

run_test 1
run_test 2
run_test 3