#!/bin/bash

basepwd=$(basename "$PWD")
cp compile_commands.json ..
ninja "$1" && (
  cd "../examples/$1" || exit
  for suffix in ".exe" "-d.exe" "" "-d"; do
    executable="../../$basepwd/bin/$1$suffix"
    if [ -x "$executable" ]; then
      "$executable"
      exit
    fi
  done
  echo "No matching executable found for $1"
)
