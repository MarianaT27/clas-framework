#!/bin/bash

ANALYSIS=./build/analyses/nuclear_transparency
POLARITY=Out
BASE_DIR=/volatile/clas12/ouillon/dipion-rgd
NUM_THREADS=10

declare -A TARGET_DIRS=(
    [LD2]=LD2
    [CxC]=CxC
    [Cu]=CuSn
    [Sn]=CuSn
)

for target in "${!TARGET_DIRS[@]}"; do
    dir=${TARGET_DIRS[$target]}
    echo "=== Running on target: $target (input: $dir) ==="
    $ANALYSIS --target "$target" --polarity "$POLARITY" --input "$BASE_DIR/$dir/" --num_threads "$NUM_THREADS"

    if [[ $? -ne 0 ]]; then
        echo "ERROR: $target failed" >&2
    fi
done