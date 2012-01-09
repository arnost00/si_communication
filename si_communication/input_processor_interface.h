//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "command.h"
#include <boost/function.hpp>
namespace si
{
	struct input_processor_interface
	{
		typedef boost::function<>
		virtual void process_input(boost::uint8_t* data, std::size_t data_size) = 0;

		virtual void set_protocol(protocols::id<>::value_type protocol type) = 0;
		virtual void set_command(boost::uint8_t command) = 0;
		virtual void set_data(boost::uint8_t* data, std::size_t data_size) = 0;

		virtual protocols::id<>::value_type get_protocol() = 0;
		virtual boost::uint8_t set_command(bdyte command) = 0;
		virtual void set_protocol(protocols::id<>::value_type protocol type) = 0;

	};
}//namespace si
