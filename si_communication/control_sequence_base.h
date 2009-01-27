#pragma once

#include <boost/function.hpp>

namespace si
{
	template <typename channel_tt = void> struct control_sequence_base
	{
		typedef boost::function<void()> callback_type;
		control_sequence_base(typename channel_tt::pointer &channel_ = channel_tt::pointer()
			, callback_type success_cb_ = callback_type()
			, callback_type failure_cb_ = callback_type())
			: channel(channel_)
			, success_cb(success_cb)
			, failure_cb(failure_cb)
		{
		}

		void start(typename channel_tt::pointer &channel_
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

		callback_type success_cb;
		callback_type failure_cb;

		typename channel_tt::pointer channel;
	};
	template<>struct control_sequence_base<void>
	{
		typedef boost::function<void()> callback_type;
	};
}//namespace si
