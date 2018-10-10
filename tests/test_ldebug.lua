--
-- Copyright INAOS GmbH, Thalwil, 2014-2018. All rights reserved
--
-- This software is the confidential and proprietary information of INAOS GmbH
-- ("Confidential Information"). You shall not disclose such Confidential
-- Information and shall use it only in accordance with the terms of the
-- license agreement you entered into with INAOS GmbH.
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
