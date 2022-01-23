-- The Great Computer Language Shootout
-- http://shootout.alioth.debian.org/
--
-- contributed by Isaac Gouy


local n = tonumber(arg and arg[1]) or 10000000
local partialSum = 0.0

for d = 1,n do partialSum = partialSum + (1.0/d) end

io.write(string.format("%0.9f",partialSum), "\n")
