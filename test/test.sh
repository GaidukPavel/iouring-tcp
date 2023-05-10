#!/bin/bash

echo -n "" > output

{ time nc -q 0 127.0.0.1 8888 < test_entry1 ; } 2>> output &

{ time nc -q 0 127.0.0.1 8888 < test_entry2  ; } 2>> output &

sleep 8

cat output
