//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_io_serial_port.h"
#include "commands_definitions.h"
#include "channel_protocol_implementations.h"
#include "control_sequence_base.h"
#include "response.h"

#include <boost/enable_shared_from_this.hpp>

namespace si
{
	class startup_sequence: public control_sequence_base<channel_io_serial_port>
			, public boost::enable_shared_from_this<startup_sequence>
	{
	public:
		typedef boost::shared_ptr<startup_sequence> pointer;
		startup_sequence()
		{
		}
		void start(channel_io_serial_port::pointer channel
					  , callback_type success_cb = callback_type()
				, callback_type failure_cb = callback_type())
		{
			control_sequence_base<channel_io_serial_port>::start(channel, success_cb, failure_cb);

			channel->set_option(boost::asio::serial_port_base::baud_rate(38400));
			channel->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
			channel->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
			channel->set_option(boost::asio::serial_port_base::character_size(8));
			channel->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

			channel->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<si::protocols::extended>()));
			extended::commands::set_ms_mode::pointer set_ms_mode(new extended::commands::set_ms_mode);
			set_ms_mode->get<extended::m_s>().value = extended::m_s::master;

			si::response<boost::mpl::deque<extended::responses::set_ms_mode, common::nak> >::reactions_type
					reaction(boost::bind(&startup_sequence::response_set_ms_hi_speed, shared_from_this(), _1)
								, boost::bind(&startup_sequence::nck_set_ms_hi_speed, shared_from_this(), _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::millisec(100)
						, boost::bind(&startup_sequence::timeout_set_ms_hi_speed, shared_from_this()));

			channel->write_command(set_ms_mode);

		}
	protected:
		void response_set_ms_hi_speed(extended::responses::set_ms_mode::pointer &response)
		{
			LOG << "hi speed response, cn: " << response->get<extended::cn>().value << std::endl;
			success();
		}
		void response_set_ms_hi_speed_basic(basic::responses::set_ms_mode::pointer &response)
		{
			LOG << "hi speed response, basic protocol cn: " << response->get<basic::cn>().value << std::endl;
			success();
		}
		void response_set_ms_lo_speed(extended::responses::set_ms_mode::pointer &response)
		{
			LOG << "lo speed response, cn: " << response->get<extended::cn>().value << std::endl;
			success();
		}
		void response_set_ms_lo_speed_basic(basic::responses::set_ms_mode::pointer &response)
		{
			LOG << "lo speed response, on basic protocol cn: " << response->get<basic::cn>().value << std::endl;
			success();
		}
		void nck_set_ms_lo_speed(common::nak::pointer &)
		{
			LOG << "lo speed nck" << std::endl;
			//check basic protocol
			channel->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<protocols::basic>()));
			basic::commands::set_ms_mode::pointer set_ms_mode(new basic::commands::set_ms_mode);
			set_ms_mode->get<basic::m_s>().value = basic::m_s::master;

			si::response<boost::mpl::deque<basic::responses::set_ms_mode, common::nak> >::reactions_type
					reaction(boost::bind(&startup_sequence::response_set_ms_lo_speed_basic, this, _1)
								, boost::bind(&startup_sequence::nck_set_ms_lo_speed_basic, this, _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::millisec(100)
						, boost::bind(&startup_sequence::timeout_set_ms_lo_speed_basic, this));

			channel->write_command(set_ms_mode);
		}
		void nck_set_ms_hi_speed(common::nak::pointer &)
		{
			LOG << "hi speed nck" << std::endl;
			//check basic protocol
			channel->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<protocols::basic>()));
			basic::commands::set_ms_mode::pointer set_ms_mode(new basic::commands::set_ms_mode);
			set_ms_mode->get<basic::m_s>().value = basic::m_s::master;

			si::response<boost::mpl::deque<basic::responses::set_ms_mode, common::nak> >::reactions_type
					reaction(boost::bind(&startup_sequence::response_set_ms_hi_speed_basic, shared_from_this(), _1)
								, boost::bind(&startup_sequence::nck_set_ms_hi_speed_basic, shared_from_this(), _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::millisec(100)
						, boost::bind(&startup_sequence::timeout_set_ms_hi_speed_basic, shared_from_this()));

			channel->write_command(set_ms_mode);
		}
		void nck_set_ms_hi_speed_basic(common::nak::pointer &)
		{
			LOG << "hi speed nck on basic protocol: fail" << std::endl;
			failure();
		}
		void nck_set_ms_lo_speed_basic(common::nak::pointer &)
		{
			LOG << "lo speed nck on basic protocol: fail" << std::endl;
			failure();
		}
		void timeout_set_ms_hi_speed()
		{
			LOG << "hi speed timeout, setting lo speed and repeating"  << std::endl;
			channel->set_option(boost::asio::serial_port_base::baud_rate(4800));

			extended::commands::set_ms_mode::pointer set_ms_mode(new extended::commands::set_ms_mode);
			set_ms_mode->get<extended::m_s>().value = extended::m_s::master;

			si::response<boost::mpl::deque<extended::responses::set_ms_mode, common::nak> >::reactions_type
					reaction(boost::bind(&startup_sequence::response_set_ms_lo_speed, shared_from_this(), _1)
								, boost::bind(&startup_sequence::nck_set_ms_lo_speed, shared_from_this(), _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::millisec(500)
						, boost::bind(&startup_sequence::timeout_set_ms_lo_speed, shared_from_this()));

			channel->write_command(set_ms_mode);
		}
		void timeout_set_ms_hi_speed_basic()
		{
			LOG << "hi speed timeout on basic protocol: fail"  << std::endl;
			failure();
		}
		void timeout_set_ms_lo_speed()
		{
			LOG << "lo speed timeout: fail"  << std::endl;
			failure();
		}
		void timeout_set_ms_lo_speed_basic()
		{
			LOG << "lo speed timeout on basic protocol: fail"  << std::endl;
			failure();
		}
	};
}//namespace si
