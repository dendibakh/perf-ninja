--- The Great Computer Lanuage Shootout
--  http://shootout.alioth.debian.org
--
-- Contributed by ???
-- Modified by Mike Pall (email withheld by request)
-- Submitted by Matthew Burke <shooutout@bluedino.net>
--
local function Ack(m, n)
  if m == 0 then return n+1 end
  if n == 0 then return Ack(m-1, 1) end
  return Ack(m-1, Ack(m, n-1))
end

local N = tonumber(arg and arg[1]) or 1
io.write("Ack(3,", N ,"): ", Ack(3,N), "\n")
