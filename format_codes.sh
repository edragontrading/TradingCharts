#!/bin/bash

# Format all .cpp, .h, .hpp files in the current directory and subdirectories
find ./src/ -regex '.*\.\(cpp\|h\|hpp\|cxx\|cc\|c\)' -exec clang-format -i {} +
find ./examples/ -regex '.*\.\(cpp\|h\|hpp\|cxx\|cc\|c\)' -exec clang-format -i {} +

echo "Formatting complete!"
