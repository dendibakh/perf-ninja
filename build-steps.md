These commands must be executed for any given lab to build it
```
cmake -E make_directory build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
cmake --build . --config Release --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```
You can also configure and build a debug version like so:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5 .. -DCMAKE_C_FLAGS="-g" -DCMAKE_CXX_FLAGS="-g"
cmake --build . --config Debug --parallel 8
cmake --build . --target validateLab
cmake --build . --target benchmarkLab
```
To record and view a profile:
```
sudo perf record -g --call-graph dwarf -F 97 ./lab
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
