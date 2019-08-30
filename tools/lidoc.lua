local table_insert = table.insert
local table_remove = table.remove
local table_concat = table.concat

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
local string_lower = string.lower
local string_format = string.format
local string_match = string.match

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
local string_explode = string.explode

function string.split( str, delimiter )
    return string_explode( delimiter, str )
end
local string_split = string.split

function string.implode( seperator, Table ) return
    table_concat( Table, seperator )
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

function string.startsWith(str, start)
    return start == "" or string_sub(str,1,string_len(start)) == start
end

function string.endsWith(str, endStr)
    return endStr == "" or string_sub(str, -string_len(endStr)) == endStr
end
function string.trim(str, char)
    if (char) then char = char:patternSafe() else char = "%s" end
    return string_match(str, "^" .. char .. "*(.-)" .. char .. "*$" ) or str
end

function string.replace( str, tofind, toreplace )
    local tbl = string_explode( tofind, str )
    if ( tbl[ 1 ] ) then return table_concat( tbl, toreplace ) end
    return str
end


local string_trim = string.trim
local string_starts = string.startsWith
local string_ends = string.endsWith
local string_replace = string.replace
local string_implode = string.implode

local is_windows = function ()
    if package.config:sub(1,1) == "\\" then
        return true
    end
    return false
end


local title
local idoc = {}

local print_log = function(message)
    print(message)
end

local print_nothing = function(message)
end

local log = print_log

local trim_line = function(line)
    local l = string_trim(line, "\r");
    return string_trim(l)
end

local is_empty_line = function(line)
    return "" == trim_line(line)
end

local file_exists = function(path)
    local file = io.open(path, "rb")
    if file ~= nil then
        file:close()
        return true
    end
    return false
end

local read_file = function (path)
    local file = io.open(path, "rb")
    if not file then return nil end

    local lines = {}

    for line in io.lines(path) do
        table_insert(lines, line)
    end

    file:close()
    return lines;
end


local wait_for_block_start = function(line)
    return string_starts(string_trim(line), "/*")
end

local wait_for_block_end = function(line)
    return string_ends(string_trim(line), "*/");
end

local wait_for_code_end = function(line, code)
    table_insert(code, line);
    return string_ends(string_trim(line), ";") or is_empty_line(line) or
            string_ends(string_trim(line), ")")
end

local parse_block = function(block, code)
    local doc = {}
    local parametersBlock = 1
    local returnBlock = 2
    local blockType = 0
    local first = true

    for k,v in pairs(block) do
        local line = string_replace(v, "/*", "")
        line = string_replace(line, "*/", "")
        line = string_replace(line, "*", "")
        line = string_trim(line)

        if string_starts(string_lower(line), "internal") or
                string_starts(string_lower(line), "internal:") then
            return doc
        end

        if (first) then
            table_insert(doc, "\n\n---")
            if string_starts(code[1], "#")  then
                local x = code[1]
                local r = string_find(x, ")", 0,1)
                if (r ~= nil and r > 0) then
                    table_insert(doc, "\n```C\n".. string_sub(x, 1, r+1) .."\n```")
                else
                    table_insert(doc, "\n```C\n"..string_implode("\n", code).."\n```")
                end
            else
                table_insert(doc, "\n```C\n"..string_implode("\n", code).."\n```")
            end
            first = false
        end

        if blockType == 0 then
            if string_lower(line) == "parameters" then
                blockType = parametersBlock
                table_insert(doc, "\n**_Parameters_**")
            elseif string_lower(line) == "return" then
                blockType = returnBlock
                table_insert(doc, "\n**_Return_**\n")
            elseif string_starts(string_lower(line), "return:") then
                blockType = returnBlock
                table_insert(doc, "\n**_Return_**: " .. string_trim(string_sub(line, 8)))
            else
                table_insert(doc, line)
            end
        elseif blockType == parametersBlock then
            if (not is_empty_line(line)) then
                local p = string_explode("%s%s%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?%s?", line,  true)
                if (nil ~= p[2]) then
                    table_insert(doc," - `"..p[1].."`: ".. p[2])
                else
                    table_insert(doc, string_trim(line))
                end
            else
                table_insert(doc,"\n")
                blockType = 0
            end
        elseif blockType == returnBlock then
            if (not is_empty_line(line)) then
                table_insert(doc, line)
            else
                table_insert(doc,"\n")
                blockType = 0
            end
        end
    end
    return doc
end

local remove_empty_lines_at_end = function(block)
    local i = #block
    while (i > 0 and is_empty_line(block[i])) do
        table_remove(block, i)
        i = i - 1
    end
end

local search_files = function(filter)
    local name = string.getFileFromFilename(filter)
    local dir = string.getPathFromFilename(filter)
    local files = {}
    if is_windows() then
        p, err = io.popen(string_format("dir /B %s", filter))
    else
        p, err = io.popen(string_format("find %s -depth 1 -type f -name '%s'", dir, name))
    end
    if (p ~= nil) then
        for file in p:lines() do
            local f = dir..string.getFileFromFilename(file)
            if (file_exists(f)) then
                table_insert(files, f)
            end
        end
    end
    return files
end

local add_to_list = function(file, files)
    local found = false
    for i,fx in ipairs(files) do
        if (file == fx) then
            found = true
            break
        end
    end
    if not found then
        log("include '"..file.."'")
        table_insert(files, file)
    end
end
local remove_from_list = function(file, files)
    for i,fx in ipairs(files) do
        if (file == fx) then
            table_remove(files, i)
            log("exclude '"..file.."'")
            break
        end
    end
end

local include_files = function(filter, files)
    if string_find(filter, "*") then
        local f = search_files(filter)
        for i,file in ipairs(f) do
            add_to_list(file, files)
        end
    elseif file_exists(filter) then
       add_to_list(filter, files)
    end
end
local exclude_files = function(filter, files)
    if (string_find(filter, "*")) then
        local f = search_files(filter)
        for k, file in ipairs(f) do
            remove_from_list(file, files)
        end
    else
        remove_from_list(filter, files)
    end
end

local load_config = function(config)
    local files = {}
    local lines = read_file(config)
    for k,line in ipairs(lines) do
        if is_windows() then
            line = string_replace(line, "/", "\\")
        end
        if string_starts(line, "+") then
            include_files(string_sub(line, 2), files)
        elseif string_starts(line,"-") then
            exclude_files(string_sub(line, 2), files)
        elseif string_starts(string_lower(line), "title:") then
            title = string_sub(line, 7)
        end
    end
    return files
end

idoc.run = function(output, config, single, quiet)
    if quiet == 1 then log = print_nothing end
    if not file_exists(config) then
        print("can't open configuration file "..config)
        return 1
    end

    local files = load_config(config)
    local outfile

    if (single == 1) then
        log("warning: title is ignored in single files mode")
    end
    if single == 0 then
        local ext = string.lower(string.getExtensionFromFilename(output))
        if (ext ~= "md") then
            print("Unsupported format '".. ext.."'")
            return 1
        end
    end

    for f, file in ipairs(files) do
        log("idoc analyzing '"..file.."'")
        local lines = read_file(file);

        local filename = output
        if single == 1 then
            filename = output .. "/" .. string.stripExtension(string.getFileFromFilename(file)) .. '.md'
        end
        if nil == outfile then
            outfile = io.open(filename,"w")
            if (nil == outfile) then
                print("couldn't not open "..filename)
                return 1
            end
            if single == 0 and title ~= nil then
                outfile:write("#"..title.."\n\n")
            end
        end

        if (string.getExtensionFromFilename(file) == "md") then
            outfile:write(string_implode("\n", lines))
        else
            local k = 1
            local summary = false;
            while (k < #lines) do
                local line = trim_line(lines[k])
                if wait_for_block_start(line) then
                    local block = {}
                    local code = {}
                    table_insert(block, line)
                    while (k < #lines and not wait_for_block_end(line)) do
                        k = k + 1
                        line = trim_line(lines[k])
                        table_insert(block, line)
                    end
                    k = k + 1
                    if (k < #lines) then
                        line = trim_line(lines[k], " ")
                        while (k < #lines and is_empty_line(line)) do
                            k = k + 1
                            line = trim_line(lines[k])
                        end
                        while (k < #lines and not wait_for_code_end(line, code)) do
                            k = k + 1
                            line = lines[k];
                        end
                    end

                    remove_empty_lines_at_end(block)
                    remove_empty_lines_at_end(code)

                    if (#code > 0 and #block > 0 and summary) then
                        local d = parse_block(block, code)
                        if #d > 0 then
                            outfile:write(string_implode("\n", d))
                        end
                    end
                    summary = true
                end
                k = k + 1
            end
        end
        if single == 1 then
            outfile:close()
            outfile = nil
        else
            outfile:write("\n\n")
        end
    end
    if outfile ~= nil then
        outfile:close()
    end
    return 0
end
return idoc