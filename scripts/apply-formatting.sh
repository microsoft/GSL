#!/bin/bash

set -euo pipefail

cf=clang-format-$(grep -i 'CLANG_VERSION:' .github/workflows/clang-format.yml | sed -E 's/.*"([^"]+)".*/\1/')
readonly cf

if ! "${cf}" -version; then
	echo "[ERROR] clang-format not found. Please install it using: sudo apt install ${cf}"
	exit 1
fi

{
	find include/gsl -type f
	find tests -type f \( -name '*.cpp' -o -name '*.h' \)
} | xargs "${cf}" -i --assume-filename=x.cpp --verbose

find scripts -type f -name '*.sh' -print -exec shfmt -w {} \;
