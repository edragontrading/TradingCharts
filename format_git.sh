#!/bin/bash

# Format only git modified or staged C++ files
for file in $(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp|cc|hh|c|h)$'); do
    if [ -f "$file" ]; then
        clang-format -i "$file"
        echo "Formatted: $file"
    fi
done
