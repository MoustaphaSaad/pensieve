project "scratch"
	language "C++"
	kind "ConsoleApp"
	targetdir (bin_path .. "/%{cfg.platform}/%{cfg.buildcfg}/")
	location  (build_path .. "/%{prj.name}/")

	files
	{
		"include/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"include/",
		cpprelude_path .. "/include/",
		pensieve_path .. "/include/"
	}

	links
	{
		"cpprelude",
		"pensieve"
	}

	--language configuration
	exceptionhandling "OFF"
	rtti "OFF"
	warnings "Extra"
	cppdialect "c++14"

	--linux configuration
	filter "system:linux"
		defines { "OS_LINUX" }
		linkoptions {"-pthread"}

	filter { "system:linux", "configurations:debug" }
		linkoptions {"-rdynamic"}

	--windows configuration
	filter "system:windows"
		defines { "OS_WINDOWS" }
		buildoptions {"/utf-8"}
		if os.getversion().majorversion == 10.0 then
			systemversion(win10_sdk_version())
		end

	filter { "system:windows", "configurations:debug" }
		links {"dbghelp"}

	--os agnostic configuration
	filter "configurations:debug"
		defines {"DEBUG"}
		symbols "On"

	filter "configurations:release"
		defines {"NDEBUG"}
		optimize "On"

	filter "platforms:x86"
		architecture "x32"

	filter "platforms:x64"
		architecture "x64"