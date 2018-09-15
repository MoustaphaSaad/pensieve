#include <cpprelude/IO.h>
#include <pensieve/Pensieve.h>

#include <msgpack.h>

using namespace cppr;
using namespace pnsv;

struct Koko
{
	r32 x, y, z;
};

void
stuff()
{
	/* msgpack::sbuffer is a simple buffer implementation. */
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);

    /* serialize values into the buffer using msgpack_sbuffer_write callback function. */
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);

    /* deserialize the buffer into msgpack_object instance. */
    /* deserialized object is valid during the msgpack_zone instance alive. */
    msgpack_zone mempool;
    msgpack_zone_init(&mempool, 2048);

    msgpack_object deserialized;
    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);

    /* print the deserialized object. */
    msgpack_object_print(stdout, deserialized);
    puts("");

    msgpack_zone_destroy(&mempool);
    msgpack_sbuffer_destroy(&sbuf);
}

void
write()
{
	Pensieve pn;
	auto fh = pn.file_create("/numbers");

	IO_Trait* io = pn.file_stream(fh);
	for(u32 i = 0; i < 10; ++i)
		vprintb(io, i);

	pn.save_on_disk("test.pnsv");
}

void
read()
{
	Pensieve pn;
	auto err = pn.load_from_disk("test.pnsv");

	auto fh = pn.file_open("/numbers");

	IO_Trait* io = pn.file_stream(fh);
	u32 ii = 0;
	for(u32 i = 0; i < 10; ++i)
	{
		vreadb(io, ii);
		if(i != ii)
			printfmt("ERROR CONTENT MISMATCH {} != {}\n", i, ii);
	}
	int sdf = 234;
}

i32
main(i32 argc, char** argv)
{
	write();
	read();
	stuff();
	Pensieve pn;
	Virtual_Handle io = pn.file_create("/usr/data");
	io = pn.file_create("/bin/data");
	io = pn.file_create("/ext/abc/data");
	io = pn.file_create("/data");
	pn.save_on_disk("Koko.pnsv");
	auto er = pn.files_match("/data");
	auto p = make_strrng("");
	auto s = make_strrng("");
	printfmt("{}\n", pattern_match(make_strrng(""), make_strrng("")));
	printfmt("{}\n", pattern_match(make_strrng("/usr/data"), make_strrng("/usr/data")));
	printfmt("{}\n", pattern_match(make_strrng("/u/data"), make_strrng("/usr/data")));
	printfmt("{}\n", pattern_match(make_strrng("/*/data"), make_strrng("/a/data")));
	printfmt("{}\n", pattern_match(make_strrng("/*/data.*"), make_strrng("/usr/data.exe")));
	printfmt("{}\n", pattern_match(make_strrng("/*/*.*"), make_strrng("/a/b.c")));
	printfmt("{}\n", pattern_match(make_strrng("/*/*.*"), make_strrng("/usr/data.")));
	printfmt("{}\n", pattern_match(make_strrng("/**/*.*"), make_strrng("/usr/data.exe")));
	printfmt("{}\n", pattern_match(make_strrng("/**/*.*"), make_strrng("/usr/asd/sdf/data.e")));
	printfmt("{}\n", pattern_match(make_strrng("/usr/**/*"), make_strrng("/usr/asd/sdf/data.e")));
	printfmt("{}\n", pattern_match(make_strrng("/usr/*"), make_strrng("/usr/asd")));
	printfmt("{}\n", pattern_match(make_strrng("/usr/*"), make_strrng("/usr/asd/dsf")));
	printfmt("{}\n", valid_path(make_strrng("//")));
	return 0;
}