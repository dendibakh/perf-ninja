-- $Id: matrix.lua,v 1.2 2004-07-04 07:29:51 bfulgham Exp $
-- http://shootout.alioth.debian.org/
-- with help from Roberto Ierusalimschy

local n = tonumber((arg and arg[1]) or 1)

local size = 30

function mkmatrix(rows, cols)
    local count = 1
    local mx = {}
    for i=1,rows do
	local row = {}
	for j=1,cols do
	    row[j] = count
	    count = count + 1
	end
	mx[i] = row
    end
    return(mx)
end

function mmult(rows, cols, m1, m2)
    local m3 = {}
    for i=1,rows do
        local m3i = {}
        m3[i] = m3i
        local m1i = m1[i]              -- "cache" m1[i]
        for j=1,cols do
            local rowj = 0
            for k=1,cols do
                rowj = rowj + m1i[k] * m2[k][j]
            end
            m3i[j] = rowj
        end
    end
    return(m3)
end

local m1 = mkmatrix(size, size)
local m2 = mkmatrix(size, size)
for i=1,n do
    mm = mmult(size, size, m1, m2)
end
io.write(string.format("%d %d %d %d\n", mm[1][1], mm[3][4], mm[4][3], mm[5][5]))
