//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma

#include "fixed_command.h"

namespace si
{
   template<byte command_tt
      , typename parameters_tt = boost::deque<>
      , bool control_sequence = false
         > struct command
      : public fixed_command<command_tt, parameters_tt, control_sequence>
      , public command_interface
   {
      typedef fixed_command<command_tt, parameters_tt> base_command;

      command():
      command_id(command_base::command_code)
      {
      }

      virtual id_type get_id(protocols::id<>::value_type protocol = protocols::id<>::value)
      {
         return command_id;
      }
      virtual void set_id(id_type new_command_code, protocols::id<>::value_type protocol = protocols::id<>::value)
      {
         command_id = new_command_code;
      }

      byte command_id;
   };
}//namespace si
