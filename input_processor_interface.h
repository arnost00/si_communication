#pragma once

#include "command.h"
#include <boost/function.hpp>
namespace si
{
   struct input_processor_interface
   {
      typedef boost::function<> 
      virtual void process_input(byte* data, std::size_t data_size) = 0;

      virtual void set_protocol(protocols::id<>::value_type protocol type) = 0;
      virtual void set_command(byte command) = 0;
      virtual void set_data(byte* data, std::size_t data_size) = 0;

      virtual protocols::id<>::value_type get_protocol() = 0;
      virtual byte set_command(bdyte command) = 0;
      virtual void set_protocol(protocols::id<>::value_type protocol type) = 0;

   };
}//namespace si
