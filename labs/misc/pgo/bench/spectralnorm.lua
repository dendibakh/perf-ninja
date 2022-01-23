-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Isaac Gouy, tuned by Mike Pall

local function A(i, j)
  local ij = i+j
  return 1.0 / (ij * (ij+1) * 0.5 + i+1)
end

local function Av(n, x, y)
  for i = 0,n-1 do
    local a = 0
    for j = 0,n-1 do a = a + A(i,j) * x[j] end
    y[i] = a
  end
end

local function Atv(n, x, y)
  for i = 0,n-1 do
    local a = 0
    for j = 0,n-1 do a = a + A(j,i) * x[j] end
    y[i] = a
  end
end

local function AtAv(n, x, y, t)
  Av(n, x, t)
  Atv(n, t, y)
end


local n = tonumber(arg and arg[1]) or 100
local u, v, t = {}, {}, {}
for i = 0,n-1 do u[i] = 1 end

for i = 1,10 do AtAv(n, u, v, t) AtAv(n, v, u, t) end

local vBv, vv = 0, 0
for i = 0,n-1 do
  local ui, vi = u[i], v[i]
  vBv = vBv + ui*vi
  vv = vv + vi*vi
end

io.write(string.format("%0.9f\n", math.sqrt(vBv / vv)))

