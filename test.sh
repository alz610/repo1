#!/bin/bash
function run_test {
    n_errors=0

    echo "========================"
    echo "runs: $n_runs"
    echo "parse threads: $1"

    for i in $(seq 1 $n_runs);
    do
        echo "------------------------"
        echo "run"
        echo "------------------------"
        ./main.out $1
        if [ $? -ne 0 ]
        then
            # echo "error occurred"
            ((n_errors+=1))
        fi
    done

    echo "------------------------"
    echo "errors: $n_errors"
}

n_runs=5

run_test 1
run_test 2
run_test 3