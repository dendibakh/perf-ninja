#!/usr/bin/lua
-- $Id: heapsort.lua,v 1.2 2004-06-12 16:19:43 bfulgham Exp $
-- http://shootout.alioth.debian.org
-- contributed by Roberto Ierusalimschy

local IM = 139968
local IA =   3877
local IC =  29573

local LAST = 42
function gen_random(max)
	LAST = math.fmod((LAST * IA + IC), IM)
	return( (max * LAST) / IM )
end

function heapsort(n, ra)
	local j, i, rra
	local l = math.floor(n/2) + 1
	local ir = n;
	while 1 do
	if l > 1 then
		l = l - 1
		rra = ra[l]
	else
		rra = ra[ir]
		ra[ir] = ra[1]
		ir = ir - 1
		if (ir == 1) then
			ra[1] = rra
			return
		end
	end
	i = l
	j = l * 2
	while j <= ir do
		if (j < ir) and (ra[j] < ra[j+1]) then
			j = j + 1
		end
		if rra < ra[j] then
			ra[i] = ra[j]
			i = j
			j = j + i
		else
			j = ir + 1
		end
	end
	ra[i] = rra
	end
end

local ary = {}
local N = (tonumber((arg and arg[1])) or 5)

for i=1, N do
	ary[i] = gen_random(1.0)
end

heapsort(N, ary)

io.write(string.format("%0.10f\n", ary[N]))
