-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Mike Pall

local gmatch, gsub, write = string.gmatch, string.gsub, io.write

local seq = io.read("*a")
local ilen = #seq

seq = gsub(seq, ">.-\n", "")
seq = gsub(seq, "\n", "")
local clen = #seq

local variants = {
  "agggtaaa", "tttaccct",
  "[cgt]gggtaaa", "tttaccc[acg]",
  "a[act]ggtaaa", "tttacc[agt]t",
  "ag[act]gtaaa", "tttac[agt]ct",
  "agg[act]taaa", "ttta[agt]cct",
  "aggg[acg]aaa", "ttt[cgt]ccct",
  "agggt[cgt]aa", "tt[acg]accct",
  "agggta[cgt]a", "t[acg]taccct",
  "agggtaa[cgt]", "[acg]ttaccct",
}
local count, prev = 0
for i,pattern in ipairs(variants) do
  for _ in gmatch(seq, pattern) do count = count + 1 end
  if prev then
    write(prev, "|", pattern, " ", count, "\n")
    count = 0
    prev = nil
  else
    prev = pattern
  end
end

local subst = {
  B = "(c|g|t)", D = "(a|g|t)",   H = "(a|c|t)", K = "(g|t)",
  M = "(a|c)",   N = "(a|c|g|t)", R = "(a|g)",   S = "(c|g)",
  V = "(a|c|g)", W = "(a|t)",     Y = "(c|t)",
}
-- Dumbing down forced by "the rules":
for k,v in pairs(subst) do seq = gsub(seq, k, v) end
-- Otherwise you're better off with: seq = gsub(seq, "[BDHKMNRSVWY]", subst)

write("\n", ilen, "\n", clen, "\n", #seq, "\n")
