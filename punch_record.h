#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace si
{
	struct punch_record
	{
		punch_record(unsigned __int16 control_number_, boost::posix_time::time_duration const& punch_time_)
			: control_number(control_number_)
			, punch_time(punch_time_)
		{}
		punch_record()
			:control_number(0)
			, punch_time(boost::posix_time::not_a_date_time)
		{}
		unsigned __int16 control_number;
		boost::posix_time::time_duration punch_time;
	};
}//namespace si
