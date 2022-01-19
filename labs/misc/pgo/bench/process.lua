-- The Great Computer Language Shootout
-- http://shootout.alioth.debian.org/
--
-- contributed by Isaac Gouy


local function link(n)
   local message, next = 0

   if n-1 > 0 then
      next = coroutine.create(link) 
      _,message = coroutine.resume(next,n-1)
   end   
   coroutine.yield(message + 1)   
end


local n = tonumber(arg[1]) or 1000
local message = 0
local chain = coroutine.create(link)

_,message = coroutine.resume(chain,n)
io.write(message, "\n")
