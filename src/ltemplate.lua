--
-- Copyright (c) 2013, INAOS GmbH
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
------------------------------------------------------------------------------
-- Based on Expand.lua - borrowed from http://lua-users.org/wiki/TextTemplate
------------------------------------------------------------------------------

local template = {}

local strfind = string.find
local strsub  = string.sub
local push    = table.insert
local pop     = table.remove
local concat  = table.concat

local statements = {}
for w in string.gfind('do if for while repeat', '%a+') do
  statements[w] = true
end

local function expand(str, vars, env)
  assert(type(str)=='string', 'expecting string')
  local searchlist = {vars, env}
  local estring,evar

  function estring(str)
    local b,e,i
    b,i = strfind(str, '%$.')
    if not b then return str end

    local R, pos = {}, 1
    repeat
      b,e = strfind(str, '^%b{}', i)
      if b then
        push(R, strsub(str, pos, b-2))
        push(R, evar(strsub(str, b+1, e-1)))
        i = e+1
        pos = i
      else
        b,e = strfind(str, '^%b()', i)
        if b then
          push(R, strsub(str, pos, b-2))
          push(R, evar(strsub(str, b+1, e-1)))
          i = e+1
          pos = i
        elseif strfind(str, '^%a', i) then
          push(R, strsub(str, pos, i-2))
          push(R, evar(strsub(str, i, i)))
          i = i+1
          pos = i
        elseif strfind(str, '^%$', i) then
          push(R, strsub(str, pos, i))
          i = i+1
          pos = i
        end
      end
      b,i = strfind(str, '%$.', i)
    until not b

    push(R, strsub(str, pos))
    return concat(R)
  end

  local function search(index)
    for _,symt in ipairs(searchlist) do
      local ts = type(symt)
      local value
      if     ts == 'function' then value = symt(index)
      elseif ts == 'table'
          or ts == 'userdata' then value = symt[index]
          if type(value)=='function' then value = value(symt) end
      else error'search item must be a function, table or userdata' end
      if value ~= nil then return value end
    end
    error('unknown variable: '.. index)
  end

  local function elist(var, v, str, sep)
    local tab = search(v)
    if tab then
      assert(type(tab)=='table', 'expecting table from: '.. var)
      local R = {}
      push(searchlist, 1, tab)
      push(searchlist, 1, false)
      for _,elem in ipairs(tab) do
        searchlist[1] = elem
        push(R, estring(str))
      end
      pop(searchlist, 1)
      pop(searchlist, 1)
      return concat(R, sep)
    else
      return ''
    end
  end

  local function get(tab,index)
    for _,symt in ipairs(searchlist) do
      local ts = type(symt)
      local value
      if     ts == 'function' then value = symt(index)
      elseif ts == 'table'
          or ts == 'userdata' then value = symt[index]
      else error'search item must be a function, table or userdata' end
      if value ~= nil then
        tab[index] = value  -- caches value and prevents changing elements
        return value
      end
    end
  end

  function evar(var)
    if strfind(var, '^[_%a][_%w]*$') then -- ${vn}
      return estring(tostring(search(var)))
    end
    local b,e,cmd = strfind(var, '^(%a+)%s.')
    if cmd == 'foreach' then -- ${foreach vn xxx} or ${foreach vn/sep/xxx}
      local vn,s
      b,e,vn,s = strfind(var, '^([_%a][_%w]*)([%s%p]).', e)
      if vn then
        if strfind(s, '%s') then
          return elist(var, vn, strsub(var, e), '')
        end
        b = strfind(var, s, e, true)
        if b then
          return elist(var, vn, strsub(var, b+1), strsub(var,e,b-1))
        end
      end
      error('syntax error in: '.. var, 2)
    elseif cmd == 'when' then -- $(when vn xxx)
      local vn
      b,e,vn = strfind(var, '^([_%a][_%w]*)%s.', e)
      if vn then
        local t = search(vn)
        if not t then
          return ''
        end
        local s = strsub(var,e)
        if type(t)=='table' or type(t)=='userdata' then
          push(searchlist, 1, t)
          s = estring(s)
          pop(searchlist, 1)
          return s
        else
          return estring(s)
        end
      end
      error('syntax error in: '.. var, 2)
    else
      if statements[cmd] then -- do if for while repeat
        var = 'local OUT="" '.. var ..' return OUT'
      else  -- expression
        var = 'return '.. var
      end
      local f = assert(loadstring(var))
      local t = searchlist[1]
      assert(type(t)=='table' or type(t)=='userdata', 'expecting table')
      setfenv(f, setmetatable({}, {__index=get, __newindex=t}))
      return estring(tostring(f()))
    end
  end

  return estring(str)
end

-- create sandbox
local env = {} -- add functions you know are safe here

env.ipairs = ipairs
env.next = next
env.pairs = pairs
env.tonumber = tonumber
env.tostring = tostring

env.string = {}
env.string.byte = string.byte
env.string.char = string.char
env.string.format = string.format
env.string.gmatch = string.gmatch
env.string.gsub = string.gsub
env.string.len = string.len
env.string.lower = string.lower
env.string.match = string.match
env.string.rep = string.rep
env.string.reverse = string.reverse
env.string.sub = string.sub
env.string.upper = string.upper

env.math = {}
env.math.abs = math.abs
env.math.acos = math.acos
env.math.asin = math.asin
env.math.atan = math.atan
env.math.atan2 = math.atan2
env.math.ceil = math.ceil
env.math.cos = math.cos
env.math.cosh = math.cosh
env.math.deg = math.deg
env.math.exp = math.exp
env.math.floor = math.floor
env.math.fmod = math.fmod
env.math.frexp = math.frexp
env.math.huge = math.huge
env.math.ldexp = math.ldexp
env.math.log = math.log
env.math.log10 = math.log10
env.math.max = math.max
env.math.min = math.min
env.math.modf = math.modf
env.math.pi = math.pi
env.math.pow = math.pow
env.math.rad = math.rad
env.math.sin = math.sin
env.math.sinh = math.sqrt
env.math.tan = math.tanh

env.table = {}
env.table.concat = table.concat
env.table.insert = table.insert
env.table.remove = table.remove

template.process = function(tpl, vars)
  return expand(tpl, vars, env)
end

return template
