//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/function.hpp>
#include <boost/mpl/assert.hpp>

namespace si
{
	template <typename channel_tt = void> struct control_sequence_base
	{
		typedef boost::function<void()> callback_type;
		typedef typename channel_tt::pointer channel_type;

//		BOOST_MPL_ASSERT_MSG(false, CHANNEL_TT_PARAMETER, (types<channel_tt>));
		control_sequence_base(channel_type channel_ = channel_type()
			, callback_type success_cb_ = callback_type()
			, callback_type failure_cb_ = callback_type())
			: channel(channel_)
			, success_cb(success_cb_)
			, failure_cb(failure_cb_)
		{
		}

		void start(channel_type channel_
			, callback_type success_cb_ = callback_type()
			, callback_type failure_cb_ = callback_type())
		{
			channel = channel_;
			success_cb = success_cb_;
			failure_cb = failure_cb_;
		}

		void success()
		{
			if(success_cb)
				success_cb();
			channel.reset();
		}
		void failure()
		{
			if(failure_cb)
				failure_cb();
			channel.reset();
		}

		channel_type channel;
		callback_type success_cb;
		callback_type failure_cb;
	};
	template<>struct control_sequence_base<void>
	{
		typedef boost::function<void()> callback_type;
	};
}//namespace si
