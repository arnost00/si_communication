#pragma once

namespace si
{
	template<typename T> struct make_pointer: public T::pointer
	{
		make_pointer(typename T::pointer pointer = T::pointer(new T))
			: T::pointer(pointer)
		{}
	};
}//namespace si
