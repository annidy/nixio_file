mio = require "nixio"
for i,v in pairs(mio) do print (i,v) end
f = mio.open('make2.bat')
repeat
  a = f:read(10)
  print(a)
until a == nil
f:close()