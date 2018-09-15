#include <cpprelude/IO.h>
#include <cpprelude/File.h>
#include <pensieve/Pensieve.h>

using namespace cppr;
using namespace pnsv;

void
print_version()
{
	println("Pensieve cli interface");
	printfmt("Version: {}.{}\n",
			 MAJOR, MINOR);
}

void
print_usage()
{
	print_version();
	println();
	println("Usage:\n\tpnsv-cli [OPTIONS] <files>");
	println();
	println("Options:");
	println("\t-help: prints this message");
	println("\t-version: prints the version of library");
	println("\t-verbose: verbosely does the operation");
	println("\t-check: check the file correctness");
}

struct Options
{
	bool help;
	bool version;
	bool check;
	bool verbose;
};

Options
parse_options(i32& argc, char**& argv)
{
	Options opts{};

	while(argc)
	{
		if(strcmp(*argv, "-help") == 0)
		{
			--argc;
			++argv;
			opts.help = true;
		}
		else if(strcmp(*argv, "-version") == 0)
		{
			--argc;
			++argv;
			opts.version = true;
		}
		else if(strcmp(*argv, "-verbose") == 0)
		{
			--argc;
			++argv;
			opts.verbose = true;
		}
		else if(strcmp(*argv, "-check") == 0)
		{
			--argc;
			++argv;
			opts.check = true;
		}
		else
		{
			break;
		}
	}

	return opts;
}

#define ASSERT_FAIL(err, ...) if((__VA_ARGS__) == false)\
{\
	printfmt((err));\
	printfmt("-1\n");\
	return;\
}

#define ASSERT_READ(...) if((__VA_ARGS__) == false)\
{\
	printfmt("[Error]: corrupted file\n");\
	printfmt("-1\n");\
	return;\
}

void
_load_version_1(IO_Trait* io)
{
	u64 data_length = 0;
	ASSERT_READ(vreadb(io, data_length) == 8);
	printfmt("data length: {}\n", data_length);
	u32 c = crc32_slurp(0, &data_length, 8);

	u32 files_count = 0;
	ASSERT_READ(vreadb(io, files_count) == 4);
	printfmt("files count: {}\n", files_count);
	c = crc32_slurp(c, &files_count, 4);

	for(usize i = 0; i < files_count; ++i)
	{
		u16 filename_size = 0;
		ASSERT_READ(vreadb(io, filename_size) == 2);
		c = crc32_slurp(c, &filename_size, 2);
		printfmt("filename size: {}\n", filename_size);

		auto filename_data = alloc<byte>(filename_size);
		if(vreadb(io, filename_data.all()) != filename_size)
		{
			free(filename_data);
			printfmt("[Error]: corrupted file\n");
			return;
		}
		c = crc32_slurp(c, filename_data.ptr, filename_data.size);

		printfmt("filename: `{}`\n", filename_data.ptr);

		u64 file_offset = 0;
		ASSERT_READ(vreadb(io, file_offset) == 8);
		c = crc32_slurp(c, &file_offset, 8);
		printfmt("file offset: {}\n", file_offset);
	}

	u32 crc = 0;
	ASSERT_READ(vreadb(io, crc) == 4);
	ASSERT_FAIL("[Error]: CRC mismatch, header corrupted", c == crc);

	printfmt("[BINARY CHUNKS SECTION]\n");

	u64 acc = 0;
	for(usize i = 0; i < files_count; ++i)
	{
		u64 bin_size = 0;
		ASSERT_READ(vreadb(io, bin_size) == 8);
		printfmt("chunk size: {}\n", bin_size);
		printfmt("chunk offset: {}\n", acc);
		auto buffer = alloc<byte>(bin_size);
		ASSERT_READ(vreadb(io, buffer.all()) == bin_size);

		printfmt("chunk data: `{}`...\n", make_strrng(buffer, bin_size > 32 ? 32 : bin_size));

		free(buffer);
		acc += bin_size;
	}

	printfmt("[END OF FILE]\n");
	printfmt("0\n");
}

void
load_from_stream(IO_Trait* io)
{
	u32 magic = 0;
	ASSERT_READ(vreadb(io, magic) == 4)
	printfmt("magic: 0x{:0>8X}\n", magic);
	ASSERT_FAIL("[Error]: magic is not correct\n", magic == MAGIC)

	u16 major = 0, minor = 0;
	ASSERT_READ(vreadb(io, major, minor) == 4)
	printfmt("version: {}.{}\n", major, minor);
	ASSERT_FAIL("[Error]: incompatible major version", major <= MAJOR)

	switch(major)
	{
		case 1:
			_load_version_1(io);
	}
}

#undef ASSERT_READ
#undef ASSERT_FAIL

void
check_file(const String& filename, const Options& opts)
{
	if(opts.verbose)
	{
		auto result = File::open(filename.data(), IO_MODE::READ, OPEN_MODE::OPEN_ONLY);
		if(result.error != OS_ERROR::OK)
		{
			printfmt("[Error]: file doesnot exist\n");
			return;
		}
		load_from_stream(result.value);
	}
	else
	{
		Pensieve pn;
		auto err = pn.load_from_disk(filename.data());
		printfmt("{}\n", err);
		switch(err)
		{
			case Pensieve::ERROR_FILE_DOESNOT_EXIST:
				printfmt("[Error]: file doesnot exist\n");
				break;

			case Pensieve::ERROR_FILE_CORRUPTED:
				printfmt("[Error]: file corrupted\n");
				break;

			case Pensieve::ERROR_NOT_PNSV_FILE:
				printfmt("[Error]: magic mismatch, not a pnsv file\n");
				break;

			case Pensieve::ERROR_INCOMPATIBLE_MAJOR_VERSION:
				printfmt("[Error]: incompatible major file version\n");
				break;

			case Pensieve::ERROR_HEADER_CORRUPTED:
				printfmt("[Error]: header is corrupted\n");
				break;
		}
	}
}

int
main(int argc, char** argv)
{
	//ignore the process name
	--argc;
	++argv;

	if(argc == 0)
	{
		print_usage();
		exit(-1);
	}

	auto opts = parse_options(argc, argv);

	if(opts.help)
	{
		print_usage();
		exit(0);
	}

	if(opts.version)
	{
		print_version();
		exit(0);
	}

	Dynamic_Array<String> files;
	files.reserve(argc);
	for(usize i = 0; i < argc; ++i)
		files.emplace_back(argv[i]);

	if(opts.check)
	{
		for(const auto& file: files)
			check_file(file, opts);
		exit(0);
	}
	else
	{
		print_usage();
		exit(-1);
	}
}