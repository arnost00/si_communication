#pragma once

#include "protocol_encoders.h"
#include "si_constants.h"

namespace si
{
   template <typename protocol_tt> struct channel_protocol_static: public channel_protocol_interface
   {
      virtual void encode_command(command_interface::pointer command, std::size_t &size, data_type& result)
      {
         return channel_protocol<protocol_tt>::encode_command(command, size, result);
      }
      virtual void process_input(command_interface::pointer command, std::size_t &size, data_type& result)
      {
         unprocessed_input.insert(unprocessed_input.end(), result.get(), result.get() + size);
         return channel_protocol<protocol_tt>::encode_command(command, size, result);
      }
      typedef std::vector<byte> unprocessed_input_type;
      unprocessed_input_type unprocessed_input;
   };
/*
   struct channel_protocol_base: public channel_protocol_interface
   {
   };

   template<typename base_tt> struct channel_protocol_common: public base_tt
   {
      typedef base_tt base_type;
      virtual void process_input(std::size_t &input_size, channel_protocol_interface::data_type& result)
      {
         unprocessed_input.insert(unprocessed_input.end(), result.get(), result.get() + input_size);
         unprocessed_input_type::iterator it = unprocessed_input.begin();
         unprocessed_input_type::iterator recentit;
         std::size_t size;
         while(it != recentit)
         {
            recentit = it;
            size = unprocessed_input.end() - it;
            base_type::detect_command(size, it);
         }
         unprocessed_input.erase(unprocessed_input.begin(), it);
      }
      typedef std::vector<si::byte> unprocessed_input_type;
      unprocessed_input_type unprocessed_input;
   };
*/
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
