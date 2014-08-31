--
-- Copyright (c) 2014, INAOS GmbH
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--     * Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--     * Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--     * Neither the name of the INAOS GmbH nor the names of its contributors
--       may be used to endorse or promote products derived from this software 
--       without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
-- ARE DISCLAIMED. IN NO EVENT SHALL INAOS GmbH BE LIABLE FOR ANY DIRECT, 
-- INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
-- CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
-- STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
-- ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--

local lsocket = require("lsocket")
local ffi = require("ffi")

ffi.cdef[[
unsigned int sleep(unsigned int seconds);
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
    ffi.C.sleep(3000)
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
