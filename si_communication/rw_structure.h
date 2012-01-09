//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_constants.h"
#include "tuple_type.h"
#include "sequence_position.h"
#include "protocol_encoders.h"
#include "command_interface.h"

#include <boost/shared_array.hpp>

namespace si
{
	template<boost::uint8_t command_tt
				, typename parameters_tt = boost::deque<>
				> struct rw_structure
			: public boostext::tuple_type<typename create_parameter_sequence<parameters_tt>::type>::type
	{

		typedef parameters_tt structure_template_type;
		typedef typename create_parameter_sequence<parameters_tt>::type parameters_type;
		typedef typename boostext::tuple_type<typename create_parameter_sequence<parameters_tt>::type>::type parameters_tuple_type;

		typedef boost::shared_ptr<rw_structure> pointer;

		typedef rw_structure this_type;
		template<typename parameter_tt>struct element
		{
			typedef boost::mpl::integral_c<unsigned, boostext::sequence_position<typename parameter_tt::parameter_type, command_parameters_type>::value> type;
		};

		template<typename parameter_tt> typename parameter_tt::type & get()
		{
			return *(typename parameter_tt::type*)&parameters_tuple_type::get<boostext::sequence_position<typename parameter_tt::parameter_type, command_parameters_type>::value>();
		}

		std::size_t get_size(protocols::id<>::value_type protocol = protocols::id<>::value)
		{
			return parameters_size_counter<parameters_tt, this_type>::get_size(this);
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
