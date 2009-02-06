#pragma once

namespace si
{
	template<typename T> struct make_pointer: public T::pointer
	{
		typedef typename T::pointer t_pointer;
		make_pointer(t_pointer pointer = t_pointer(new T))
			: t_pointer(pointer)
		{}
	};
}//namespace si
