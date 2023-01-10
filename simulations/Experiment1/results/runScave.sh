#!/bin/bash
find . -name "*.csv" -type f -delete

opp_scavetool export -o Exp1N2.csv  -F CSV-R N2-#0.vec

