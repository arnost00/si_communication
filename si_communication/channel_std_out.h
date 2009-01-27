#pragma once

#include "channel_output.h"
#include "channel_protocol_interface.h"
#include "channel_protocol_implementations.h"
#include <boost/bind.hpp>
#include <iostream>

namespace si
{
   struct channel_std_out: public channel_output
   {
      channel_std_out()
      {
         set_protocol(si::channel_protocol_interface::pointer(new channel_protocol<protocols::extended>()));
      }
      virtual void write_command(command_interface::pointer command)
      {
         if(command)
         {
            std::size_t size;
            channel_protocol_interface::data_type data;
            protocol->encode_command(command, size, data);
            char_array_to_std_out(size, data.get());
//            command->write(boost::bind(&channel_std_out::char_array_to_std_out, _1, _2), protocols::id<protocols::extended>::value);
         }
      }
      virtual ~channel_std_out(){}
      template<typename iterator>static void char_array_to_std_out(const std::size_t size, iterator data)
      {
         for(std::size_t i = 0; i< size; i++, data ++)
         {
            std::cout << unsigned(*data) << ',';
         }
         std::cout << std::endl;
      }
   };
}//namespace si
