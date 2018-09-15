#include "pensieve/Pensieve.h"

#include <cpprelude/File.h>

namespace pnsv
{
	bool
	pattern_match(String_Range p, String_Range s)
	{
		if(p.empty())
			return false;

		while(!p.empty())
		{
			Rune c = p.front();
			p.pop_front();

			if(c == '*')
			{
				//recursive ** case
				if(p.front() == '*')
				{
					p.pop_front();
					if(p.front() != '/') return false;
					p.pop_front();

					//the first starting option
					if(pattern_match(p, s))
						return true;

					//skips folders incrementally and tests
					while(!s.empty())
					{
						if(s.front() == '/')
						{
							s.pop_front();
							if(pattern_match(p, s))
								return true;
						}
						else
						{
							s.pop_front();
						}
					}
				}
				//simple * case
				else
				{
					if(s.empty()) return false;
					s.pop_front();

					while(true)
					{
						if(s.front() == p.front())
							break;

						if(s.front() == '/')
							break;

						if(s.empty()) return false;
						s.pop_front();
					}
				}
			}
			else
			{
				if(c != s.front())
					return false;
				s.pop_front();
			}
		}
		return s.empty();
	}

	bool
	valid_path(String_Range path)
	{
		if(path.empty())
			return false;

		if(path.front() != '/')
			return false;

		//paths cannot exceed 2^16 byte
		if(path.size() >= 0xFFFF)
			return false;

		while(!path.empty())
		{
			if(path.front() == '/')
			{
				usize count = 0;
				while(path.front() == '/')
				{
					++count;
					path.pop_front();
				}
				if(count > 1) return false;
			}
			else if(path.front() == '*')
			{
				usize count = 0;
				while(path.front() == '*')
				{
					++count;
					path.pop_front();
				}
				if(count > 2) return false;
			}
			else
			{
				path.pop_front();
			}
		}
	}

	static u32*
	_crc_gen_table()
	{
		static u32 table[256];
		u32 polynomial = 0xEDB88320;
		for(u32 i = 0; i < 256; i++) 
		{
			u32 c = i;
			for (usize j = 0; j < 8; j++) 
			{
				if (c & 1) {
					c = polynomial ^ (c >> 1);
				}
				else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
		return table;
	}

	static u32*
	_crc_table()
	{
		static u32* _table = _crc_gen_table();
		return _table;
	}

	u32
	crc32(const void* ptr, usize size)
	{
		return crc32_slurp(0, ptr, size);
	}

	u32
	crc32_slurp(u32 cv, const void* ptr, usize size)
	{
		u32* table = _crc_table();
		u32 c = cv ^ 0xFFFFFFFF;
		const u8* u = static_cast<const u8*>(ptr);
		for (usize i = 0; i < size; ++i) 
		{
			c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
		}
		return c ^ 0xFFFFFFFF;
	}


	Header::Header()
		:deleted_files_count(0)
	{}

	Virtual_Handle
	Header::file_create(const String& path, usize index)
	{
		if(deleted_files_count > 0)
		{
			for(usize i = 0; i < files.count(); ++i)
			{
				if(files[i].name.empty())
				{
					--deleted_files_count;
					return Virtual_Handle { i };
				}
			}
		}

		files.insert_back(File_Header_Entry{
			path,
			index
		});
		return Virtual_Handle { files.count() - 1 };
	}

	Virtual_Handle
	Header::file_exists(const String& path) const
	{
		for(usize i = 0; i < files.count(); ++i)
			if(files[i].name == path)
				return Virtual_Handle { i };
		return INVALID_FILE_HANDLE;
	}

	API_PNSV usize
	Header::file_remove(const String& path)
	{
		usize result = usize(-1);

		auto entry = file_exists(path);
		if(entry.valid())
		{
			files[entry.header_entry_index].name.clear();
			result = files[entry.header_entry_index].index;
			++deleted_files_count;
		}

		return result;
	}

	API_PNSV Dynamic_Array<Virtual_Handle>
	Header::files_match(const String& pattern) const
	{
		Dynamic_Array<Virtual_Handle> result;
		for(usize i = 0; i < files.count(); ++i)
			if(pattern_match(pattern.all(), files[i].name.all()))
				result.insert_back(Virtual_Handle { i });
		return result;
	}


	Virtual_Handle
	Pensieve::file_create_open(const String& path)
	{
		assert(valid_path(path.all()));

		Virtual_Handle handle = header.file_exists(path);
		if(handle.valid()) handle;
		
		content.emplace_back();
		return header.file_create(path, content.count() - 1);
	}

	Virtual_Handle
	Pensieve::file_create(const String& path)
	{
		assert(valid_path(path.all()));
		if(header.file_exists(path).valid())
			return INVALID_FILE_HANDLE;

		content.emplace_back();
		return header.file_create(path, content.count() - 1);
	}

	Virtual_Handle
	Pensieve::file_open(const String& path)
	{
		return header.file_exists(path);
	}

	void
	Pensieve::file_clear(Virtual_Handle handle)
	{
		assert(header.files.count() > handle.header_entry_index);

		content[header.files[handle.header_entry_index].index].bin.clear();
	}

	String_Range
	Pensieve::file_name(Virtual_Handle handle) const
	{
		assert(header.files.count() > handle.header_entry_index);
		return header.files[handle.header_entry_index].name.all();
	}

	const Memory_Stream&
	Pensieve::file_stream(Virtual_Handle handle) const
	{
		assert(header.files.count() > handle.header_entry_index);
		return content[header.files[handle.header_entry_index].index].bin;
	}

	Memory_Stream&
	Pensieve::file_stream(Virtual_Handle handle)
	{
		assert(header.files.count() > handle.header_entry_index);
		return content[header.files[handle.header_entry_index].index].bin;
	}

	bool
	Pensieve::file_exists(const String& path) const
	{
		assert(valid_path(path.all()));

		return header.file_exists(path).valid();
	}

	bool
	Pensieve::file_remove(const String& path)
	{
		assert(valid_path(path.all()));

		usize index = header.file_remove(path);
		if(index != usize(-1))
		{
			content[index].bin.reset();
			return true;
		}
		return false;
	}

	Dynamic_Array<Virtual_Handle>
	Pensieve::files_match(const String& pattern) const
	{
		return header.files_match(pattern);
	}

	u64
	Pensieve::total_data_size() const
	{
		u64 size = 0;
		for(const auto& c: content)
			size += c.bin.size();
		return size;
	}

	void
	Pensieve::write_to_stream(IO_Trait* io)
	{
		vprintb(io, MAGIC, MAJOR, MINOR);

		u32 crc = 0;

		u64 data_length = total_data_size();
		vprintb(io, data_length);
		crc = crc32_slurp(crc, &data_length, sizeof(data_length));

		u32 files_count = header.files.count() - header.deleted_files_count;
		vprintb(io, files_count);
		crc = crc32_slurp(crc, &files_count, sizeof(files_count));

		Dynamic_Array<Slice<byte>> bin_data;

		u64 acc = 0;
		for(const auto& file: header.files)
		{
			if(valid_path(file.name.all()) == false)
				continue;

			u16 filename_size = file.name.size();
			vprintb(io, filename_size, file.name, acc);
			crc = crc32_slurp(crc, &filename_size, sizeof(filename_size));
			crc = crc32_slurp(crc, file.name.data(), file.name.size());
			crc = crc32_slurp(crc, &acc, sizeof(acc));

			bin_data.emplace_back(content[file.index].bin.bin_content());

			//+ sizeof(u64): for the sizes of the binary content chunks
			acc += content[file.index].bin.size() + sizeof(u64);
		}

		vprintb(io, crc);

		for(const auto& bin: bin_data)
		{
			u64 bin_size = bin.size;
			vprintb(io, bin_size, bin);
		}
	}

	bool
	Pensieve::save_on_disk(const char* path)
	{
		auto result = File::open(path);
		if(result.error != OS_ERROR::OK)
			return false;
		write_to_stream(result.value);
	}

	Pensieve::ERROR_CODE
	Pensieve::load_from_stream(IO_Trait* io)
	{
		#define ASSERT_FAIL(err, ...) if((__VA_ARGS__) == false) return (err);
		
		u32 magic = 0;
		ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, magic) == 4);
		ASSERT_FAIL(ERROR_NOT_PNSV_FILE, magic == MAGIC);

		u16 major = 0, minor = 0;
		ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, major, minor) == 4);
		ASSERT_FAIL(ERROR_INCOMPATIBLE_MAJOR_VERSION, major <= major);

		switch (major)
		{
			case 1:
				return _load_version_1(io);

			default:
				return ERROR_INCOMPATIBLE_MAJOR_VERSION;
		}

		#undef ASSERT_FAIL
	}

	Pensieve::ERROR_CODE
	Pensieve::_load_version_1(IO_Trait* io)
	{
		#define ASSERT_FAIL(err, ...) if((__VA_ARGS__) == false) return (err);

		u64 data_length = 0;
		ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, data_length) == 8);
		u32 c = crc32_slurp(0, &data_length, 8);

		u32 files_count = 0;
		ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, files_count) == 4);
		c = crc32_slurp(c, &files_count, 4);

		for(usize i = 0; i < files_count; ++i)
		{
			u16 filename_size = 0;
			ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, filename_size) == 2);
			c = crc32_slurp(c, &filename_size, 2);

			auto filename_data = alloc<byte>(filename_size);
			if(vreadb(io, filename_data.all()) != filename_size)
			{
				free(filename_data);
				return ERROR_FILE_CORRUPTED;
			}
			c = crc32_slurp(c, filename_data.ptr, filename_data.size);
			//insert the files
			content.emplace_back();
			header.files.insert_back(File_Header_Entry{
				std::move(filename_data),
				content.count() - 1
			});

			u64 file_offset = 0;
			ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, file_offset) == 8);
			c = crc32_slurp(c, &file_offset, 8);
		}

		u32 crc = 0;
		ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, crc) == 4);

		ASSERT_FAIL(ERROR_HEADER_CORRUPTED, c == crc);

		for(usize i = 0; i < files_count; ++i)
		{
			u64 bin_size = 0;
			ASSERT_FAIL(ERROR_FILE_CORRUPTED, vreadb(io, bin_size) == 8);
			if(bin_size > 0)
			{
				ASSERT_FAIL(ERROR_FILE_CORRUPTED, content[i].bin.pipe_in(io, bin_size) == bin_size);
				content[i].bin.move_to_start();
			}
		}

		return ERROR_OK;
		#undef ASSERT_FAIL
	}

	Pensieve::ERROR_CODE
	Pensieve::load_from_disk(const char* path)
	{
		auto result = File::open(path, IO_MODE::READ, OPEN_MODE::OPEN_ONLY);
		if(result.error != OS_ERROR::OK)
			return ERROR_FILE_DOESNOT_EXIST;
		return load_from_stream(result.value);
	}
}
