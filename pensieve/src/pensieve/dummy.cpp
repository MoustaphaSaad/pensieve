#include "pensieve/dummy.h"
#include <cpprelude/IO.h>
#include <typeinfo>

namespace pnsv
{
	using namespace cppr;

	int
	add(int a, int b)
	{
		auto* x = &typeid(int);
		printfmt("0x{:0>16X}\n", usize(x));
		return a + b;
	}
}