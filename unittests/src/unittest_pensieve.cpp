#include <catch2/catch.hpp>

#include <pensieve/Pensieve.h>

using namespace pnsv;

TEST_CASE("Path manipulations", "[path]")
{
	SECTION("valid paths")
	{
		CHECK(valid_path(make_strrng("/Aasd")) == true);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf/dsf")) == true);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf/*")) == true);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf/**")) == true);
		CHECK(valid_path(make_strrng("/Aasd/**/sdf/*")) == true);
	}

	SECTION("invalid paths")
	{
		CHECK(valid_path(make_strrng("")) == false);
		CHECK(valid_path(make_strrng("/")) == false);
		CHECK(valid_path(make_strrng("Aasd")) == false);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf/dsf/sdfsd/")) == false);
		CHECK(valid_path(make_strrng("/Aasd/sdf//sdf/dsf/sdfsd/")) == false);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf///////dsf")) == false);
		CHECK(valid_path(make_strrng("/Aasd/sdf/sdf/***")) == false);
	}

	SECTION("path patterns")
	{
		CHECK(pattern_match(make_strrng(""), make_strrng("")) == false);
		CHECK(pattern_match(make_strrng("/user/data"), make_strrng("/user/data")) == true);
		CHECK(pattern_match(make_strrng("/u/data"), make_strrng("/user/data")) == false);
		CHECK(pattern_match(make_strrng("/*/data"), make_strrng("/user/data")) == true);
		CHECK(pattern_match(make_strrng("/*/data.*"), make_strrng("/user/data.exe")) == true);
		CHECK(pattern_match(make_strrng("/*/data.*"), make_strrng("/user/data.v1.2")) == true);
		CHECK(pattern_match(make_strrng("/*/*.*"), make_strrng("/user/data.v1.2")) == true);
		CHECK(pattern_match(make_strrng("/*/*.*"), make_strrng("/user/data")) == false);
		CHECK(pattern_match(make_strrng("/*/*.*"), make_strrng("/user/data.")) == false);
		CHECK(pattern_match(make_strrng("/**/*.*"), make_strrng("/user/data.asd")) == true);
		CHECK(pattern_match(make_strrng("/**/*.*"), make_strrng("/user/bin/data.asd")) == true);
		CHECK(pattern_match(make_strrng("/usr/**/*"), make_strrng("/usr/asd/sdf/data.e")) == true);
		CHECK(pattern_match(make_strrng("/usr/*"), make_strrng("/usr/asd")) == true);
		CHECK(pattern_match(make_strrng("/usr/*"), make_strrng("/usr/asd/dsf")) == false);
	}
}

TEST_CASE("Pensieve", "[pensieve]")
{
	SECTION("files creation opening")
	{
		Pensieve pn;
		CHECK(pn.file_create_open("/usr/data").valid() == true);
		CHECK(pn.file_create_open("/usr/data").valid() == true);
		CHECK(pn.file_create("/usr/data").valid() == false);
		CHECK(pn.file_create("/usr/data2").valid() == true);
		CHECK(pn.file_open("/usr/data3").valid() == false);
		CHECK(pn.file_open("/usr/data2").valid() == true);
	}

	SECTION("file aux")
	{
		Pensieve pn;
		auto h = pn.file_create_open("/moustapha");
		CHECK(h.valid() == true);
		CHECK(pn.file_name(h) == "/moustapha");
		CHECK(pn.file_exists("/moustapha") == true);
		CHECK(pn.file_exists("/mdsoustapha") == false);
		CHECK(pn.file_remove("/moustapha2") == false);
		CHECK(pn.file_remove("/moustapha") == true);
	}

	SECTION("files match")
	{
		Pensieve pn;
		CHECK(pn.file_create_open("/usr/data").valid() == true);
		CHECK(pn.file_create_open("/usr/data.exe").valid() == true);
		CHECK(pn.file_create_open("/usr/asd/data.exe").valid() == true);
		CHECK(pn.file_create_open("/usr/sdf/data.exe").valid() == true);
		CHECK(pn.file_create_open("/usr/sdf/data.abc").valid() == true);
		
		CHECK(pn.files_match("/usr/*").count() == 2);
		CHECK(pn.files_match("/usr/*.exe").count() == 1);
		CHECK(pn.files_match("/usr/**/*.exe").count() == 3);
		CHECK(pn.files_match("/usr/**/*.*").count() == 4);
	}

	SECTION("save load")
	{
		Memory_Stream disk;
		{
			Pensieve pn;
			auto h = pn.file_create("/usr/data");
			IO_Trait* io = pn.file_stream(h);
			for(usize i = 0; i < 10; ++i)
				vprintb(io, i);
			pn.save_to_stream(disk);
		}

		disk.move_to_start();
		{
			Pensieve pn;
			CHECK(pn.load_from_stream(disk) == Pensieve::ERROR_OK);
			auto h = pn.file_open("/usr/data");
			IO_Trait* io = pn.file_stream(h);
			for(usize i = 0; i < 10; ++i)
			{
				usize ii = 0;
				CHECK(vreadb(io, ii) == sizeof(usize));
				CHECK(i == ii);
			}
		}
	}
}
