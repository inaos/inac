--
-- Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
--
-- This software is the confidential and proprietary information of INAOS GmbH
-- ("Confidential Information"). You shall not disclose such Confidential
-- Information and shall use it only in accordance with the terms of the
-- license agreement you entered into with INAOS GmbH.
--

local lsocket = require("lsocket")
local ffi = require("ffi")

ffi.cdef[[
typedef uint64_t time_t;
ina_rc_t ina_time_sleep(time_t msec);
]]

-- We do not want that much output in our tests
print = function() end

local testlsocket = {}

testlsocket.echo_server = function(host, port)
  local s = lsocket.bind(host, port)
  
  while true do
    local c = s:accept()
    local buf = c:receive()
    print("Server: "..buf)
    c:send(buf)
    ffi.C.ina_time_sleep(3000)
  end
  
  return 0
end

testlsocket.echo_client = function(host, port)
  print("Connecting to: "..host..":"..port)
  local client = lsocket.connect(host, port)
  print("Connected to server!")
  
  client:send("hello lsocket")
  
  local answer = client:receive()
  
  print("Answer: "..answer)
  
  client:close()
  
  return 0
end

return testlsocket
