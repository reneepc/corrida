#!/usr/bin/bash

for i in `seq 1 30`; do
    /usr/bin/time -v ./ep2 1000 30
done;
