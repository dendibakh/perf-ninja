--- The Great Computer Language Shootout
-- http://shootout.alioth.debian.org/
--
-- implemented by: Isaac Gouy
-- Modified by Mike Pall

local function Tak(x,y,z)
	if y>=x then return z end
	return Tak( Tak(x-1.0,y,z), Tak(y-1.0,z,x), Tak(z-1.0,x,y) )
end
	
n = tonumber(arg and arg[1]) or 1
io.write( string.format("%.1f\n", Tak(n*3.0,n*2.0,n*1.0)) )

