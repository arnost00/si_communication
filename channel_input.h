//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_input_interface.h"
#include "io_base.h"

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <algorithm>
#include <list>
#include <vector>

#include <memory.h>

namespace si
{

	class channel_input: public channel_input_interface, public boost::enable_shared_from_this<channel_input>
	{
	public:
		typedef io_base::service_pointer service_pointer;
		typedef boost::shared_ptr<boost::asio::deadline_timer> timer_pointer;
		typedef boost::shared_ptr<channel_input> pointer;

		channel_input(service_pointer service_)
			: input_size(0)
			, service(service_)
		{
			if(!service)
			{
				throw std::invalid_argument("channel_input::channel_iput: valid service required");
			}
		}
		~channel_input()
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			std::for_each(response_expectations.begin()
							  , response_expectations.end()
							  , boost::bind(&channel_input::cancel_timer, _1));
		}
		virtual void set_protocol(channel_protocol_interface::pointer protocol_)
		{
			protocol = protocol_;
		}
		virtual channel_protocol_interface::pointer get_protocol()
		{
			return protocol;
		}

		void timeout_expired(timer_pointer timeout_timer, channel_input_interface::timeout_call_type timeout_call)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			response_expectations_container::iterator it = std::find_if(response_expectations.begin(), response_expectations.end()
																							, (timeout_timer == boost::lambda::bind(&response_expectations_container::value_type::second, boost::lambda::_1)));
			if(response_expectations.end() != it)
			{
				response_expectations.erase(it);
				if(timeout_call)
				{
					timeout_call();
				}
			}
		}
		void register_response_expectation(response_interface::const_pointer expectation
													  , boost::posix_time::time_duration timeout  = boost::posix_time::not_a_date_time
				, channel_input_interface::timeout_call_type timeout_call = channel_input_interface::timeout_call_type())
		{
			timer_pointer timeout_timer;

			mutal_exclusion_type::scoped_lock sl(mtx);
			if(!(timeout.is_not_a_date_time()))
			{
				timeout_timer.reset(new timer_pointer::element_type(*service.get()));
				timeout_timer->expires_from_now(timeout);
				timeout_timer->async_wait(boost::bind(&channel_input::timeout_expired, shared_from_this(), timeout_timer, timeout_call));
			}
			response_expectations.push_front(response_expectations_container::value_type(expectation, timeout_timer));
			life_bound = shared_from_this();
		}

		void unregister_response_expectation(response_interface::pointer expectation)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			response_expectations_container::iterator it = std::find_if(response_expectations.begin(), response_expectations.end()
																							, (expectation == boost::lambda::bind(&response_expectations_container::value_type::first, boost::lambda::_1)));
			if(response_expectations.end() != it)
			{
				response_expectations.erase(it);
			}
			if(response_expectations.empty())
				life_bound.reset();
		}

		void process_input(std::size_t data_size, boost::uint8_t* data)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);

			if(0 == data_size)
			{
				return;
			}
			data_type new_input(new data_type::element_type[input_size + data_size]);
			memcpy(new_input.get(), input.get(), input_size);
			memcpy(new_input.get() + input_size, data, data_size);
			input = new_input;
			input_size += data_size;
			check_input();
			//         new_input_arrived.notify_all();
		}
		void check_input()
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			std::size_t size = input_size;

			channel_protocol_interface::raw_data_type raw_input = input.get();

			while((0 != size) && get_protocol()->process_input(size, raw_input, boost::bind(&channel_input::process_command_input, shared_from_this(), _1, _2, _3, _4)));

			if(size != input_size)
			{
				data_type new_input(new data_type::element_type[size]);
				memcpy(new_input.get(), input.get() + input_size - size, size);
				input = new_input;
				input_size = size;
			}
		}
	protected:
		typedef std::pair<response_interface::pointer, timer_pointer> response_with_timeout_type;
		typedef std::list<response_with_timeout_type> response_expectations_container;
		typedef channel_protocol_interface::data_type data_type;
		typedef boost::recursive_mutex mutal_exclusion_type;

		void process_command_input(command_interface::id_type command_id ,std::size_t size, command_interface::data_type data, bool control_sequence)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			response_expectations_container::iterator it = std::find_if(response_expectations.begin(), response_expectations.end()
																							, boost::bind(&response_expectations_container::value_type::first_type::element_type::check_input_command, boost::bind(&response_expectations_container::value_type::first, _1), command_id, size, data, control_sequence));
			if(response_expectations.end() == it)
				return;
			if(it->second)
			{
				it->second->cancel();
			}
			if(it->first->remove())
			{
				response_expectations.erase(it);
			}
		}
		static void cancel_timer(response_expectations_container::value_type &value)
		{
			if(value.second)
			{
				value.second->cancel();
			}
		}

		response_expectations_container response_expectations;
		data_type input;
		std::size_t input_size;

		mutal_exclusion_type mtx;
		boost::condition new_input_arrived;

		service_pointer service;
		channel_protocol_interface::pointer protocol;

		pointer life_bound;

	};

}//namespace si
