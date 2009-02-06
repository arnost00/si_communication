#pragma once

#include "si_constants.h"
#include "si_auxilary.h"

#include "command_interface.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/size.hpp>

#include <cstddef>

namespace si
{
   template <typename command_sequence> struct command_binary_type: public boost::array<byte, boost::mpl::size<command_sequence>::type::value>
   {
   protected:
      typedef boost::array<byte, boost::mpl::size<command_sequence>::type::value> base_type;
   public:
      template<typename sequence = command_sequence, std::size_t position = 0, bool is_empty = boost::mpl::empty<sequence>::value> struct copy_sequence_to_base
      {
         static inline  void run(command_binary_type *that)
         {
            if(!boost::mpl::empty<sequence>::value)
            {
               that->at(position) = boost::mpl::front<sequence>::type::value;
               copy_sequence_to_base<typename boost::mpl::pop_front<sequence>::type, position + 1>::run(that);
            }
         }
      };
      template<typename sequence, std::size_t position> struct copy_sequence_to_base<sequence, position, true>
      {
         static inline  void run(command_binary_type *that){}
      };

      command_binary_type()
      {
         copy_sequence_to_base<>::run(this);
      }
   };

   template<byte command_tt
      , typename data_tt
      , typename protocol_tt = typename select_protocol_by_command<typename byte_type<command_tt>::type>::type
      , typename allowed_in_protocols_policy = protocols::less_or_equeal_protocol<protocol_tt> >
   struct static_command_base
   {
    BOOST_STATIC_CONSTANT(byte, command = command_tt);
    typedef typename construct_sequence<typename byte_type<command>::type
            , data_tt
            , protocol_tt>::type command_sequence;
    typedef protocol_tt protocol;

    inline static void write(command_interface::writer_type writer, protocols::id<>::value_type protocol)
      {
         if(! allowed_in_protocols_policy::check(protocol))
         {
            throw std::invalid_argument("si::command::write::incorrect_protocol");
         }

         static const command_binary_type<command_sequence> command_binary;
         writer(&command, sizeof(command));
         writer(command_binary.data(), command_binary.size());
      }
      inline static std::size_t size(protocols::id<>::value_type protocol)
      {
         if(! allowed_in_protocols_policy::check(protocol))
         {
            throw std::invalid_argument("si::command::size::incorrect_protocol");
         }
         return boost::mpl::size<command_sequence>::value;
      }
      inline static bool allowed_for_protocol(protocols::id<>::value_type protocol)
      {
         return allowed_in_protocols_policy::check(protocol);
      }

   };

   template<typename shared_ptr_type> struct no_deleter
   {
      no_deleter(){}
      ~no_deleter(){}
      no_deleter(no_deleter const&){}
      no_deleter& operator = (no_deleter const&){return *this;}
      void operator()(typename shared_ptr_type::element_type* p){}
   };

   template<byte command_tt
      , typename data_tt
      , typename protocol_tt = typename select_protocol_by_command<typename byte_type<command_tt>::type>::type
      , typename allowed_in_protocols_policy = protocols::less_or_equeal_protocol<protocol_tt> >
   class static_command: public command_interface, public static_command_base<command_tt, data_tt, protocol_tt>, public boost::enable_shared_from_this<static_command<command_tt, data_tt, protocol_tt> >
   {
   public:
      typedef static_command_base<command_tt, data_tt, protocol_tt> static_base;
      static_command()
      {
      }
      virtual void write(command_interface::writer_type writer, protocols::id<>::value_type protocol)
      {
         static_base::write(writer, protocol);
      }
      virtual std::size_t size(protocols::id<>::value_type protocol)
      {
         return static_base::size(protocol);
      }
      virtual bool allowed_for_protocol(protocols::id<>::value_type protocol)
      {
         return static_base::allowed_for_protocol(protocol);
      }
      operator pointer()
      {
         return pointer(this, no_deleter<pointer>());
      }
   };

}//namespace si
