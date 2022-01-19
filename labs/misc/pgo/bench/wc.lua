--- The Great Computer Language Shootout
-- http://shootout.alioth.debian.org/
--
-- Contributed by Roberto Ierusalimschy
-- Modified by Mike Pall

BUFSIZE = 2^12
local read, len, gsub = io.read, string.len, string.gsub

local cc,lc,wc = 0,0,0
while true do
    local lines, rest = read(BUFSIZE, "*l")
    if lines == nil then break end
    if rest then lines = lines..rest..'\n' end
    cc = cc+len(lines)
    local _,t = gsub(lines, "%S+", "")   -- count words in the line
    wc = wc+t
    _,t = gsub(lines, "\n", "\n")   -- count newlines in the line
    lc = lc+t
end

io.write(lc, " ", wc, " ", cc, "\n")
