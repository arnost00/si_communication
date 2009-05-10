#pragma once

#include "command_interface.h"
#include <boost/shared_ptr.hpp>

namespace si
{

   struct response_interface
   {
      typedef boost::shared_ptr<response_interface> pointer;
	  typedef boost::add_const<pointer>::type const_pointer;

      virtual bool check_input_command(command_interface::id_type command_id
         , std::size_t size
         , command_interface::data_type data
         , bool is_control_sequence) = 0;
      virtual bool remove() = 0;
   };

}//namespace si
