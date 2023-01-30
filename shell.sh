#!/bin/zsh
g++ -std=c++11 parser.cpp -o parser; ./parser -f > output-files/Shakespeare.txt
python3 indi_parse.py
cd gedcom-parser
ng serve