//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "command_interface.h"
#include "channel_protocol_interface.h"
#include <boost/shared_ptr.hpp>

namespace si
{
	struct channel_interface
	{
		typedef boost::shared_ptr<channel_interface> pointer;
		virtual void set_protocol(channel_protocol_interface::pointer const& protocol) = 0;
		virtual channel_protocol_interface::pointer get_protocol() = 0;
		virtual ~channel_interface(){}
	};
}//namespace si
