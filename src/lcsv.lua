--
-- Copyright (c) 2015, INAOS GmbH
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

local ssub = string.sub
local sfind = string.find
local schar = string.char
local sbyte = string.byte

local csv = {}

local function _emit_line(ref, num, vals)
  local s = ""
  for i=1,num do
    if i > 1 then
      s = s..ref._delim
    end
    if type(vals[i]) == "number" then
      s = s..tostring(vals[i])
    else
      if ref._use_quoted_string then
        s = s.."\""
      end
      if not (type(vals[i]) == "string" and (sbyte(vals[i], 1) == 28)) then
        s = s..tostring(vals[i])
      end
      if ref._use_quoted_string then
        s = s.."\""
      end
    end    
  end
  s = s.."\n"
  ref._file:write(s)
end

csv.create = function(delim, use_quoted_string)
  local uqs
  if use_quoted_string == false then
    uqs = false
  else
    uqs = true
  end
  local ref = {
    _file = nil,
	_hc = 0,
    _header_add = false,
    _header_fin = false,
    _headers = {},
    _cols = 0,
	_rc = 0,
    _delim = delim,
    _use_quoted_string = uqs
  }
  if not ref._delim then
    ref._delim = ","
  end
  return ref
end

local function _check_ref(ref)
  if not ref or not type(ref) == "table" then
    error("Argument 'ref' must be present and a valid reference!")
  end
end

csv.open_file = function(ref, filename)
  if not filename or not type(filename) == "string" then
    error("Argument `filename` must be present and a string")
  end
  ref._file = io.open(filename, "w")
  if not ref._file then
    error("Could not open file: "..filename)
  end
end

csv.add_header = function(ref, ...)
  _check_ref(ref)
  if ref._header_fin then
    error("Header already finalized!")
  end
  local n = select('#',...)
  for i=1,n do
    local v = select(i, ...)
	ref._hc = ref._hc + 1
    ref._headers[ref._hc] = v
  end
  ref._header_add = true
end

csv.finalize_header = function(ref)
  _check_ref(ref)
  if ref._header_fin then
    error("Header already finalized!")
  end
  if not ref._header_add then
    error("You need at least 1 header column!")
  end
  ref._cols = #ref._headers
  _emit_line(ref, ref._cols, ref._headers)
  ref._header_fin = true
end

csv.add_row = function(ref, ...)
  _check_ref(ref)
  if not ref._header_fin then
    error("Header not finalized yet!")
  end
  local n = select('#',...)
  if n ~= ref._cols then
    error("Your trying to add "..n.." tupels however allowed only "..ref._cols)
  end
  local rows = {}
  for i=1,n do
    local v = select(i, ...)
	ref._rc = ref._rc + 1
    if not v then
      rows[ref._rc] = schar(28)
    else
      rows[ref._rc] = v
    end
  end
  _emit_line(ref, ref._cols, rows)
  rows = nil
end

csv.add_row_table = function(ref, rows)
  _check_ref(ref)
  if not ref._header_fin then
    error("Header not finalized yet!")
  end
  local n = #rows
  if n ~= ref._cols then
    error("Your trying to add "..n.." tupels however allowed only "..ref._cols)
  end
  _emit_line(ref, ref._cols, rows)
end

csv.destroy = function(ref)
  _check_ref(ref)
  if ref._file then
    ref._file:close()
  end
  ref = nil
end

csv.parse_line = function(line,sep) 
	local res = {}
	local resc = 0
	local pos = 1
	sep = sep or ','
	while true do
		local c = ssub(line,pos,pos)
		if (c == "") then break end
		if (c == '"') then
			-- quoted value (ignore separator within)
			local txt = ""
			repeat
				local startp,endp = sfind(line,'^%b""',pos)
				txt = txt..ssub(line,startp+1,endp-1)
				pos = endp + 1
				c = ssub(line,pos,pos) 
				if (c == '"') then txt = txt..'"' end 
				-- check first char AFTER quoted string, if it is another
				-- quoted string without separator, then append it
				-- this is the way to "escape" the quote char in a quote. example:
				--   value1,"blub""blip""boing",value3  will result in blub"blip"boing  for the middle
			until (c ~= '"')
			resc = resc + 1
			res[resc] = txt
			assert(c == sep or c == "")
			pos = pos + 1
		else	
			-- no quotes used, just look for the first separator
			local startp,endp = sfind(line,sep,pos)
			resc = resc + 1
			if (startp) then 
				res[resc] = ssub(line,pos,startp-1)
				pos = endp + 1
			else
				-- no separator found -> use rest of string and terminate
				res[resc] = ssub(line,pos)
				break
			end 
		end
	end
	return res
end

return csv

