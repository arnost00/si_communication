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

#include <boost/thread.hpp>

namespace si
{
	template<typename base_tt = void, typename enabled = void> struct io_base: public io_base<boost::mpl::deque<base_tt> >
	{
      typedef boost::shared_ptr<boost::asio::io_service> service_pointer;
		io_base(service_pointer service_ = service_pointer())
			: io_base<boost::mpl::deque<base_tt> >(service_)
		{}
	};
	template<> struct io_base<void, void>
	{
		typedef boost::shared_ptr<boost::asio::io_service> service_pointer;

		class service_wrapper: public service_pointer
		{
		public:
			service_wrapper(service_pointer &service)
				: service_pointer(service)
				, created(!service)
			{
				if(created)
				{
					reset(new service_pointer::element_type());
				}
			}
			service_wrapper()
				: service_pointer(new service_pointer::element_type())
				, created(true)
			{}
			bool is_created()
			{
				return created;
			}
			operator service_pointer::element_type&()
			{
				return *get();
			}
		protected:
			bool created;
		};
		typedef boost::shared_ptr<boost::asio::io_service::work> work_pointer;
		typedef boost::shared_ptr<boost::thread> thread_pointer;

		template<typename bases_tt, typename enabled = void> struct bases
			: public boost::mpl::front<bases_tt>::type
			, public bases<typename boost::mpl::pop_front<bases_tt>::type>
		{
			typedef typename boost::mpl::front<bases_tt>::type item_base_type;
                        typedef typename si::io_base<void, void>::bases<typename boost::mpl::pop_front<bases_tt>::type> remaining_bases_type;
			bases(service_wrapper service)
				: item_base_type(service)
				, remaining_bases_type(service)
			{}	
		};
		template<typename bases_tt> struct bases<bases_tt, typename boost::enable_if<typename boost::mpl::empty<bases_tt>::type>::type>
		{
			bases(service_wrapper )
			{}	
		};
	};

	template<typename base_tt> struct io_base<base_tt
		, typename boost::enable_if<typename boost::mpl::is_sequence<base_tt>::type>::type>
			: public io_base<>::bases<base_tt>
	{
		io_base(io_base<>::service_wrapper service_ = io_base<>::service_wrapper())
			: io_base<>::bases<base_tt>(service_)
			, service(service_)
		{
			work.reset(new io_base<>::work_pointer::element_type(*service.get()));
			if(service_.is_created())
			{
				thread.reset(new io_base<>::thread_pointer::element_type(boost::bind(&boost::asio::io_service::run, service.get())));
			}
		}
		~io_base()
		{
			work.reset();
			if(thread && (thread->joinable()))
			{
				thread->interrupt();
				thread->join();
			}
		}

		typename io_base<>::service_pointer service;
		typename io_base<>::work_pointer work;
		typename io_base<>::thread_pointer thread;
	};

}//namespace si
