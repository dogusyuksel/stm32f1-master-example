#!/bin/bash

function renode_test() {
    OUTFILE=$(mktemp)

    renode custom_board.resc > "$OUTFILE" 2>&1

    OUTPUT=$(tail -n 10 "$OUTFILE" | tr -d '\r')
    echo "$OUTPUT"

    if echo "$OUTPUT" | grep -q "Hello, STM32"; then
        echo "TEST OK"
        return 0
    else
        echo "TEST NOK"
        return 1
    fi

    return 2
}

renode_test
