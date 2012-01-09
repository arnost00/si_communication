//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/mpl/deque.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/asio/serial_port_service.hpp>
#include <boost/thread.hpp>

namespace si
{

	struct io_base
	{
		struct extended_service_ptr: public boost::shared_ptr<boost::asio::io_service>
		{
			typedef boost::shared_ptr<boost::asio::io_service> base_type;
			extended_service_ptr()
			{}
			extended_service_ptr(base_type::element_type *element)
				: base_type(element)
			{}
			extended_service_ptr(base_type const& base)
				: base_type(base)
			{}
			operator base_type::element_type&()
			{
				return **(base_type*)this;
			}
			~extended_service_ptr()
			{
				//std::cout << "~io_service&" << this->use_count() << std::endl;
			}
		};
		//typedef boost::shared_ptr<boost::asio::io_service> service_pointer;
		typedef boost::shared_ptr<io_base> pointer;
		typedef extended_service_ptr service_pointer;
		typedef boost::shared_ptr<boost::asio::serial_port_service> serial_service_pointer;
		typedef boost::shared_ptr<boost::asio::io_service::work> work_pointer;
		typedef boost::shared_ptr<boost::thread> thread_pointer;

		io_base(io_base::service_pointer service_ = io_base::service_pointer())
			: service(service_)
		{
			if(!service)
			{
				service.reset(new service_pointer::element_type());
				work.reset(new io_base::work_pointer::element_type(*service));
				thread.reset(new io_base::thread_pointer::element_type
								 (boost::bind(&service_pointer::element_type::run, service.get())));
				serial_service.reset(new serial_service_pointer::element_type(*service));
			}
		}
		~io_base()
		{
			work.reset();
			if(thread && (thread->joinable()))
			{
				if(serial_service)
				{
					serial_service_pointer::element_type::implementation_type i_t;
					boost::system::error_code ec;
					serial_service->cancel(i_t, ec);
					if(ec)
					{
						std::cout << ec.message();
					}
				}
				service->stop();
				service->reset();
				thread->interrupt();
				//            thread->join();
				serial_service.reset();
				thread.reset();
			}
		}

		service_pointer service;
		serial_service_pointer serial_service;
		work_pointer work;
		thread_pointer thread;

	};

	template<typename bases_tt, typename enabled = void> struct bases
			: public boost::mpl::front<bases_tt>::type
			, public bases<typename boost::mpl::pop_front<bases_tt>::type>
	{
		typedef typename boost::mpl::front<bases_tt>::type item_base_type;
		typedef bases<typename boost::mpl::pop_front<bases_tt>::type> remaining_bases_type;
		bases(io_base::service_pointer service)
			: item_base_type(service)
			, remaining_bases_type(service)
		{}
	};
	template<typename bases_tt> struct bases
			<bases_tt, typename boost::enable_if<typename boost::mpl::empty<bases_tt>::type>::type>
	{
		bases(io_base::service_pointer service)
		{}
	};

	template<typename base_tt> struct io_bases
			: public io_base
			, public bases<base_tt>
	{
		io_bases(io_base::service_pointer service_ = io_base::service_pointer())
			: io_base(service_)
			, bases<base_tt>(io_base::service)
		{
		}
	};

}//namespace si
