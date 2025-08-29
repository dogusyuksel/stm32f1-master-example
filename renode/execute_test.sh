#!/bin/bash

function renode_test() {
    OUTFILE=$(mktemp)

    renode custom_board_test.resc > "$OUTFILE" 2>&1

    OUTPUT=$(cat "$OUTFILE" | tr -d '\r')
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
