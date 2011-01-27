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

namespace si
{
	class startup_sequence: public control_sequence_base<channel_io_serial_port>
	{
	public:
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

			si::response<boost::mpl::deque<extended::responses::set_ms_mode, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&startup_sequence::response_set_ms_hi_speed, this, _1)
				, boost::bind(&startup_sequence::nck_set_ms_hi_speed, this, _1));
			channel->register_response_expectation(
				si::response<>::create(reaction)
				, boost::posix_time::millisec(100)
				, boost::bind(&startup_sequence::timeout_set_ms_hi_speed, this));

			channel->write_command(set_ms_mode);

		}
	protected:
		void response_set_ms_hi_speed(extended::responses::set_ms_mode::pointer &response)
		{
			std::cout << "hi speed response, cn: " << response->get<extended::cn>().value << std::endl;
			success();
		}
		void response_set_ms_hi_speed_basic(basic::responses::set_ms_mode::pointer &response)
		{
			std::cout << "hi speed response, basic protocol cn: " << response->get<basic::cn>().value << std::endl;
			success();
		}
		void response_set_ms_lo_speed(extended::responses::set_ms_mode::pointer &response)
		{
			std::cout << "lo speed response, cn: " << response->get<extended::cn>().value << std::endl;
			success();
		}
		void response_set_ms_lo_speed_basic(basic::responses::set_ms_mode::pointer &response)
		{
			std::cout << "lo speed response, on basic protocol cn: " << response->get<basic::cn>().value << std::endl;
			success();
		}
		void nck_set_ms_lo_speed(extended::responses::nak::pointer &response)
		{
			std::cout << "lo speed nck" << std::endl;
			//check basic protocol
		   channel->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<protocols::basic>()));
			basic::commands::set_ms_mode::pointer set_ms_mode(new basic::commands::set_ms_mode);
			set_ms_mode->get<basic::m_s>().value = basic::m_s::master;

			si::response<boost::mpl::deque<basic::responses::set_ms_mode, basic::responses::nak> >::reactions_type
				reaction(boost::bind(&startup_sequence::response_set_ms_lo_speed_basic, this, _1)
				, boost::bind(&startup_sequence::nck_set_ms_lo_speed_basic, this, _1));
			channel->register_response_expectation(
				si::response<>::create(reaction)
				, boost::posix_time::millisec(100)
				, boost::bind(&startup_sequence::timeout_set_ms_lo_speed_basic, this));

			channel->write_command(set_ms_mode);		
		}
		void nck_set_ms_hi_speed(extended::responses::nak::pointer &response)
		{
			std::cout << "hi speed nck" << std::endl;
			//check basic protocol
		   channel->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<protocols::basic>()));
			basic::commands::set_ms_mode::pointer set_ms_mode(new basic::commands::set_ms_mode);
			set_ms_mode->get<basic::m_s>().value = basic::m_s::master;

			si::response<boost::mpl::deque<basic::responses::set_ms_mode, basic::responses::nak> >::reactions_type
				reaction(boost::bind(&startup_sequence::response_set_ms_hi_speed_basic, this, _1)
				, boost::bind(&startup_sequence::nck_set_ms_hi_speed_basic, this, _1));
			channel->register_response_expectation(
				si::response<>::create(reaction)
				, boost::posix_time::millisec(100)
				, boost::bind(&startup_sequence::timeout_set_ms_hi_speed_basic, this));

			channel->write_command(set_ms_mode);		
		}
		void nck_set_ms_hi_speed_basic(basic::responses::nak::pointer &response)
		{
			std::cout << "hi speed nck on basic protocol: fail" << std::endl;
			failure();
		}
		void nck_set_ms_lo_speed_basic(basic::responses::nak::pointer &response)
		{
			std::cout << "lo speed nck on basic protocol: fail" << std::endl;
			failure();
		}
		void timeout_set_ms_hi_speed()
		{
			std::cout << "hi speed timeout, setting lo speed and repeating"  << std::endl;
			channel->set_option(boost::asio::serial_port_base::baud_rate(4800));

			extended::commands::set_ms_mode::pointer set_ms_mode(new extended::commands::set_ms_mode);
			set_ms_mode->get<extended::m_s>().value = extended::m_s::master;

			si::response<boost::mpl::deque<extended::responses::set_ms_mode, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&startup_sequence::response_set_ms_lo_speed, this, _1)
				, boost::bind(&startup_sequence::nck_set_ms_lo_speed, this, _1));
			channel->register_response_expectation(
				si::response<>::create(reaction)
				, boost::posix_time::millisec(500)
				, boost::bind(&startup_sequence::timeout_set_ms_lo_speed, this));

			channel->write_command(set_ms_mode);
		}
		void timeout_set_ms_hi_speed_basic()
		{
			std::cout << "hi speed timeout on basic protocol: fail"  << std::endl;
			failure();
		}
		void timeout_set_ms_lo_speed()
		{
			std::cout << "lo speed timeout: fail"  << std::endl;
			failure();
		}
		void timeout_set_ms_lo_speed_basic()
		{
			std::cout << "lo speed timeout on basic protocol: fail"  << std::endl;
			failure();
		}
	};
}//namespace si
