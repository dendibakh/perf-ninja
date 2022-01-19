-- $Id: nestedloop.lua,v 1.1.1.1 2004-05-19 18:10:56 bfulgham Exp $
-- http://www.bagley.org/~doug/shootout/

local n = tonumber((arg and arg[1]) or 1)
local x = 0
for a=1,n do
    for b=1,n do
	for c=1,n do
	    for d=1,n do
		for e=1,n do
		    for f=1,n do
			x = x + 1
		    end
		end
	    end
	end
    end
end
io.write(x,"\n")
