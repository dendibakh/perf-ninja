These commands must be executed for any given lab to build it
```
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_C_FLAGS="-g" -DCMAKE_CXX_FLAGS="-g" ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```

You can also configure and build a debug version like so:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. -DCMAKE_C_FLAGS="-g --fno-omit-frame-pointer" -DCMAKE_CXX_FLAGS="-g --fno-omit-frame-pointer"
cmake --build . --config Debug --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```
To record and view a profile:
```
sudo perf record --call-graph fp -g --call-graph dwarf -F 97 ./lab
sudo perf report
```
You may need to install linux-tools-aws

There seems to be a bug in the lightning AI studio (e.g. the VM that this is running on) where
the perf report is written with the wrong binary path. 
For example, perf may record a binary path as `/content/perf-ninja/labs/misc/warmup/build/lab` but 
the actual path is `/teamspace/studios/this_studio/perf-ninja/labs/misc/warmup/build/lab`. 
This path mismatch can preventing perf from resolving the symbols.
In this example...

## Fix A
Make a symlink via:
```
sudo mkdir -p /content/perf-ninja/labs/misc/warmup/build && sudo ln -s /teamspace/studios/this_studio/perf-ninja/labs/misc/warmup/build/lab /content/perf-ninja/labs/misc/warmup/build/lab
```
## Fix B 
Use perf buildid-cache:
```
sudo perf buildid-cache -a /teamspace/studios/this_studio/perf-ninja/labs/misc/warmup/build/lab
```

This was confirmed by first ensuring that the binary was written with debug symbols, via:
```
⚡ main ~/perf-ninja/labs/misc/warmup/build readelf -S /teamspace/studios/this_studio/perf-ninja/labs/misc/warmup/build/lab | grep debug
  [30] .debug_aranges    PROGBITS         0000000000000000  00066453
  [31] .debug_info       PROGBITS         0000000000000000  00066513
  [32] .debug_abbrev     PROGBITS         0000000000000000  00085dae
  [33] .debug_line       PROGBITS         0000000000000000  000870ba
  [34] .debug_str        PROGBITS         0000000000000000  00087ef6
  [35] .debug_loc        PROGBITS         0000000000000000  000b6e65
  [36] .debug_ranges     PROGBITS         0000000000000000  000b86ff
```
Then checking the perf script and seeing wrong path:
```
⚡ main ~/perf-ninja/labs/misc/warmup/build sudo perf script | head -20
lab 49903  3478.651109:   10309278 cpu-clock:pppH: 
            55555555ed43 [unknown] (/content/perf-ninja/labs/misc/warmup/build/lab)

lab 49903  3478.661427:   10309278 cpu-clock:pppH: 
            55555555ec96 [unknown] (/content/perf-ninja/labs/misc/warmup/build/lab)

lab 49903  3478.671736:   10309278 cpu-clock:pppH: 
            55555555ec96 [unknown] (/content/perf-ninja/labs/misc/warmup/build/lab)
```