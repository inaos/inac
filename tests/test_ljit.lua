local ffi = require("ffi")

ffi.cdef [[
    const char* ina_app_get_name(void);
    ]]

local testljit = {}

testljit.test_app_get_name = function()
    local name = ffi.C.ina_app_get_name()
    return ffi.string(name);
end

testljit.test_params = function(p1,p2)
    return p2*p1
end

testljit.test = function()
    return 99
end

return testljit