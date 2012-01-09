//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_output.h"
#include "channel_input.h"
#include "io_base.h"

namespace si
{
	class channel_loopback: public io_bases<boost::mpl::deque<channel_input, channel_output> >//, public channel_input
	{
	public:
		channel_loopback(io_base::service_pointer service_ = io_base::service_pointer())
		{}
		virtual void set_protocol(channel_protocol_interface::pointer protocol_)
		{
			channel_output::set_protocol(protocol_);
		}
		virtual channel_protocol_interface::pointer get_protocol()
		{
			return channel_output::get_protocol();
		}
		/*
		virtual void write_command(command_interface::pointer command)
		{
			if(command)
			{
				std::size_t size;
				channel_protocol_interface::data_type data;
				if(!get_protocol())
				{
					throw std::runtime_error("channel_loopback::write_command: no protocol specified");
				}
				get_protocol()->encode_command(command, size, data);
				process_input(size, data.get());
			}
		)*/
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data)
		{
			process_input(size, data.get());
		}

	};
}//namespace si
