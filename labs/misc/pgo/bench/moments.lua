-- $Id: moments.lua,v 1.2 2004-05-24 02:23:25 bfulgham Exp $
-- http://www.bagley.org/~doug/shootout/
-- implemented by: Roberto Ierusalimschy

local nums = {}
local n = 0
local sum = 0
while 1 do
  local line = io.read()
  if line == nil then break end
  line = line+0        -- convert line to number
  sum = sum + line
  n = n + 1
  nums[n] = line
end

local mean = sum/n

local average_deviation, variance, skew, kurtosis = 0, 0, 0, 0

for i = 1, n do
  local deviation = nums[i] - mean
  average_deviation = average_deviation + math.abs(deviation)
  variance = variance + deviation^2
  skew = skew + deviation^3
  kurtosis = kurtosis + deviation^4
end

average_deviation = average_deviation/n
variance = variance/(n-1)
local standard_deviation = math.sqrt(variance)
if variance ~= 0 then
  skew = skew / (n * variance * standard_deviation)
  kurtosis = kurtosis/(n * variance * variance) - 3.0
end

table.sort(nums)
local mid = math.floor(n/2)
local median
if math.fmod(n,2) == 1 then
  median = nums[mid+1]
else
  median = (nums[mid] + nums[mid+1])/2
end

io.write(string.format("n:                  %d\n", n))
io.write(string.format("median:             %f\n", median))
io.write(string.format("mean:               %f\n", mean))
io.write(string.format("average_deviation:  %f\n", average_deviation))
io.write(string.format("standard_deviation: %f\n", standard_deviation))
io.write(string.format("variance:           %f\n", variance))
io.write(string.format("skew:               %f\n", skew))
io.write(string.format("kurtosis:           %f\n", kurtosis))
