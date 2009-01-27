#pragma once
#include "static_command.h"
namespace si
{
   template<typename commands, bool no_commands = boost::mpl::empty<commands>::value>class static_multicommand_parallel_base
      : public boost::mpl::front<commands>::type
      , public static_multicommand_parallel_base<typename boost::mpl::pop_front<commands>::type>
   {
   public:
      typedef boost::mpl::front<commands>::type prefered_command_base;
      typedef public static_multicommand_parallel_base<typename boost::mpl::pop_front<commands>::type> alternate_commands;

      inline static void write(command_interface::writer_type writer, protocols::id<>::value_type protocol)
      {
         return prefered_command_base::allowed_for_protocol(protocol)? prfered_command::write(writer, protocol): alternate_commands::write(writer, protocol);
      }
      inline static std::size_t size(protocols::id<>::value_type protocol)
      {
         return prefered_command_base::allowed_for_protocol(protocol)? prfered_command::size(protocol): alternate_commands::write(protocol);
      }
      inline static bool allowed_for_protocol(protocols::id<>::value_type protocol)
      {
         return prefered_command_base::allowed_for_protocol(protocol)? true: alternate_commands::allowed_for_protocol(protocol);
      }
   };
   template<typename commands>class static_multicommand_parallel_base<commands, true>
   {
      inline static void write(command_interface::writer_type writer, protocols::id<>::value_type protocol)
      {
            throw std::invalid_argument("si::command::write::incorrect_protocol");
      }
      inline static std::size_t size(protocols::id<>::value_type protocol)
      {
            throw std::invalid_argument("si::command::size::incorrect_protocol");
      }
      inline static bool allowed_for_protocol(protocols::id<>::value_type protocol)
      {
         return false;
      }
   }

}//namespace si
