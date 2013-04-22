local conffile = {}

local function _dump()
    if type(o) == 'table' then
        local s = '{ '
        for k,v in pairs(o) do
            if type(k) ~= 'number' then k = '"'..k..'"' end
            s = s .. '['..k..'] = ' .. _dump(v) .. ','
        end
        return s .. '} '
    else
        return tostring(o)
    end
end

local _section_func = function(content)
    if not content then
        error("Section argument can't be nil!")
    end
    if not type(content) == "table" then
        error("Section argument must be a table!")
    end
    local sn = debug.getinfo(1,"n").name
    local section = sections[sn]
    for k,v in pairs(section.keys) do
        if not content[k] and v.required then
            error("Key: "..k.." not found in section: "..section.name)  
        end
        if content[k] then
            if not type(content[k]) == v.typename then
                error("Wrong type for value in key: "..k)
            end
            v.value = content[k]
            v.has_value = true
        end
    end
    section.configured = true
end

local _named_section_func = function(name)
    if not name or not type(name) == "string" then
        error("Section 'name' must be a string!")
    end
    local sn = debug.getinfo(1,"n").name
    return function(content)
        if not content then
            error("Section argument can't be nil!")
        end
        if not type(content) == "table" then
            error("Section argument must be a table!")
        end
        local section = sections[sn]
        section.children[name] = {}
        local subsec = section.children[name]
        for k,v in pairs(section.keys) do
            if not content[k] and v.required then
                error("Key: "..k.." not found in section: "..section.name)
            end
            if content[k] then
                if not type(content[k]) == v.typename then
                    error("Wrong type for value in key: "..k)
                end
                subsec[k] = {
                    value = content[k],
                    has_value = true,
                    typename = v.typename
                }
            end
        end
        section.configured = true
    end
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


-- run code under environment [Lua 5.1]
local function _run(code)
    local untrusted_function, message = loadstring(code)
    if not untrusted_function then
        return false, message 
    end
    setfenv(untrusted_function, env)
    local ret, initfunc = pcall(untrusted_function)
    return ret, initfunc
end

conffile.save_sections = function(sections, section_file)
    local f = io.open(section_file, "w")
    local code = f:write(_dump(o)) 
end

conffile.process = function(sections, config_file)
  local f = io.open(config_file, "r")
  if not f then
    error("Error opening file")
  end
  local code = f:read("*a")
  f:close()
  for sk,section in pairs(sections) do
    if not section.named then
      env[section.name] = _section_func
    else
      env[section.name] = _named_section_func
      section.children = {}
    end
  end

  -- load config and validate
  local success, err = _run(code)
  if not success then
    error(err)
  end

  -- additional validation
  for sk,s in pairs(sections) do
    if s.required and not s.configured then
      error("Section: "..sk.." required but not configured")
    end
  end
end

return conffile
