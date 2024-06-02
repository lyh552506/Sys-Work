#!/bin/bash

if [[ $1 == "-load" ]]; then
    make
    insmod hellokernel.ko
    dmesg
elif [[ $1 == "-deload" ]]; then
    rmmod hellokernel
    dmesg
else
    echo "Invalid argument. Usage: ./run.sh [-load | -deload]"
fi