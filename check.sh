#!/usr/bin/env bash
cppcheck --std=c99 --enable=all --inline-suppr --suppress=variableScope -I./include src
