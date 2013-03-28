//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/assert.hpp>

namespace boostext
{
	template<typename searched_type, typename searched_sequence> struct type_not_found_assert
	{
		typedef type_not_found_assert type;
		BOOST_MPL_ASSERT_MSG(false, SEQUENCE_DOESN_T_CONTAIN_TYPE, (types<searched_sequence, searched_type>));
	};
	template<typename searched_type, typename searched_sequence
		, typename not_found_type = type_not_found_assert<searched_type, searched_sequence> > struct sequence_position
			: public boost::mpl::if_<
			typename boost::is_same<
			typename boost::mpl::find<searched_sequence, searched_type>::type
			, typename boost::mpl::end<searched_sequence>::type>::type
			, not_found_type
			, typename boost::mpl::find<searched_sequence, searched_type>::type::pos>::type{};

}
