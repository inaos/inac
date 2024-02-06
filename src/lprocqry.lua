--
-- Copyright 2016-2020 INAOS GmbH, Thalwil
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
        local fcmdline = iopen(cmdline, "rb")
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

