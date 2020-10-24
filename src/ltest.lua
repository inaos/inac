--
-- Copyright 2014-2020 INAOS GmbH, Thalwil
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--   http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
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
