-- $Id: sumcol.lua,v 1.3 2004-07-08 03:47:07 bfulgham Exp $
-- http://www.bagley.org/~doug/shootout/

local sum = 0
for line in io.lines() do
    sum = sum + line
end
print(sum)
