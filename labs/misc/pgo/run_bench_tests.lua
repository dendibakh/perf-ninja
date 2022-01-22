-- run benchmark scripts.
-- This file was significantly simplified for the Performance Ninja project

local write = io.write
local output_fd = io.stdout
local script_print = print
local run_type = (arg and arg[1]) or "bench"

local function run_bench(filename, input, ...)
	local old_arg=arg

	io.input(input)
	for i,val in ipairs(...) do
		-- load script.
		local func=loadfile(filename)
		-- create arg environment variable for script.
		local arg={}
		arg[0]=filename
		arg[1]=val
		local env=getfenv(func)
		env['arg'] = arg
		env['print'] = script_print
        -- call srcipt
		local res= {pcall(func,{val})}
		write("results = ")
		write(unpack(res) and 'true\n' or 'false\n')
		io.flush()
	end
	collectgarbage("collect")
	arg=old_arg
end

local scripts={
 {"../bench/ackermann.lua", io.stdin, {9}},
 {"../bench/ary.lua", io.stdin, {7000}},
 {"../bench/binarytrees.lua", io.stdin, {12}},
 {"../bench/chameneos.lua", io.stdin, {350000}},
 {"../bench/fannkuch.lua", io.stdin, {9}},
 {"../bench/fibo.lua", io.stdin, {31}},
 {"../bench/harmonic.lua", io.stdin, {20000000}},
 {"../bench/hash2.lua", io.stdin, {250}},
 {"../bench/heapsort.lua", io.stdin, {150000}},
 {"../bench/hello.lua", io.stdin, {1}},
 {"../bench/knucleotide.lua", "../input/knucleotide-input20000.txt", {1}},
 {"../bench/matrix.lua", io.stdin, {250}},
 {"../bench/meteor.lua", io.stdin, {0}},
 {"../bench/moments.lua", "../input/moments-input400000.txt", {1}},
 {"../bench/nbody.lua", io.stdin, {70000}},
 {"../bench/nestedloop.lua", io.stdin, {17}},
 {"../bench/nsieve.lua", io.stdin, {7}},
 {"../bench/nsievebits.lua", io.stdin, {6}},
 {"../bench/partialsums.lua", io.stdin, {800000}},
 {"../bench/process.lua", io.stdin, {60000}},
 {"../bench/prodcons.lua", io.stdin, {1000000}},
 {"../bench/random.lua", io.stdin, {3000000}},
 {"../bench/recursive.lua", io.stdin, {5}},
 {"../bench/regexdna.lua", "../input/regexdna-input100000.txt", {1}},
 {"../bench/regexmatch.lua", "../input/regexmatch-input2000.txt", {1}},
 {"../bench/revcomp.lua", "../input/revcomp-input250000.txt", {1}},
 {"../bench/reversefile.lua", "../input/reversefile-input50.txt", {1}},
 {"../bench/sieve.lua", io.stdin, {400}},
 {"../bench/spectralnorm.lua", io.stdin, {300}},
 {"../bench/takfp.lua", io.stdin, {8}},
 {"../bench/wc.lua", "../input/wc-input3000.txt", {1}},
 {"../bench/wordfreq.lua", "../input/wordfreq-input20.txt", {1}}
}

if run_type == "bench" then
	output_fd = io.open("/dev/null","a") -- Linux
	if output_fd == nil then
		output_fd = io.open("NUL","a") -- Windows
	end
	script_print = function(...) end
elseif run_type == "test" then
	if arg[2] then
		output_fd = io.open(arg[2],"w")
	end
end

io.output(output_fd)

for i,v in ipairs(scripts) do
	if v then
		run_bench(unpack(v))
	end
end

io.close(output_fd)

