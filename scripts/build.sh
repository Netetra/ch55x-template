#!/bin/bash

# Config
xram_size=0x1800
xram_loc=0x0000
code_size=0xEFFF
dfreq_sys=48000000

# Clean
bash ./scripts/clean.sh

# Compile
find ./src -name "*.c" | xargs -I{} sdcc --std=c23 -c -V -mmcs51 --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -I/ -DFREQ_SYS=$dfreq_sys -o out/ {}
find ./out -name "*.rel" | xargs -i sdcc --std=c23 {} -V -mmcs51 --model-large --xram-size $xram_size --xram-loc $xram_loc --code-size $code_size -I/ -DFREQ_SYS=$dfreq_sys -o ./out/main.ihx

# Convert to hex file
packihx ./out/main.ihx > ./out/main.hex

# Complete!
