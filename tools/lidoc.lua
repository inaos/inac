
function string.getExtensionFromFilename( path )
    return path:match( "%.([^%.]+)$" )
end

function string.stripExtension( path )
    local i = path:match( ".+()%.%w+$" )
    if ( i ) then return path:sub( 1, i - 1 ) end
    return path
end

function string.getPathFromFilename( path )
    return path:match( "^(.*[/\\])[^/\\]-$" ) or ""
end

function string.getFileFromFilename( path )
    if ( not path:find( "\\" ) and not path:find( "/" ) ) then return path end
    return path:match( "[\\/]([^/\\]+)$" ) or ""
end

local totable = string.ToTable
local string_sub = string.sub
local string_find = string.find
local string_len = string.len
function string.explode(separator, str, withpattern)
    if ( separator == "" ) then return totable( str ) end
    if ( withpattern == nil ) then withpattern = false end

    local ret = {}
    local current_pos = 1

    for i = 1, string_len( str ) do
        local start_pos, end_pos = string_find( str, separator, current_pos, not withpattern )
    if ( not start_pos ) then break end
        ret[ i ] = string_sub( str, current_pos, start_pos - 1 )
    current_pos = end_pos + 1
    end

    ret[ #ret + 1 ] = string_sub( str, current_pos )

    return ret
end

function string.split( str, delimiter )
    return string.explode( delimiter, str )
end

function string.implode( seperator, Table ) return
    table.concat( Table, seperator )
end

local pattern_escape_replacements = {
    ["("] = "%(",
    [")"] = "%)",
    ["."] = "%.",
    ["%"] = "%%",
    ["+"] = "%+",
    ["-"] = "%-",
    ["*"] = "%*",
    ["?"] = "%?",
    ["["] = "%[",
    ["]"] = "%]",
    ["^"] = "%^",
    ["$"] = "%$",
    ["\0"] = "%z"
}
function string.patternSafe(str)
    return (str:gsub(".", pattern_escape_replacements ) )
end

function string.starts(str, start)
    return start == "" or string.sub(str,1,string.len(start)) == start
end

function string.endsWith(str, endStr)
    return endStr == "" or string.sub(str, -string.len(endStr)) == endStr
end
function string.trim(str, char)
    if (char) then char = char:patternSafe() else char = "%s" end
    return string.match(str, "^" .. char .. "*(.-)" .. char .. "*$" ) or str
end

function string.replace( str, tofind, toreplace )
    local tbl = string.explode( tofind, str )
    if ( tbl[ 1 ] ) then return table.concat( tbl, toreplace ) end
    return str
end



local idoc = {}

local trim_line = function(line)
    return string.trim(string.trim(line, "\r"), " ")
end

local is_empty_line = function(line)
    return "" == trim_line(line)
end

local read_file = function (path)
    local file = io.open(path, "rb")
    if not file then return nil end

    local lines = {}

    for line in io.lines(path) do
        table.insert(lines, line)
    end

    file:close()
    return lines;
end


local wait_for_block_start = function(line)
    return string.starts(string.trim(line, " "), "/*")
end

local wait_for_block_end = function(line)
    return string.endsWith(string.trim(line, " "), "*/");
end

local wait_for_code_end = function(line, code)
    table.insert(code, line);
    return string.endsWith(string.trim(line, " "), ";") or is_empty_line(line)
end

local parse_block = function(block, code)
    local doc = {}
    local parametersBlock = 1
    local returnBlock = 2
    local blockType = 0

    table.insert(doc, "\n\n---")
    table.insert(doc, "\n```C\n"..string.implode("\n", code).."\n```")

    for k,v in pairs(block) do
        line = string.replace(v, "/*", "")
        line = string.replace(line, "*/", "")
        line = string.replace(line, "*", "")
        line = string.trim(line)
        if (blockType == 0) then
            if (string.lower(line) == "parameters") then
                blockType = parametersBlock
                table.insert(doc, "\n**Parameters**")
            elseif (string.lower(line) == "return") then
                blockType = returnBlock
                table.insert(doc, "\n**Return**\n")
            else
                table.insert(doc, line)
            end
        elseif blockType == parametersBlock then
            if (not is_empty_line(line)) then
                local p = string.explode("%s%s%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?", line,  true)
                if (nil ~= p[2]) then
                    table.insert(doc," - `"..p[1].."`: ".. p[2])
                else
                    table.insert(doc, string.trim(line, " "))
                end
            else
                table.insert(doc,"\n")
                blockType = 0
            end
        elseif blockType == returnBlock then
            if (not is_empty_line(line)) then
                table.insert(doc, line)
            else
                table.insert(doc,"\n")
                blockType = 0
            end
        end
    end
    return doc
end



idoc.run = function(outputDir, fileFilter)
    local files = {}

    local name = string.getFileFromFilename(fileFilter)
    local dir = string.getPathFromFilename(fileFilter)

    print("document generation to ".. outputDir);

    p, err = io.popen(string.format("find %s -type f -name '%s'", dir, name))
    if (p == null) then
        print(err)
        return
    end
    for file in p:lines() do
        local lines = read_file(file);
        local filename = outputDir .. "/".. string.stripExtension(string.getFileFromFilename(file)) .. '.md'
        print("analyzing  "..file)

        local outfile = io.open(filename,"w")
        if (nil == outfile) then
            print("couldn't not open "..filename)
            return
        end

        local k = 1
        while (k < #lines) do
            local line = trim_line(lines[k])
            if wait_for_block_start(line) then
                local block = {}
                local code = {}
                table.insert(block, line)
                while (k < #lines and not wait_for_block_end(line)) do
                    k = k + 1
                    line = trim_line(lines[k])
                    table.insert(block, line)
                end
                k = k + 1
                if (k < #lines) then
                    line = trim_line(lines[k])
                    while (k < #lines and is_empty_line(line)) do
                        k = k + 1
                        line = trim_line(lines[k])
                    end
                    while (k < #lines and not wait_for_code_end(line, code)) do
                        k = k + 1
                        line = lines[k];
                    end
                end

                if (#code > 0 and #block > 0) then
                    outfile:write(string.implode("\n", parse_block(block, code)))
                end
            end
            k = k + 1
        end
        outfile:close()
    end
end
return idoc