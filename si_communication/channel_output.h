//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_output_interface.h"
#include "channel_input.h"
#include <boost/shared_ptr.hpp>

namespace si
{
   struct channel_output: public channel_output_interface
   {
      typedef boost::function<void(const byte* data, const std::size_t size)> writer_type;
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
				write_raw_data(size, data);
         }
      }
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data)
		{
		}
      virtual void set_protocol(channel_protocol_interface::pointer protocol_)
      {
         protocol = protocol_;
      }
      virtual channel_protocol_interface::pointer get_protocol()
      {
         return protocol;
      }

      virtual ~channel_output(){}
   protected:
      channel_protocol_interface::pointer protocol;
   };
}//namespace si
