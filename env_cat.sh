#!/bin/bash

# Usage: env_cat.sh <file> [--stdin]
# Outputs file to stdout, then optionally echoes stdin back

if [[ -z "$1" ]]; then
    echo "Usage: $0 <file> [--stdin]" >&2
    exit 1
fi

FILE="$1"
STDIN_MODE="${2:-}"

# Cat the file first
if [[ -f "$FILE" ]]; then
    cat "$FILE"
else
    echo "Error: File '$FILE' not found" >&2
    exit 1
fi

# If --stdin flag is set, echo stdin back to stdout
if [[ "$STDIN_MODE" == "-i" ]]; then
    cat  # This echoes stdin to stdout
fi