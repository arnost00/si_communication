//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_interface.h"

namespace si
{

	struct channel_output_interface: public channel_interface
	{
		virtual void write_command(command_interface::pointer command) = 0;
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data) = 0;
	};

}//namespace si
