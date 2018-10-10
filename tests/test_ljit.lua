--
-- Copyright INAOS GmbH, Thalwil, 2013-2018. All rights reserved
--
-- This software is the confidential and proprietary information of INAOS GmbH
-- ("Confidential Information"). You shall not disclose such Confidential
-- Information and shall use it only in accordance with the terms of the
-- license agreement you entered into with INAOS GmbH.
--
local ffi = require("ffi")

ffi.cdef [[
    const char* ina_app_get_name(void);
    ]]

local testljit = {}

testljit.test_app_get_name = function()
    local name = ffi.C.ina_app_get_name()
    return name
end

testljit.test_params = function(p1,p2)
    return p2*p1
end

testljit.test = function()
    return 99
end

testljit.test_boolean_true = function()
    return true
end

testljit.test_boolean_false = function()
    return false
end

return testljit