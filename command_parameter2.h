//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_auxilary.h"
#include "tuple_type.h"
#include "create_parameter_sequence.h"
#include "sequence_position.h"

#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/mpl/deque.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/minus.hpp>


namespace si
{

	template<typename description_tt
				, typename T>struct parameters_array2
			: public boostext::tuple_type<typename create_parameter_sequence<description_tt>::type >::type
			, public parameter
	{
		template<typename description, typename enabled = void> struct get_description_item_length
				: public boost::mpl::integral_c<unsigned, 1>
		{};
		template<typename description> struct get_description_item_length
				<description, typename boost::enable_if<typename boost::is_base_of<has_byte_size, description>::type>::type>
				: public boost::mpl::integral_c<unsigned, description::size::value>
		{};
		template<typename description, bool enabled = boost::mpl::empty<description>::value> struct get_description_length
				: public boost::mpl::integral_c<unsigned
				, get_description_item_length<typename boost::mpl::front<description>::type>::value
				+ get_description_length<typename boost::mpl::pop_front<description>::type>::value >
		{};
		template<typename description> struct get_description_length<description, true>
				: public boost::mpl::integral_c<unsigned, 0>
		{};
		typedef description_tt description_type;
		typedef typename boostext::tuple_type<typename create_parameter_sequence<description_tt>::type >::type base_tuple_type;
		typedef typename boost::mpl::integral_c<std::size_t, get_description_length<description_type>::value>  description_size_type;
		typedef T type;
		typedef T parameter_type;

		typedef typename create_parameter_sequence<description_tt>::type parameters_type;

		template<typename parameter_tt> typename boost::tuples::element<boostext::sequence_position<typename parameter_tt::parameter_type, parameters_type>::value, base_tuple_type>::type & get()
		{
			return base_tuple_type::template get<boostext::sequence_position<typename parameter_tt::parameter_type, parameters_type>::value>();
		}

		template<typename description_part, bool enabled = boost::is_base_of<parameter, description_part>::value> struct item_data_writer
		{
			template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				that->get<typename description_part::parameter_type>().write_data(size, it);
			}
			template<typename that_type, typename iterator_t>static inline void read_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				that->get<typename description_part::parameter_type>().read_data(size, it);
			}
		};
		template<typename description_part> struct item_data_writer<description_part, false>
		{
			template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				it += description_part::size::value;
			}
			template<typename that_type, typename iterator_t>static inline void read_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				it += description_part::size::value;
			}
		};


		template<typename description, bool empty = boost::mpl::empty<description>::value> struct data_writer
		{
			typedef typename boost::mpl::front<description>::type current_part;

			template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				item_data_writer<current_part>::write_data(that, size, it);
				return data_writer<typename boost::mpl::pop_front<description>::type>::write_data(that, size, it);
			}
			template<typename that_type, typename iterator_t>static inline bool read_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				item_data_writer<current_part>::read_data(that, size, it);
				return data_writer<typename boost::mpl::pop_front<description>::type>::read_data(that, size, it);
			}
		};
		template<typename description> struct data_writer<description
				, true >
		{
			template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				return;
			}
			template<typename that_type, typename iterator_t>static inline bool read_data(that_type *that, std::size_t &size, iterator_t &it)
			{
				return true;
			}
		};

		static inline std::size_t get_size()
		{
			return description_size_type::value;
		}
		template<typename iterator_t> static inline bool can_read_data(std::size_t &size, iterator_t &it)
		{
			return true;
		}
		template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t size, iterator_t & it)
		{
			return get_size();
		}
		template<typename iterator_t>inline void write_data(std::size_t &size, iterator_t &it )
		{
			data_writer<description_type>::write_data(this, size, it);
		}
		template<typename iterator_t>inline bool read_data(std::size_t &size, iterator_t &it)
		{
			data_writer<description_type>::read_data(this, size, it);
			return true;
		}
	};




}//namespace si
