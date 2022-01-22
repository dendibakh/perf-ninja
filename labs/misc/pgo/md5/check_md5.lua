package.path = package.path .. ";../md5/?.lua"
local md5 = require 'md5'

local outputFileName = arg and arg[1]
local outputFile = io.open(outputFileName, "r")
io.input(outputFile)
local output = io.read()
local md5_as_hex   = md5.sumhexa(output)

output_md5 = io.open(outputFileName .. '.md5',"w")
io.output(output_md5)

io.write(md5_as_hex)
io.close(outputFile)
io.close(output_md5)
