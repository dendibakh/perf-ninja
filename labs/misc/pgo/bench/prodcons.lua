-- The Great Computer Language Shootout
-- http://shootout.alioth.debian.org
-- contributed by Isaac Gouy (Lua novice)

n = tonumber(arg[1]) or 1

produced = 0
consumed = 0
buffer = 0

function producer()
   return coroutine.create( 

      function()
         while produced < n do
            produced = produced + 1
            buffer = produced
               -- io.write(buffer)
            coroutine.yield()
         end
      end

   )
end
   

function consumer(p)
   return coroutine.create( 

      function()
         local value = 0
         while consumed < n do
            coroutine.resume(p)
            value = buffer
               -- io.write(" -> ", value, "\n")
            consumed = consumed + 1
         end
      end

   )
end


coroutine.resume( consumer( producer() ))
io.write(produced, " ", consumed, "\n")
