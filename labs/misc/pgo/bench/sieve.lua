-- $Id: sieve.lua,v 1.1.1.1 2004-05-19 18:12:27 bfulgham Exp $
-- http://www.bagley.org/~doug/shootout/
--
-- Roberto Ierusalimschy pointed out the for loop is much
-- faster for our purposes here than using a while loop.

function main(num)
    local flags = {}
    for num=num,1,-1 do
	count = 0
	for i=2,8192 do
	    flags[i] = 1
	end
	for i=2,8192 do
	    if flags[i] == 1 then
	        for k=i+i, 8192, i do
		    flags[k] = 0
		end
	        count = count + 1	
	    end
	end
    end
end

NUM = tonumber((arg and arg[1])) or 1
count = 0
main(NUM)
io.write("Count: ", count, "\n")
