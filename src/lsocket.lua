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

-- NOTES:
-- * This is a very limited and simplified wrapper for lua-socket
-- * Its been stripped down to only support mobdebug
-- * We might enhance it in the future

local ffi = require("ffi")

local sw = {}

local _MODE_BLOCKING = 0
local _MODE_NON_BLOCKING = 1

local INA_EAGAIN = 22

ffi.cdef[[
typedef uint32_t ina_rc_t;
/*
 *
 */
ina_rc_t ina_net_tcp_server(int *fd, int port, const char *bindaddr);

/*
 *
 */
ina_rc_t ina_net_tcp_accept(int *fd, int sfd, char *ip, int *port);

/*
 *
 */
ina_rc_t ina_net_tcp_connect(int *fd, const char *addr, int port, int timeout_sec);

/*
 *
 */
ina_rc_t ina_net_read(int fd, unsigned char *buf, int nb, int* nb_read);

/*
 *
 */
ina_rc_t ina_net_write(int fd, const unsigned char *buf, int nb, int* nb_write);

/*
 *
 */
ina_rc_t ina_net_nonblock(int fd);

/*
 *
 */
ina_rc_t ina_net_block(int fd);

/*
 *
 */
ina_rc_t ina_net_set_read_timeout(int fd, int msec);

/*
 *
 */
ina_rc_t ina_net_set_write_timeout(int fd, int msec);

/*
 *
 */
ina_rc_t ina_net_close(int fd);
]]

local meta_connection = {
  __index = {
    send = function(c, str)
      if not c then
        error("Argument 'connection' must be present")
      end
      local buf = ffi.new("char[?]", str:len())
	  ffi.copy(buf, str, str:len())
      local nwrite = ffi.new("int [1]")
      local written = 0
      repeat
        if ffi.C.ina_net_write(c._fd[0], ffi.cast("unsigned char*", buf), str:len(), nwrite) > 0 then
          return false, "Unknown write error occured"
        end
        written = written + nwrite[0]
      until written == str:len()
      return true
    end,
    receive = function(c, num_bytes)
      if not c then
        error("Argument 'connection' must be present")
      end
      local sread = num_bytes or 1024
      local buf = ffi.new("char[?]", sread)
      local nread = ffi.new("int [1]")
      local err = ffi.C.ina_net_read(c._fd[0], ffi.cast("unsigned char*", buf), sread, nread)
      if c._mode == _MODE_NON_BLOCKING and err == INA_EAGAIN then
        return nil
      elseif err > 0 then
        return nil, "Unknown error during net read"
      end
      local ret = ffi.string(buf, nread[0])
      return ret
    end,
    -- if argument is 0 set non-blocking, otherwise blocking
    settimeout = function(c, secs)
      if not c then
        error("Argument 'connection' must be present")
      end
      if secs and secs == 0 then
        ffi.C.ina_net_nonblock(c._fd[0])
        c._mode = _MODE_NON_BLOCKING
      else
        ffi.C.ina_net_block(c._fd[0])
        if secs then
          ffi.C.ina_net_set_read_timeout(c._fd[0], secs)
          ffi.C.ina_net_set_write_timeout(c._fd[0], secs)
        end
        c._mode = _MODE_BLOCKING
      end
    end,
    close = function(c)
      if not c then
        error("Argument 'connection' must be present")
      end
      ffi.C.ina_net_close(c._fd[0])
    end,
  }
}

local function _new_connection(host, port)
  local conn = {
    _fd = ffi.new("int [1]"),
    _mode = _MODE_BLOCKING,
    _addr = host,
    _port = port,
    _raddr = ffi.new("char [128]"), --FIXME: too large for IP but fix later
    _rport = ffi.new("int [1]")
  }
  setmetatable(conn, meta_connection)
  return conn
end

local meta_server = {
  __index = {
    accept = function(s)
      if not s then
        error("Argument 'server' must be present")
      end
      local c = _new_connection()
      local err = ffi.C.ina_net_tcp_accept(c._fd, s._fd[0], c._raddr, c._rport)
      if err > 0 and s._mode == _MODE_BLOCKING then
        return nil, "could not accept connection "
      elseif err > 0 then
        -- handle EAGAIN if non blocking socket
        return nil, "non blocking server not supported yet"
      end
      return c
    end,
    close = function(s)
      if not s then
        error("Argument 'server' must be present")
      end
      ffi.C.ina_net_close(s._fd[0])
    end,
  }
}

sw.connect = function(host, port)
  if not host or not port then
    error("Arguments 'host' and 'port' must be present")
  end
  local conn = _new_connection(host, port)
  if ffi.C.ina_net_tcp_connect(conn._fd, conn._addr, conn._port, 0) > 0 then
    return nil, "could not establish connection with: "..host.." on port: "..port
  end
  return conn
end

sw.bind = function(host, port)
  local server = {
    _fd = ffi.new("int [1]"),
    _addr = host,
    _port = port,
    _mode = _MODE_BLOCKING
  }
  if ffi.C.ina_net_tcp_server(server._fd, server._port, server._addr) > 0 then
    return nil, "could not bind TCP server"
  end
  setmetatable(server, meta_server)
  return server
end

return sw
