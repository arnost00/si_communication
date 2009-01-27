#pragma once

#include "channel_output.h"
#include "channel_input.h"
#include "io_base.h"

namespace si
{
   class channel_loopback: public channel_output, public io_base<channel_input>//, public channel_input
   {
   public:
      virtual void set_protocol(channel_protocol_interface::pointer protocol_)
      {
         channel_output::set_protocol(protocol_);
      }
      virtual channel_protocol_interface::pointer get_protocol()
      {
         return channel_output::get_protocol();
      }
/*
      virtual void write_command(command_interface::pointer command)
      {
         if(command)
         {
            std::size_t size;
            channel_protocol_interface::data_type data;
            if(!get_protocol())
            {
               throw std::runtime_error("channel_loopback::write_command: no protocol specified");
            }
            get_protocol()->encode_command(command, size, data);
            process_input(size, data.get());
         }
      )*/
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data)
		{
			process_input(size, data.get());
		}

   };
}//namespace si
