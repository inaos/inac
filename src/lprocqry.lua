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

local ffi = require("ffi")
local jit = require("jit")
local iopen = io.open

ffi.cdef[[
    typedef unsigned long long ino_t;
    typedef long long off_t;
    typedef struct __dirstream DIR;
    struct dirent {
        ino_t d_ino;             /* inode number */
        off_t d_off;             /* not an offset; see NOTES */
        unsigned short d_reclen; /* length of this record */
        unsigned char d_type;    /* type of file; not supported
                                    by all filesystem types */
        char d_name[256];        /* filename */
    };
    
    DIR *opendir(const char *name);
    struct dirent *readdir(DIR *dirp);
    int closedir(DIR *dirp);
]]

local proc_parse = {}

local function _query_process(needle)
  local dir = ffi.C.opendir("/proc/")
  if dir == NULL then
    return nil
  end
  local dp = NULL
  repeat
    dp = ffi.C.readdir(dir)
    if dp ~= NULL then
      local fname = tonumber(ffi.string(dp.d_name))
      if fname then
        local cmdline = "/proc/"..fname.."/cmdline"
        local fcmdline = iopen(cmdline)
        if fcmdline then
          local cmd = fcmdline:read("*a")
          if cmd:find(needle, 1, true) then
            local info = {}
            info._cmd = cmd
            local fstatus = assert(iopen("/proc/"..fname.."/status"))
            for sl in fstatus:lines() do
              if sl:find("VmRSS") then
                local rss = sl:match("%w+%:%s+(%d+)%s%w+")
                info._rss = tonumber(rss)*1024
              end
              if sl:find("Threads") then
                local threads = sl:match("%w+%:%s+(%d+)")
                info._threads = threads
              end
            end
            fstatus:close()
            fcmdline:close()
            ffi.C.closedir(dir)
            return info
          end
          fcmdline:close()
        end
      end
    end
  until dp == NULL
  ffi.C.closedir(dir)
  return nil
end

proc_parse.query = function(needle)
  if jit.os ~= "Linux" then
    return nil
  end
  return _query_process(needle)
end

return proc_parse

