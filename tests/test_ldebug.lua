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

local ldebug = require("ldebug")
local ffi = require("ffi")

ffi.cdef[[
ina_rc_t ina_time_sleep(uint64_t msec);
]]

local testldebug = {}

-- We do not want that much output in our tests
print = function() end

testldebug.debug_server = function()
  local socket = require("lsocket")
  local server = socket.bind("127.0.0.1", 8172)

  print("Lua Remote Debugger")
  print("Run the program you wish to debug")

  local client = server:accept()

  local commands = {
    'load test_ldebug_auto.lua', -- load Lua script and start debugger
    'over', 'over', 'step', 'over', 'setb - 15', 'run', 
    'reload', -- reload the same script; breakpoints/watches still stay
    'run',
    {'eval tab.foo', 2, "this should fail"}, -- should display "not ok"
    {'eval tab.bar', 2, "this should work"}, -- should display "ok"
    'exec old_tab = tab', 'exec tab = 2', 'eval tab',
    'exec tab = old_tab', 'eval tab.foo', 'run',
    'eval tab.foo', 'delb auto\\test.lua 15', -- this removes breakpoint set with "setb - 15"
    'setw tab.foo == 32',
    'run', 'eval tab.foo', 'delw 1', 'run'
  }

  local test = 0
  local curfile, curline = '', ''
  for i=1,24 do
    local command = table.remove(commands, 1)
    local expected
    if type(command) == 'table' then
      command, expected, msg = command[1], command[2], (command[3] or '')
    end  

    print("> " .. command)
    local result, line, err = ldebug.handle(command, client)

    if not err and expected then
      local ok = tostring(result) == tostring(expected)
      test = test + 1
      print((not ok and "not " or "") .. "ok " .. test .. (msg and (" - " .. msg) or ""))
      print((not ok and ("#     Failed test (" .. curfile .. " at line " .. curline .. ")" ..
                         "\n#          got: " .. result .. 
                         "\n#     expected: " .. expected) or ""))
    else
      curfile, curline = result, line
    end
  end
end

testldebug.debug_client = function()
  ldebug.loop()
  ffi.C.ina_time_sleep(2000)
end

return testldebug
