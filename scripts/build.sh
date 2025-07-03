#!/bin/bash

# Config
xram_size=0x0800
xram_loc=0x0600
code_size=0xEFFF
dfreq_sys=48000000

# Clean
rm ./out/*

# Compile
find ./src -name "*.c" | xargs -I{} sdcc -c -V -mmcs51 --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -I/ -DFREQ_SYS=$dfreq_sys -o out/ {}
find ./out -name "*.rel" | xargs -i sdcc {} -V -mmcs51 --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -I/ -DFREQ_SYS=$dfreq_sys -o ./out/main.ihx

# Convert to hex file
packihx ./out/main.ihx > ./out/main.hex

# Complete!
