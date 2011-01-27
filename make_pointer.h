//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
