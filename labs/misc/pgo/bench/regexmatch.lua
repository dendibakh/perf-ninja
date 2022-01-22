-- $Id: regexmatch.lua,v 1.2 2004-06-12 16:19:44 bfulgham Exp $
-- http://shootout.alioth.debian.org
-- contributed by Roberto Ierusalimschy

local text = io.read("*a")

-- make sure text does not start with a number
text = "\n" .. text

-- pattern is: not a digit, optional (, 3 digits, optional ),
-- space, 3 digits, space or hyphen, 4 digits, not a digit
local pattern = "%D(%(?)(%d%d%d)(%)?) (%d%d%d)[- ](%d%d%d%d)%f[%D]"

local N = tonumber((arg and arg[1])) or 1
local count = 0
for i=N,1,-1 do
  for open,area,close,exch,digits in string.gmatch(text, pattern) do
      if (open == '(') == (close == ')') then
        local tel = "("..area..") "..exch.."-"..digits
        if i == 1 then
          count = count+1
          io.write(count, ": ", tel, "\n")
        end
      end
    end
end


