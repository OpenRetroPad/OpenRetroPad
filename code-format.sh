#!/bin/sh

find src test -type f \( -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' \) -print0 | xargs -0 clang-format -style=file -i -fallback-style=none

