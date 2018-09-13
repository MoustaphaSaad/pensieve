project "pensieve"
	language "C++"
	kind "SharedLib"
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
		cpprelude_path .. "/include/"
	}

	links { "cpprelude" }

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
		if os.getversion().majorversion == 10.0 then
			systemversion(win10_sdk_version())
		end

	filter { "system:windows", "configurations:debug" }
		links {"dbghelp"}

	--os agnostic configuration
	filter "configurations:debug"
		targetsuffix "d"
		defines {"DEBUG", "PNSV_DLL"}
		symbols "On"

	filter "configurations:release"
		defines {"NDEBUG", "PNSV_DLL"}
		optimize "On"

	filter "platforms:x86"
		architecture "x32"

	filter "platforms:x64"
		architecture "x64"