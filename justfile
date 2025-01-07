baseline := env("BASELINE", "0")
build_dir := "build"
generator := `command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles"`
# Pin to a P-core on hybrid CPUs (check: cat /sys/devices/cpu_core/cpus)
cpu := "0"

defines := if baseline == "1" { "" } else { "-DSOLUTION" } 

flags := "-g -fno-omit-frame-pointer " + defines

default:
    @just --list

# Configure the lab (Release + debug symbols)
[no-cd]
configure:
    cmake -S . -B {{build_dir}} -G "{{generator}}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_FLAGS="{{flags}}" \
        -DCMAKE_CXX_FLAGS="{{flags}}" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build all lab targets
[no-cd]
build: configure
    cmake --build {{build_dir}} --parallel

# Check the solution is correct
[no-cd]
validate: build
    cmake --build {{build_dir}} --target validateLab

# Run the lab benchmark, pinned to one core
[no-cd]
bench: build
    taskset -c {{cpu}} cmake --build {{build_dir}} --target benchmarkLab

# Top-down level 1 (pass e.g. `just topdown TopdownL2` for level 2)
[no-cd]
topdown level="TopdownL1": build
    taskset -c {{cpu}} perf stat -M {{level}} --cputype core ./{{build_dir}}/lab

# perf stat with custom events: just stat -e cpu_core/arith.div_active/
[no-cd]
stat *args: build
    taskset -c {{cpu}} perf stat {{args}} ./{{build_dir}}/lab

# Sample with precise events, then open the annotation: just profile arith.div_active
[no-cd]
profile event="cycles": build
    taskset -c {{cpu}} perf record -e cpu_core/{{event}}/pp -o {{build_dir}}/perf.data ./{{build_dir}}/lab
    perf annotate -i {{build_dir}}/perf.data

# Build baseline and #ifdef SOLUTION variants N times each and compare them
[no-cd]
speedup runs="3" *args:
    mkdir -p {{build_dir}}/speedup && cd {{build_dir}}/speedup && \
    taskset -c {{cpu}} uv run --no-project --with scipy {{justfile_directory()}}/tools/check_speedup.py \
        -lab_path "{{invocation_directory()}}" -num_runs {{runs}} {{args}}

[no-cd]
remarks pass="loop-vectorize|slp-vectorizer":
    cmake -S . -B {{build_dir}}-remarks -G "{{generator}}" -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_CXX_FLAGS="-g {{defines}} -Rpass='{{pass}}' -Rpass-missed='{{pass}}' -Rpass-analysis='{{pass}}' -fsave-optimization-record" \
        > /dev/null
    cmake --build {{build_dir}}-remarks --clean-first 2>&1 | grep 'remark:' | grep -v '/_deps/\|benchmark/' | sort -u

# Remove the build directory
[no-cd]
clean:
    rm -rf {{build_dir}}
