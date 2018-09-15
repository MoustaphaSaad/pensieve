#pragma once

#include "pensieve/Exports.h"

#include <cpprelude/IO_Trait.h>
#include <cpprelude/Dynamic_Array.h>
#include <cpprelude/String.h>
#include <cpprelude/Memory_Stream.h>

#include <assert.h>

namespace pnsv
{
	using namespace cppr;

	/**
	 * Paths spec:
	 * All utf-8 rune is supported
	 * [/*] are the only not allowed characters in [file/folder]names
	 * All paths should start with the root `/`
	 * No relative paths support
	 *
	 * Pensieve spec:
	 * All values are little endian
	 * Address Size Description
	 * +00 4 Magic number
	 * +04 2 Major version
	 * +06 2 Minor version
	 * +12 8 Length of the data
	 * +20 4 Count of the files in the system
	 * +24 List of files in the system
	 * 	+24 2 filename length
	 * 	+26 N filename
	 * 	+26+N 8 offset of the file measured from the start of the data
	 * +08 4 CRC32 starting from the start of the file to this byte
	 * START OF DATA
	 * +00 8 binary content size in bytes
	 * +08 N binary content
	 */

	constexpr static u32 MAGIC = 0x33D9AFEE;
	constexpr static u16 MAJOR = u16(1);
	constexpr static u16 MINOR = u16(0);

	struct File_Header_Entry
	{
		String 	name;
		usize 	index;
	};

	struct File_Content
	{
		Memory_Stream bin;
	};

	struct Virtual_Handle
	{
		usize header_entry_index;

		bool
		valid() const
		{
			return header_entry_index != usize(-1);
		}
	};
	constexpr static Virtual_Handle INVALID_FILE_HANDLE { usize(-1) };

	API_PNSV bool
	pattern_match(String_Range pattern, String_Range str);

	API_PNSV bool
	valid_path(String_Range path);

	API_PNSV u32
	crc32(const void* ptr, usize size);

	API_PNSV u32
	crc32_slurp(u32 current_value, const void* ptr, usize size);
	
	struct Header
	{
		Dynamic_Array<File_Header_Entry> files;
		usize deleted_files_count;

		API_PNSV
		Header();

		API_PNSV Virtual_Handle
		file_create(const String& path, usize index);

		API_PNSV Virtual_Handle
		file_exists(const String& path) const;

		API_PNSV usize
		file_remove(const String& path);

		API_PNSV Dynamic_Array<Virtual_Handle>
		files_match(const String& pattern) const;
	};

	struct Pensieve
	{
		enum ERROR_CODE
		{
			ERROR_OK,
			ERROR_FILE_DOESNOT_EXIST,
			ERROR_FILE_CORRUPTED,
			ERROR_NOT_PNSV_FILE,
			ERROR_INCOMPATIBLE_MAJOR_VERSION,
			ERROR_HEADER_CORRUPTED
		};

		Header header;
		Dynamic_Array<File_Content> content;

		API_PNSV Virtual_Handle
		file_create_open(const String& path);

		API_PNSV Virtual_Handle
		file_create(const String& path);

		API_PNSV Virtual_Handle
		file_open(const String& path);

		API_PNSV void
		file_clear(Virtual_Handle handle);

		API_PNSV String_Range
		file_name(Virtual_Handle handle) const;

		API_PNSV const Memory_Stream&
		file_stream(Virtual_Handle handle) const;

		API_PNSV Memory_Stream&
		file_stream(Virtual_Handle handle);

		API_PNSV bool
		file_exists(const String& path) const;

		API_PNSV bool
		file_remove(const String& path);

		API_PNSV Dynamic_Array<Virtual_Handle>
		files_match(const String& pattern) const;

		API_PNSV u64
		total_data_size() const;

		API_PNSV void
		write_to_stream(IO_Trait* io);

		API_PNSV bool
		save_on_disk(const char* path);

		API_PNSV ERROR_CODE
		load_from_stream(IO_Trait* io);

		API_PNSV ERROR_CODE
		load_from_disk(const char* path);

		API_PNSV ERROR_CODE
		_load_version_1(IO_Trait* io);
	};
}