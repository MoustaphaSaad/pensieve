#include <cpprelude/IO.h>
#include <pensieve/dummy.h>
using namespace cppr;

i32
main(i32 argc, char** argv)
{
	printfmt("Hello, World!\n");
	printfmt("1 + 234 = {}\n", pnsv::add(1, 234));
	return 0;
}