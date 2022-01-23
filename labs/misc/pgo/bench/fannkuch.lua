-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Mike Pall

local function fannkuch(n)
  local p, q, s, odd, check, maxflips = {}, {}, {}, true, 0, 0
  for i=1,n do p[i] = i; s[i] = i end
  repeat
    -- Print max. 30 permutations.
    if check < 30 then
      if not p[n] then return maxflips end	-- Catch n = 0, 1, 2.
      for i=1,n do io.write(p[i]) end; io.write("\n")
      check = check + 1
    end
    -- Copy and flip.
    local q1 = p[1]				-- Cache 1st element.
    if p[n] ~= n and q1 ~= 1 then		-- Avoid useless work.
      for i=2,n do q[i] = p[i] end		-- Work on a copy.
      for flips=1,1000000 do			-- Flip ...
	local j = q1 - 1
	for i=2,q1/2 do q[i], q[j] = q[j], q[i]; j = j - 1 end
	local qq = q[q1]
	if qq == 1 then				-- ... until 1st element is 1.
	  if flips > maxflips then maxflips = flips end -- New maximum?
	  break
	end
	q[q1] = q1
	q1 = qq
      end
    end
    -- Permute.
    if odd then
      p[2], p[1] = p[1], p[2]; odd = false	-- Rotate 1<-2.
    else
      p[2], p[3] = p[3], p[2]; odd = true	-- Rotate 1<-2 and 1<-2<-3.
      for i=3,n do
	local sx = s[i]
	if sx ~= 1 then sx = sx - 1; s[i] = sx; break end
	if i == n then return maxflips end	-- Out of permutations.
	s[i] = i
	-- Rotate 1<-...<-i+1.
	local t = p[1]; for j=1,i do p[j] = p[j+1] end; p[i+1] = t
      end
    end
  until false
end

local n = tonumber(arg and arg[1]) or 1
io.write("Pfannkuchen(", n, ") = ", fannkuch(n), "\n")
