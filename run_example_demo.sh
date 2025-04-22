#!/bin/bash

basepwd=$(basename "$PWD")
cp compile_commands.json ..
ninja "$1_demo" && (
  cd "../examples/$1" || exit
  for suffix in ".exe" "-d.exe" "" "-d"; do
    executable="../../$basepwd/bin/$1_demo$suffix"
    if [ -x "$executable" ]; then
      "$executable" "${@:2}"
      exit
    fi
  done
  echo "No matching executable found for $1_demo"
)
