#!/bin/bash
find ./src | grep pp | xargs clang-format -i
find ./include | grep pp | xargs clang-format -i
find ./include | grep inl | xargs clang-format -i
find ./test | grep pp | xargs clang-format -i
find ./examples | grep cpp | xargs clang-format -i
find ./examples | grep hpp | xargs clang-format -i
find ./examples | grep inl | xargs clang-format -i
