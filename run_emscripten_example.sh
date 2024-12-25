#!/bin/bash

ninja $1 && (killall python3 ; emrun ./bin/$1*.html &)
