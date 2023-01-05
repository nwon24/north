#!/bin/sh

NORTH=./north

run_cmd_test() {
    expected=$(cat "$2")
    actual=$(($1) 2>&1)
    if [ "$expected" = "$actual" ]; then
	return 0
    else
	echo "Test failed."
	printf "Expected\n%s but got\n%s" "$expected" "$actual"
	return 1
    fi

}

run_test() {
    printf "Running simulation test on %s\n" "$1"
    cmd="$NORTH -Istdlib -s $1"
    run_cmd_test "$cmd" "$2"
    printf "Running compilation test on %s\n" "$1"
    cmd="$NORTH -Istdlib -rc $1"
    run_cmd_test "$cmd" "$2"
}


#run_test "./tests/001-arithmetic/001-arithmetic.nth" "./tests/001-arithmetic/001-arithmetic.txt"

success=0

for dir in ./tests/*; do
    run_test "$dir/"*.nth "$dir/"*.txt
    if [ "$?" -eq 0 ]; then
	success=$(($success+1))
    fi
done

printf -- "---------------------------------------------------\n"
printf "TEST RESULTS\n"
printf "Passed: %s\n" "$success"
printf -- "---------------------------------------------------\n"

