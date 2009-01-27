#pragma once

#include "channel_interface.h"

namespace si
{

   struct channel_output_interface: public channel_interface
   {
      virtual void write_command(command_interface::pointer command) = 0;
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data) = 0;
   };

}//namespace si
