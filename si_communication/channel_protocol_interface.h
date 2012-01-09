//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "command_interface.h"
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/add_pointer.hpp>

namespace si
{
	struct channel_protocol_interface
	{
		typedef boost::shared_ptr<channel_protocol_interface> pointer;
		typedef boost::shared_array<boost::uint8_t> data_type;
		typedef boost::add_pointer<data_type::element_type>::type raw_data_type;
		typedef boost::function<void(command_interface::id_type ,std::size_t, command_interface::data_type, bool)> callback_type;

		virtual void encode_command(command_interface::pointer command, std::size_t &size, data_type& result) = 0;
		virtual bool process_input(std::size_t &size, raw_data_type& result, callback_type callback) = 0;

		virtual ~channel_protocol_interface(){}

	};
}//namespace si
