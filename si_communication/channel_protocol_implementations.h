//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "protocol_encoders.h"
#include "si_constants.h"

namespace si
{

   template<typename protocol_tt> struct channel_protocol: public channel_protocol_interface
   {
      BOOST_MPL_ASSERT_MSG(false, UNKNOWNT_PROTOCOL, (types<protocol_tt>));
   };

   template<> struct channel_protocol<protocols::basic>: public channel_protocol_interface
   {


      virtual void encode_command(command_interface::pointer command, std::size_t &size, channel_protocol_interface::data_type& result)
      {
         byte command_id = command->get_id();
         if((command_id >= 0x80) && (command_id != 0xC4))
         {
            throw std::invalid_argument("channel_protocol<protocols::basic>::write_command: invalid command_id");
         }
         return protocol_encoder<>::encode<protocols::basic>(command, size, result);
      }
      virtual bool process_input(std::size_t &size,  raw_data_type& input, callback_type callback)
      {
         return protocol_encoder<protocols::basic>::detect_command(size, input, callback);
      }
   };

   template<> struct channel_protocol<protocols::extended>
      : public channel_protocol_interface
   {
      virtual void encode_command(command_interface::pointer command, std::size_t &size, channel_protocol_interface::data_type& result)
      {
         if(command->is_control_sequence())
         {
            return protocol_encoder<>::encode<control_sequence>(command, size, result);
         }
         byte command_id = command->get_id();
         if((command_id < 0x80) || (command_id == 0xC4))
         {
            return protocol_encoder<protocols::basic>::encode(command, size, result);
         }
         return protocol_encoder<protocols::extended>::encode(command, size, result);
      }
      virtual bool process_input(std::size_t &size,  raw_data_type& input, callback_type callback)
      {
         return protocol_encoder<protocols::extended>::detect_command(size, input, callback);
      }
   };
}//namespace si
