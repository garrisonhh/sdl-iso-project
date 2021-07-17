#!/usr/bin/env bash
cppcheck --std=c99 --enable=all --suppress=variableScope -I./include src
