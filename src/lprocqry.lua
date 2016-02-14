--
-- Copyright (c) 2016, INAOS GmbH
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

local popen = io.popen

local proc_parse = {}

local meta_proc_info = {
  __index = {
    get_cmd = function(i)
      return i._cmd
    end,
    get_used_mem = function(i)
      return i._rss
    end,
    get_num_threads = function(i)
      return i._threads
    end,
  }
}

local function _query_process(needle)
  local f = assert(popen("ls /proc"))
  for line in f:lines() do
    if tonumber(line) then
      local bail = false
      local cmd_p = "cat /proc/"..line.."/cmdline"
      local status_p = "cat /proc/"..line.."/status"
      local cmd_f = assert(popen(cmd_p))
      local cmd = cmd_f:read("*a")
      if cmd:find(needle) then
        local info = {}
        info._cmd = cmd
        local status_f = assert(popen(status_p))
        for sl in status_f:lines() do
          if sl:find("VmRSS") then
            local rss = sl:match("%w+%:%s+(%d+)%s%w+")
            info._rss = tonumber(rss)*1024
          end
          if sl:find("Threads") then
            local threads = sl:match("%w+%:%s+(%d+)")
            info._threads = threads
          end
        end
        bail = true
        status_f:close()
      end
      cmd_f:close()
      if bail then
        f:close()
        return true
      end
    end
  end
  f:close()
  return false
end

proc_parse.query = function(needle)
  if jit.os() ~= "Linux" then
    return nil
  end
  local i = _query_process(needle)
  setmetatable(i, meta_proc_info)
  return i
end

return proc_parse

