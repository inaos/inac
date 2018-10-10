--
-- Copyright INAOS GmbH, Thalwil, 2016-2018. All rights reserved
--
-- This software is the confidential and proprietary information of INAOS GmbH
-- ("Confidential Information"). You shall not disclose such Confidential
-- Information and shall use it only in accordance with the terms of the
-- license agreement you entered into with INAOS GmbH.
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

