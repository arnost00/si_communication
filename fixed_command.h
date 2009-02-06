#pragma once

#include "si_constants.h"
#include "tuple_type.h"
#include "sequence_position.h"
#include "protocol_encoders.h"
#include "command_interface.h"

#include <boost/shared_array.hpp>

namespace si
{
   template<byte command_tt
      , typename parameters_tt = boost::mpl::deque<>
      , bool control_sequence_tt = false
         > struct fixed_command
      : public boostext::tuple_type<typename create_parameter_sequence<parameters_tt>::type>::type
      , public command_interface
   {
      BOOST_STATIC_CONSTANT(byte, code = command_tt);

      BOOST_STATIC_CONSTANT(bool, control_sequence = control_sequence_tt);

      typedef parameters_tt command_template_type;
      typedef typename create_parameter_sequence<parameters_tt>::type command_parameters_type;
      typedef typename boostext::tuple_type<typename create_parameter_sequence<parameters_tt>::type>::type parameters_tuple_type;

      typedef boost::shared_ptr<fixed_command> pointer;

      typedef fixed_command this_type;
		template<typename parameter_tt>struct element
		{
			typedef boost::mpl::integral_c<unsigned, boostext::sequence_position<typename parameter_tt::parameter_type, command_parameters_type>::value> type;
		};

      template<typename parameter_tt> typename parameter_tt::type & get()
      {
//          static typename parameter_tt::type tmp;
//          return tmp = parameters_tuple_type::template get<0>();
         return *(typename parameter_tt::type*)&parameters_tuple_type::template get<boostext::sequence_position<typename parameter_tt::parameter_type, command_parameters_type>::value>();
      }

      virtual id_type get_id(protocols::id<>::value_type protocol = protocols::id<>::value)
      {
         return code;
      }
      virtual void get_data(std::size_t &size, data_type& data, protocols::id<>::value_type protocol = protocols::id<>::value)
      {
         size = get_size();
         data.reset(new si::byte[size]);
         data_type::element_type *it = data.get();

         raw_data_writer<this_type>::write_data(this, size, it);

         return;
      }

      std::size_t get_size(protocols::id<>::value_type protocol = protocols::id<>::value)
      {
         return parameters_size_counter<parameters_tt, this_type>::get_size(this);
      }

      virtual bool is_control_sequence()
      {
         return control_sequence_tt;
      }

      virtual bool accept_data(std::size_t size, data_type data)
      {
         boost::add_pointer<data_type::element_type>::type it = data.get();
         return raw_data_reader<this_type>::read_data(this, size, it);
      }
      virtual bool can_accept_data(std::size_t size, data_type data)
      {
         boost::add_pointer<data_type::element_type>::type it = data.get();
         return raw_data_reader<this_type>::can_read_data(this, size, it);
      }
      static inline bool can_accept_data_static(std::size_t size, data_type data)
      {
         boost::add_pointer<data_type::element_type>::type it = data.get();
         return raw_data_reader<this_type>::can_read_data_static(size, it);
      }

   };
};//namespace si
