//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include "punch_record.h"
#include "tuple_child.hpp"
#include <boost/tuple/tuple.hpp>
#include <vector>
#include <iomanip>

namespace si
{
	typedef boost::tuple<
		unsigned __int32
		, unsigned __int32
		, boost::posix_time::time_duration
		, boost::posix_time::time_duration
		, boost::posix_time::time_duration
		, boost::posix_time::time_duration
		, std::vector<punch_record> > card_record_tuple;
	struct card_record: public card_record_tuple
	{
		typedef boost::shared_ptr<card_record> pointer;
//		TUPLE_CHILD_DIRECT_CONSTRUCTS(7, card_record, card_record_tuple);
		BOOST_STATIC_CONSTANT(unsigned, CARD_ID = 0);
		BOOST_STATIC_CONSTANT(unsigned, START_NO = 1);
		BOOST_STATIC_CONSTANT(unsigned, START_TIME = 2);
		BOOST_STATIC_CONSTANT(unsigned, FINISH_TIME = 3);
		BOOST_STATIC_CONSTANT(unsigned, CHECK_TIME = 4);
		BOOST_STATIC_CONSTANT(unsigned, CLEAR_TIME = 5);
		BOOST_STATIC_CONSTANT(unsigned, PUNCH_RECORDS = 6);
//		BOOST_MPL_ASSERT_MSG(false, nothing_special, (types<boost::tuples::element<card_record::START_TIME,card_record_tuple>::type>));
	};

	void stdout_ptime_duration(boost::posix_time::time_duration &duration)
	{
	if(duration.is_not_a_date_time())
	{
		std::cout << "No time ";
		return;
	}
	std::cout << std::setfill('0') << std::dec;
	std::cout << std::setw(2) << duration.hours();
	std::cout << ':' << std::setw(2) << duration.minutes();
	std::cout << ':' << std::setw(2) << duration.seconds();
//		std::cout << duration.total_milliseconds() % 1000;

	}
	void stdout_punch_record(punch_record &punch)
	{
		std::cout << std::setfill(' ') << std::setw(3) << punch.control_number << "/";
		stdout_ptime_duration(punch.punch_time);
		std::cout << " \t";
	}

	void stdout_card_record(card_record &card)
	{
		std::cout << "Card id: \t" << card.get<card_record::CARD_ID>() << std::endl;
		std::cout << "Start no: \t" << card.get<card_record::START_NO>() << std::endl;
		std::cout << "Start time: \t";
		stdout_ptime_duration(card.get<card_record::START_TIME>());
		std::cout << std::endl;
		std::cout << "Finish time: \t";
		stdout_ptime_duration(card.get<card_record::FINISH_TIME>());
		std::cout << std::endl;
		std::cout << "Check time: \t";
		stdout_ptime_duration(card.get<card_record::CHECK_TIME>());
		std::cout << std::endl;
		std::cout << "Clear time: \t";
		stdout_ptime_duration(card.get<card_record::CLEAR_TIME>());
		std::cout << std::endl;

		std::cout << "Punch recods: \t" << std::endl;
		std::for_each(card.get<card_record::PUNCH_RECORDS>().begin()
			, card.get<card_record::PUNCH_RECORDS>().end()
			, boost::bind(&stdout_punch_record, _1));
		std::cout << std::endl;
	}
}//namespace si
