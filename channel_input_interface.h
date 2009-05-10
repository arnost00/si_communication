#pragma once

#include "channel_interface.h"
#include "response_interface.h"
//#include <boost/date_time.hpp>
namespace si
{

   struct channel_input_interface: public channel_interface
   {
      typedef boost::function<void()> timeout_call_type;

      virtual void register_response_expectation(response_interface::const_pointer
         , boost::posix_time::time_duration timeout = boost::posix_time::not_a_date_time
         , channel_input_interface::timeout_call_type timeout_call = channel_input_interface::timeout_call_type()) = 0;
      virtual void unregister_response_expectation(response_interface::pointer) = 0;
   };

}//namespace si
