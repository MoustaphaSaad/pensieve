# pensieve
pensieve is a binary file format library

## Spec
- All values are little endian
```
- [Address] [Size in bytes] [Unsigned/Signed] [Description]
- +00 	4u Magic number
- +04 	2u Major version
- +06 	2u Minor version
- +08 	8u Length of the binary chunks in the file		<- CRC Start
- +16 	4u Count of the files in the system
- Sequence of the files
	- +20 	2u Filename length
	- +22 	N  Filename
	- +22+N 8u Offset of the file data measured from the start of the binary chunks section
- XX	4u CRC32 of the file measured from the start of the data length		<- CRC End (this is not included)
- Start of binary chunks data
	- +00 8u Binary content size in bytes
	- +08 N  Binary content
```

## Paths
- utf-8 is supported
- [/*] are the only not allowed characters in [file/folder]names
- All paths should start with the root `/`
- Paths should not have any `/` at the end
- No relative paths support

## Example
```C++
#include <pensieve/Pensieve.h>

void
write(const char* filename)
{
	using namespace pnsv;

	Pensieve pn;
	auto handle = pn.file_create("/numbers");

	IO_Trait* io = pn.file_stream(handle);
	for(u32 i = 0; i < 10; ++i)
		vprintb(io, i);

	pn.save_on_disk(filename);
}

void
read(const char* filename)
{
	Pensieve pn;
	auto err = pn.load_from_disk(filename);
	if(err) return;

	auto handle = pn.file_open("/numbers");
	if(handle.valid() == false) return;

	IO_Trait* io = pn.file_stream(handle);
	u32 ii = 0;
	for(u32 i = 0; i < 10; ++i)
	{
		vreadb(io, ii);
		assert(i == ii);
	}
}

void
read_pattern_search(const char* filename)
{
	Pensieve pn;
	auto err = pn.load_from_disk(filename);
	if(err) return;

	//* is used to traverse the top level of the folder
	//** is used to traverse the folders recursivly

	//here i search recursivly for all the files in meshes folder that has a `.car` ext
	auto files = pn.files_match("/meshes/**/*.car");
	for(const auto& handle: files)
	{
		IO_Trait* io = pn.file_stream(handle);
		//read the mesh
	}
}
```

## pnsv-cli
This is a cli tool to check and parse pnsv files
```
$ pnsv-cli -verbose -check file.pnsv
magic: 0x33D9AFEE
version: 1.0
data length: 40
files count: 1
filename size: 9
filename: `/numbers`
file offset: 0
[BINARY CHUNKS SECTION]
chunk size: 40
chunk offset: 0
chunk data: `          ♥   ♦   ♣   ♠      `...
[END OF FILE]
0
```