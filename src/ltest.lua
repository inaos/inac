--
-- Copyright (c) 2014-2015, INAOS GmbH
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

require "luamock"
require "luaspec"

local ffi = require("ffi")
local jit = require("jit")

local tins = table.insert

local ltest = {}

local function split(str, pat)
   local t = {}
   local fpat = "(.-)" .. pat
   local last_end = 1
   local s, e, cap = str:find(fpat, 1)
   while s do
      if s ~= 1 or cap ~= "" then
        tins(t,cap)
      end
      last_end = e+1
      s, e, cap = str:find(fpat, last_end)
   end
   if last_end <= #str then
      cap = str:sub(last_end)
      tins(t, cap)
   end
   return t
end

local function os_capture(cmd, raw)
  local f = assert(io.popen(cmd, 'r'))
  local s = assert(f:read('*a'))
  f:close()
  if raw then return s end
  s = string.gsub(s, '^%s+', '')
  s = string.gsub(s, '%s+$', '')
  s = string.gsub(s, '[\n\r]+', ' ')
  return s
end

local function parse_file_list(raw_list, prefix)
  local nllist = split(raw_list, "\n")
  local filtered = {}
  for _,entry in ipairs(nllist) do
    local s,e = string.find(entry, prefix)
    if s and e and s == 1 and entry:sub(entry:len()-2) == "lua" then
      tins(filtered, entry)
    end
  end
  return filtered
end

local function execute_tests_by_filter(dir, filter)
  local ls
  local pwd
  local old_dir
  local tests = 0
  if jit.os == "Windows" then
    old_dir = os_capture("cd")
  else
    old_dir = os_capture("pwd")
  end
  if dir then
    os.execute("cd "..dir)
  end
  if jit.os == "Windows" then
    pwd = os_capture("cd")
    ls = os_capture("dir /A-D /B "..pwd, true)
  else
    pwd = os_capture("pwd")
    ls = os_capture("find . -maxdepth 1 -not -type d | sed s,^./,,", true)
  end
  local spec_list = parse_file_list(ls, filter)
  for _,test in ipairs(spec_list) do
    dofile(test)
    tests = tests + 1
  end
  if dir then
    os.execute("cd "..old_dir)
  end
  return tests
end

ltest.run = function()
  local count
  
  -- somehow handle the bootstrap
  --
  
  count = execute_tests_by_filter(nil, "spec_")

  if count > 0 then
    spec:report(true)
  end
end

return ltest
