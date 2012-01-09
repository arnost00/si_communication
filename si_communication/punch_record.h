//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/cstdint.hpp>

namespace si
{
	struct punch_record
	{
		punch_record(boost::uint16_t control_number_, boost::posix_time::time_duration const& punch_time_)
			: control_number(control_number_)
			, punch_time(punch_time_)
		{}
		punch_record()
			:control_number(0)
			, punch_time(boost::posix_time::not_a_date_time)
		{}
		boost::uint16_t control_number;
		boost::posix_time::time_duration punch_time;
	};
}//namespace si
