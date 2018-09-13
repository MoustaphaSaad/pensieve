-- [[ function returning the sdk version of windows 10 --]]
function win10_sdk_version()
	cmd_file = io.popen("reg query \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots\" | C:\\Windows\\System32\\find.exe \"10.0\"", 'r')
	output = cmd_file:read("*all")
	cmd_file:close()
	out_root, out_leaf, out_ext = string.match(output, "(.-)([^\\]-([^%.]+))$")
	sdk_version = out_leaf:gsub("%s+", "")
	return sdk_version
end

bin_path 		= path.getabsolute("bin")
build_path 		= path.getabsolute("build")
cpprelude_path 	= path.getabsolute("external/cpprelude/cpprelude")
pensieve_path	= path.getabsolute("pensieve")

workspace "pensieve"
	configurations {"debug", "release"}
	platforms {"x86", "x64"}
	location "build"
	startproject "scratch"

	include (cpprelude_path .. "/cpprelude.lua")
	include ("pensieve/pensieve.lua")
	include ("scratch/scratch.lua")