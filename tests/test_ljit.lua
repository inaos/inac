local ffi = require("ffi")

ffi.cdef [[
    const char* ina_appname(void);
    ]]

local testljit = {}


testljit.test_appname = function()
    local name = ffi.new("const char[1]")
    name = ffi.C.ina_appname()
    print(name)
    return ffi.string(name)
end

testljit.test_params = function(p1,p2)
    return p2*p1
end

testljit.test = function()
    return 99
end

return testljit