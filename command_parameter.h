//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_auxilary.h"
#include "tuple_type.h"
#include "create_parameter_sequence.h"
#include "sequence_position.h"
#include "type_forwarder.h"

#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/mpl/deque.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/minus.hpp>

#include <boost/cstdint.hpp>

namespace si
{

	typedef boost::mpl::deque<boost::int8_t, boost::int16_t, boost::int32_t, boost::int64_t> signed_integers;
	typedef boost::mpl::deque<boost::uint8_t, boost::uint16_t, boost::uint32_t, boost::uint64_t> unsigned_integers;
	struct value_not_fixed{};


	template <unsigned size
		, typename integer_types = unsigned_integers
		, bool empty =  boost::mpl::empty<integer_types>::value>
	struct decide_integer_size;

	template<unsigned size
		, typename integer_types
		, bool match_found = (size <= sizeof(typename boost::mpl::front<integer_types>::type))>
	struct decision
	{
		typedef typename decide_integer_size<size, typename boost::mpl::pop_front<integer_types>::type>::type type;
	};
	template<unsigned size
		, typename integer_types>
		struct decision<size, integer_types, true>
	{
	   typedef typename boost::mpl::front<integer_types>::type type;
	};

	template <unsigned size
		, typename integer_types
		, bool empty>
	struct decide_integer_size
	{
		typedef typename decision<size, integer_types>::type type;
	};

	template <unsigned size
		, typename integer_types> 
	struct decide_integer_size<size, integer_types, true>
	{
		BOOST_MPL_ASSERT_MSG(false, NO_TYPE_OF_SUITABLE_SIZE_FOUND, (types<typename boost::mpl::integral_c<unsigned, size> >));
	};

	template <typename value_type_tt, typename value_tv> struct value_holder_static
	{
		typedef value_type_tt value_type;
		typedef value_holder_static<value_type_tt, value_tv> value_holder_type;
		BOOST_STATIC_CONSTANT(value_type, value = value_tv::value);
		operator value_type()
		{
			return value;
		}
	};

	template <typename value_type_tt> struct value_holder
	{
	   typedef value_type_tt value_type;
	   typedef value_holder<value_type_tt> value_holder_type;
	   value_type value;

	   value_holder()
		  : value(0)
		{}
		value_holder &operator = (value_type new_value)
		{
			value = new_value;
			return *this;
		}
		operator value_type()
		{
			return value;
		}
	}; 
	template <unsigned ordinal, typename base_type>
	struct byte_part: public byte_part<ordinal - 1, base_type>
	{
		typedef byte_part type;
		static inline std::size_t get_size()
		{
			return 1;
		}
		template<typename iterator_t> static inline bool can_read_data(std::size_t &size, iterator_t &it)
		{
			return true;
		}
		template<typename iterator_t> static inline std::size_t get_read_data_size(std::size_t size, iterator_t & it)
		{
			return get_size();
		}

		template<typename iterator_t>inline void write_data(std::size_t &, iterator_t &it)
		{
			*it++ = (base_type::value >> (ordinal << 3) ) & 0xFF;
		}
		template<typename iterator_t>inline bool read_data(std::size_t &, iterator_t &it)
		{
			typename base_type::value_type new_part = typename base_type::value_type(0xFF) << (ordinal << 3);
			base_type::value &= ~new_part;
			new_part = typename base_type::value_type((*it++) & 0xFF) << (ordinal << 3);
			base_type::value |= new_part;
			return true;
		}
	};
	template <typename base_type> struct byte_part<0, base_type>: public base_type
	{
		typedef byte_part type;
		static inline std::size_t get_size()
		{
			return 1;
		}
		template<typename iterator_t> static inline bool can_read_data(std::size_t &size, iterator_t &it)
		{
			return true;
		}
		template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t size, iterator_t & it)
		{
			return get_size();
		}
		template<typename iterator_t>inline void write_data(std::size_t &, iterator_t &it )
		{
			*it++ = base_type::value & 0xFF;
		}
		template<typename iterator_t>inline bool read_data(std::size_t &, iterator_t &it )
		{
			typename base_type::value_type new_part = typename base_type::value_type(0xFF);
			base_type::value &= ~new_part;
			new_part = typename base_type::value_type((*it++) & 0xFF);
			base_type::value |= new_part;
			return true;
		}
	};

		template <unsigned size_tt, typename T, typename fixed_value = value_not_fixed/*, typename implementation_candidates = unsigned_integers*/>
                struct unsigned_integral_parameter
		: public parameter
		, public byte_part<size_tt - 1, typename boost::mpl::if_< typename boost::is_same<fixed_value, value_not_fixed>::type
															, type_forwarder<value_holder<typename decide_integer_size<size_tt, unsigned_integers>::type> >
															, value_holder_static<type_forwarder<typename decide_integer_size<size_tt, unsigned_integers>::type>
																, fixed_value > >::type::type
							>
	{
		typedef T type;
		typedef T parameter_type;
		typedef unsigned_integral_parameter<size_tt, T/*, implementation_candidates*/> this_type;
		typedef typename decide_integer_size<size_tt, unsigned_integers>::type decided_type;
		typedef byte_part<size_tt - 1
			, value_holder<decided_type> > byte_parts;
		typedef boost::integral_constant<unsigned, size_tt> parameter_size_type;

//		typedef unsigned_integers implementation_candidates;
		 
		template <unsigned ordinal> struct byte: public parameter
		{
			BOOST_MPL_ASSERT_MSG(ordinal < size_tt, REQUESTED_PART_IS_OUT_OF_RANGE, (types<typename boost::mpl::integral_c<unsigned, ordinal>, typename boost::mpl::integral_c<unsigned, size_tt> >));
			typedef byte_part<ordinal, typename this_type::value_holder_type> type;
			typedef T parameter_type;
		};
		template <unsigned ordinal_begin, unsigned ordinal_end = ordinal_begin> struct bits_range
		 : public parameter
		 , public has_bit_rw
		{
			typedef typename unsigned_integral_parameter<size_tt, T>::template bits_range<ordinal_begin, ordinal_end> type;
			template<unsigned ordinal_begin_gbc, unsigned ordinal_end_gbc, bool bigger_first = (ordinal_end_gbc < ordinal_begin_gbc)> struct get_bits_count
			{
				typedef boost::mpl::integral_c<unsigned, ordinal_begin_gbc - ordinal_end_gbc +1> type;
			};
			template<unsigned ordinal_begin_gbc, unsigned ordinal_end_gbc> struct get_bits_count<ordinal_begin_gbc, ordinal_end_gbc, false>
			{
				typedef boost::mpl::integral_c<unsigned, ordinal_end_gbc - ordinal_begin_gbc +1> type;
			};
			BOOST_MPL_ASSERT_MSG((ordinal_begin < (size_tt << 3)) && (ordinal_end < (size_tt << 3)), REQUESTED_PART_IS_OUT_OF_RANGE, (types<typename boost::mpl::integral_c<unsigned, ordinal_begin>, typename boost::mpl::integral_c<unsigned, ordinal_begin>, typename boost::mpl::integral_c<unsigned, size_tt> >));
			typedef T parameter_type;
			typedef typename get_bits_count<ordinal_begin, ordinal_end>::type bit_size;
			static inline std::size_t get_size()
			{
				return ordinal_begin < ordinal_end? ordinal_end - ordinal_begin + 1: ordinal_begin - ordinal_end + 1;
			}
			template<typename iterator_t> static inline bool can_read_data(std::size_t &size, iterator_t &it)
			{
				return true;
			}
			template<typename iterator_t> static inline std::size_t get_read_data_size(std::size_t size, iterator_t & it)
			{
				return get_size();
			}

			template<typename iterator_t>static void write_bits_data(this_type *, std::size_t &, iterator_t &it, unsigned &bit_offset)
			{
				int step = ordinal_begin <= ordinal_end? +1: -1;
				unsigned i = ordinal_begin;
				typename this_type::value_type mask;
				bool bit_value;

				while(true)
				{
                    mask = typename this_type::value_type(0x01) << i;
					bit_value = 0 != (bit_offset & *it);


					if(bit_value)
					{
						*it |= bit_offset;
					}
					else
					{
						*it &= ~bit_offset;
					}
					if(0 == bit_offset)
					{
						bit_offset = boost::uint8_t(1) << 7;
						it++;
					}
					else
						bit_offset >>= 1;

					if(ordinal_begin == ordinal_end)
						break;
					i += step;
				}
				return;
			 }
			 template<typename iterator_t>static bool read_bits_data(this_type *that, std::size_t &size, iterator_t &it, unsigned &bit_offset)
			 {
				int step = ordinal_begin <= ordinal_end? +1: -1;
				unsigned i = ordinal_begin;
				typedef typename this_type::value_type this_value_type;
				this_value_type mask;
				bool bit_value;

				while(true)
				{
					bit_value = (0 != (bit_offset & *it));

					mask = this_value_type(0x01) << i;

					if(bit_value)
					{
						that->value |= mask;
					}
					else
					{
						that->value &= ~mask;
					}

					bit_offset >>= 1;
					if(0 == bit_offset)
					{
						bit_offset = 1;
						bit_offset <<= 7;
						size --;
						it++;
					}

					if(i == ordinal_end)
						break;
					i += step;
				}
			return true;
			}
		};
      template<unsigned byte_part_tt, typename for_happy_compilers = void> struct data_writer
      {
         template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it )
         {
			((typename this_type::template byte<byte_part_tt>::type*)(that))->write_data(size, it);
            data_writer<byte_part_tt - 1>::write_data(that, size, it);
         }         
         template<typename that_type, typename iterator_t>static inline bool read_data(that_type *that, std::size_t &size, iterator_t &it )
         {
			((typename this_type::template byte<byte_part_tt>::type*)(that))->read_data(size, it);
            data_writer<byte_part_tt - 1>::read_data(that, size, it);
            return true;
         }
      };
      template<typename for_happy_compilers> struct data_writer<0, for_happy_compilers>
      {
         template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it )
         {
			((typename this_type::template byte<0>::type*)(that))->write_data(size, it);
         }         
         template<typename that_type, typename iterator_t>static inline bool read_data(that_type *that, std::size_t &size, iterator_t &it )
         {
			((typename this_type::template byte<0>::type*)(that))->read_data(size, it);
            return true;
         }
      };

      static inline std::size_t get_size()
      {
         return size_tt;
      }
      template<typename iterator_t> static inline bool can_read_data(std::size_t &size, iterator_t &it)
      {
         return true;
      }
	  template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t , iterator_t & )
      {
         return get_size();
      }
      template<typename iterator_t>inline void write_data(std::size_t &size, iterator_t &it )
      {
         data_writer<size_tt - 1>::write_data(this, size, it);
      }         
      template<typename iterator_t>inline bool read_data(std::size_t &size, iterator_t &it )
      {
         data_writer<size_tt - 1>::read_data(this, size, it);
         return true;
      }         
   };
   template <typename T> struct unsigned_integral_parameter<0, T>
      : public parameter
   {
      typedef T type;
      typedef T parameter_type;
      typedef unsigned_integral_parameter<0, T> this_type;

      static inline std::size_t get_size()
      {
         return 0;
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
      }         
      template<typename iterator_t>inline bool read_data(std::size_t &size, iterator_t &it )
      {
         return true;
      }         
   };

	struct bit: public parameter
	{
		BOOST_STATIC_CONSTANT(std::size_t, size = 1);
	};
	template<std::size_t size_tp> struct bits: public parameter
	{
		BOOST_STATIC_CONSTANT(std::size_t, size = size_tp);
	};
   namespace bit_array_internal
   {
      template<typename bits_description, typename enabled = void> struct get_description_item_length
         : public boost::mpl::integral_c<unsigned, 1>
      {};
      template<typename bits_description> struct get_description_item_length
         <bits_description, typename boost::enable_if<typename boost::is_base_of<has_bit_size, bits_description>::type>::type>
         : public boost::mpl::integral_c<unsigned, bits_description::bit_size::value>
      {};
      template<typename bits_description, bool enabled = boost::mpl::empty<bits_description>::value> struct get_description_length
         : public boost::mpl::integral_c<unsigned
         , get_description_item_length<typename boost::mpl::front<bits_description>::type>::value 
         + get_description_length<typename boost::mpl::pop_front<bits_description>::type>::value >
      {
      };
      template<typename bits_description> struct get_description_length
         <bits_description, true>
         : public boost::mpl::integral_c<unsigned, 0>
      {
      };
      template<typename original_description> struct normalize_description 
      {
         typedef typename boost::mpl::if_c<
            ((get_description_length<original_description>::value >> 3) << 3) == get_description_length<original_description>::value
            , original_description
            , normalize_description<typename boost::mpl::push_front<original_description, don_t_care<> >::type> >::type::type type;
      };
      template<typename output_sequence, typename parameter_to_add, typename is_parameter = typename boost::is_base_of<parameter, parameter_to_add>::type> struct add_bits_to_sequence
      {
         typedef typename boost::mpl::push_front<output_sequence, typename parameter_to_add::parameter_type>::type type;
      };
      template<typename output_sequence, typename parameter_to_add> struct add_bits_to_sequence<output_sequence, parameter_to_add, boost::false_type>
      {
         typedef output_sequence type;
      };
      template<typename input_sequence, bool enabled = boost::mpl::empty<input_sequence>::value> struct create_bits_sequence
      {
         typedef typename boost::mpl::if_<typename boost::is_base_of<parameter, typename boost::mpl::front<input_sequence>::type>::type
            , typename add_bits_to_sequence<typename create_bits_sequence<typename boost::mpl::pop_front<input_sequence>::type>::type, typename boost::mpl::front<input_sequence>::type >::type
            , typename create_bits_sequence<typename boost::mpl::pop_front<input_sequence>::type>::type >::type type;
//         BOOST_MPL_ASSERT_MSG(false, CREATE_BITS_SEQUENCE_MSG, (types<type>));
      };
      template<typename input_sequence> struct create_bits_sequence<input_sequence
         , true>
      {
         typedef typename boost::mpl::deque<> type;
      };
   }

   template<typename description_tt
      , typename T
      >struct bit_array
      : public boostext::tuple_type<typename create_parameter_sequence<description_tt>::type >::type
	  , public parameter
   {
      
      typedef typename bit_array_internal::normalize_description<description_tt>::type description_type;
      typedef typename boost::mpl::integral_c<std::size_t, (bit_array_internal::get_description_length<description_type>::value >> 3)>  description_size_type;
      typedef T type;
      typedef T parameter_type;

      typedef typename create_parameter_sequence<description_tt>::type bit_parameters_type;

		typedef typename bit_array_internal::create_bits_sequence<description_tt>::type parametrized_bits;
      typedef typename boostext::tuple_type<bit_parameters_type>::type base_tuple_type;

/*
      template<typename parameters_tt, unsigned position> struct parameter_initializer 
      {
         template<typename t> struct the_type{typedef t type;};
         typedef typename boost::mpl::if_c< (position >= boost::mpl::size<parameters_tt>::value)
            , the_type<boost::tuples::null_type>
            , typename boost::mpl::at_c<parameters_tt, position> >::type::type initializer_type;
         template<typename initializer_type>struct init_param
         {
            inline static initializer_type get_init(typename array_type_base::value_type &value)
            {
               return initializer_type(value);
            }
         };

         template<>struct init_param<boost::tuples::null_type>
         {
            inline static initializer_type get_init(...)
            {
               return initializer_type();
            }
         };
         inline static initializer_type get_init(typename array_type_base::value_type &value)
         {
            return init_param<initializer_type>::get_init(value);
         }
      };
      bit_array()
         : base_tuple_type(
				  parameter_initializer<parametrized_bits, 0>::get_init(array_type::value)
            , parameter_initializer<parametrized_bits, 1>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 2>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 3>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 4>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 5>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 6>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 7>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 8>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value)
            , parameter_initializer<parametrized_bits, 9>::get_init(integral_parameter_internal::value_holder<typename decide_integer_size<(boost::mpl::size<description_type>::value >> 3), unsigned_integers>::type >::value))
      {}
*/
      template<typename parameter_tt> typename boost::tuples::element<boostext::sequence_position<typename parameter_tt::parameter_type, bit_parameters_type>::value, base_tuple_type>::type & get()
      {
         return base_tuple_type::template get<boostext::sequence_position<typename parameter_tt::parameter_type, bit_parameters_type>::value>();
      }

      template<typename description_part, typename enabled = void> struct item_data_bit_size_decicer
         : public boost::mpl::integral_c<std::size_t, 1>
      {};
      template<typename description_part> struct item_data_bit_size_decicer<description_part
         , typename boost::enable_if<typename boost::is_base_of<has_bit_size, description_part>::type>::type>
         : public description_part::bit_size
      {};


      template<typename description_part, typename enabled = void> struct item_data_writer 
      {
         template<typename iterator_t>static inline void skip(std::size_t &size, iterator_t &it, unsigned &bit_offset)
         {
            it += item_data_bit_size_decicer<description_part>::value >> 3;
            size -= item_data_bit_size_decicer<description_part>::value >> 3;
            for(unsigned offset_shift = item_data_bit_size_decicer<description_part>::value % 8; 0 < offset_shift; offset_shift--)
            {
               bit_offset >>= 1;
               if(0 == bit_offset)
               {
                  it++;
                  size --;
                  bit_offset = 1 << 7;
               }
            }
         }
		 template<typename that_type, typename iterator_t>static inline void write_data(that_type *, std::size_t &size, iterator_t &it, unsigned &bit_offset)
         {
            skip(size, it, bit_offset);
         }         
		 template<typename that_type, typename iterator_t>static inline void read_data(that_type *, std::size_t &size, iterator_t &it, unsigned &bit_offset)
         {
            skip(size, it, bit_offset);
         }         
      };
      template<typename description_part> struct item_data_writer<description_part, typename boost::enable_if<typename boost::is_base_of<has_bit_rw, description_part>::type>::type>
      {
//         BOOST_MPL_ASSERT_MSG(false, DESCRIPTION_PART_DEBUGGING, (types<description_part>));
         template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it, unsigned &bit_offset)
         {
            description_part::write_bits_data(&that->get<typename description_part::parameter_type>(), size, it, bit_offset);
         }         
         template<typename that_type, typename iterator_t>static inline void read_data(that_type *that, std::size_t &size, iterator_t &it, unsigned &bit_offset)
         {
            description_part::read_bits_data(&that->get<typename description_part::parameter_type>(), size, it, bit_offset);
         }         
      };

	template<typename description, bool empty = boost::mpl::empty<description>::value> struct data_writer
	{
		typedef typename boost::mpl::front<description>::type current_part;
//		BOOST_MPL_ASSERT_MSG(false, DATA_WRITER_DBG, (types<description, typename boost::mpl::size<description>::type, typename boost::mpl::integral_c<bool, empty> >));

		template<typename that_type, typename iterator_t>static inline void write_data(that_type *that, std::size_t &size, iterator_t &it, unsigned &bit_offset)
		{
			item_data_writer<current_part>::write_data(that, size, it, bit_offset);
			return data_writer<typename boost::mpl::pop_front<description>::type>::write_data(that, size, it, bit_offset);
		}
		 template<typename that_type, typename iterator_t>static inline bool read_data(that_type *that, std::size_t &size, iterator_t &it, unsigned &bit_offset)
		{
			item_data_writer<current_part>::read_data(that, size, it, bit_offset);
			return data_writer<typename boost::mpl::pop_front<description>::type>::read_data(that, size, it, bit_offset);
		}
	};
      template<typename description> struct data_writer<description
		 , true >
      {
//		BOOST_MPL_ASSERT_MSG(false, DATA_WRITER_DBG, (types<description, typename boost::mpl::empty<description>::type>));
		 template<typename that_type, typename iterator_t>static inline void write_data(that_type *, std::size_t &, iterator_t &, unsigned &)
         {
            return;
         }         
		 template<typename that_type, typename iterator_t>static inline bool read_data(that_type *, std::size_t &, iterator_t &, unsigned &)
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
	  template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t , iterator_t & )
      {
         return get_size();
      }
      template<typename iterator_t>inline void write_data(std::size_t &size, iterator_t &it )
      {
         unsigned bit_offset = 1 << 7;
         data_writer<description_type>::write_data(this, size, it, bit_offset);
      }         
      template<typename iterator_t>inline bool read_data(std::size_t &size, iterator_t &it)
      {
         unsigned bit_offset = 1 << 7;
         data_writer<description_type>::read_data(this, size, it, bit_offset);
         return true;
      } 
   };

	template<typename description_tt
		/*, typename T*/>struct parameters_array
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
//		typedef T type;
//		typedef T parameter_type;

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
				template<typename that_type, typename iterator_t>static inline void read_data(that_type *, std::size_t &, iterator_t &it)
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
				template<typename that_type, typename iterator_t>static inline bool read_data(that_type *, std::size_t &, iterator_t &)
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


   template<typename parameters_tt, typename that_type, bool empty = boost::mpl::empty<parameters_tt>::value> struct parameters_size_counter
   {
      template<typename parameter_tt, bool is_parameter = boost::is_base_of<parameter, parameter_tt>::value > struct parameter_size
      {
         static inline std::size_t get_size(that_type* that)
         {
            return 1;
         }
      };
      template<typename parameter_tt> struct parameter_size<parameter_tt, true>
      {

         static inline std::size_t get_size(that_type* that)
         {
            return that->template get<parameter_tt>().get_size();
         }
      };
      static inline std::size_t get_size(that_type* that)
      {
         return parameter_size<typename boost::mpl::front<parameters_tt>::type>::get_size(that)
            + parameters_size_counter<typename boost::mpl::pop_front<parameters_tt>::type, that_type >::get_size(that);
      }
   };

   template<typename parameters_tt, typename that_type> struct parameters_size_counter<parameters_tt, that_type, true/*typename boost::enable_if<typename boost::mpl::empty<parameters_tt>::type >::type*/ >
   {
	  static inline std::size_t get_size(that_type* )
      {
         return 0;
      }
   };


	template<typename T, std::size_t length = unknown_size> struct byte_array
		: public parameter
		, public std::vector<boost::uint8_t>
   {
		typedef std::vector<boost::uint8_t> storage_base_type;
		typedef T type;
		typedef T parameter_type;

      byte_array()
      {
         if(unknown_size!= length)
         {
            resize(length);
         }
      }
	  byte_array(byte_array const &that)
		  : storage_base_type(that)
	  {
	  }

	  typedef std::vector<boost::uint8_t> value_type;

      static inline std::size_t get_size()
      {
         return length;
      }
	  template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t , iterator_t & )
      {
         return get_size();
      }
      template<typename iterator_t>inline void write_data(std::size_t &out_size, iterator_t &it )
      {
         const_iterator my_it;
         for(std::size_t i = 0; i < out_size; i++, it++, my_it++)
         {
            *it = *my_it;
         }
      }
      template<typename iterator_t>inline static bool can_read_data(std::size_t &out_size, iterator_t &it )
	  {
		  //TODO: check data size
         return true;
      }         
      template<typename iterator_t>inline bool read_data(std::size_t &out_size, iterator_t &it )
      {
         resize(out_size);
         iterator my_it = begin();
         for(std::size_t i = 0; i < out_size; i++, it++, my_it++)
         {
            *my_it = *it;
         }
			return true;
      }         
   };

   template<typename T> struct byte_array<T, unknown_size>
      : public parameter
	  , public std::vector<boost::uint8_t>
   {
	  typedef std::vector<boost::uint8_t> value_type;
	  typedef T type;
	  typedef T parameter_type;

	  byte_array()
		  :value_type(0)
      {
      }

	  inline std::size_t get_size()
      {
         return size();
      }
      template<typename iterator_t>static inline std::size_t get_read_data_size(std::size_t size, iterator_t & it)
      {
         return unknown_size;
      }

      template<typename iterator_t>inline void write_data(std::size_t &out_size, iterator_t &it )
      {
         const_iterator my_it;
         for(std::size_t i = 0; i < out_size; i++, it++, my_it++)
         {
            *it = *my_it;
         }
      }
      template<typename iterator_t>inline static bool can_read_data(std::size_t &out_size, iterator_t &it )
      {
         return true;
      }         
      template<typename iterator_t>inline bool read_data(std::size_t &out_size, iterator_t &it )
      {
         resize(out_size);
         iterator my_it = begin();
         for(std::size_t i = 0; i < out_size; i++, it++, my_it++)
         {
            *my_it = *it;
         }
         return true;
      }         
   };

   template<typename that_type, typename command_template_tt = typename that_type::command_template_type, class enabled = void> struct raw_data_writer
   {
      template<typename item_t, typename enabled_iw = typename boost::is_base_of<parameter, item_t>::type> struct item_writer
      {
         template<typename iterator_t>static inline void write_data(that_type* that, std::size_t size, iterator_t &it)
         {
            *it++ = item_t::value;
         }         
      };
      template<typename item_t>struct item_writer<item_t
         , boost::true_type>
      {
         template<typename iterator_t>static inline void write_data(that_type* that, std::size_t size, iterator_t & it)
         {
            that->template get<item_t>().write_data(size, it);
         }         
      };
      template<typename iterator_t>static inline void write_data(that_type* that, std::size_t size, iterator_t & it)
      {
         item_writer<typename boost::mpl::front<command_template_tt>::type>::write_data(that, size, it);
         raw_data_writer<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::write_data(that, size, it);
      }
   };
   template<typename that_type, typename command_template_tt> struct raw_data_writer<that_type, command_template_tt, typename boost::enable_if<typename boost::mpl::empty<command_template_tt>::type>::type>
   {
	  template<typename iterator_t>static inline void write_data(that_type* , std::size_t , iterator_t &)
      {
      }
   };

   template<typename that_type, typename command_template_tt = typename that_type::command_template_type, class enabled = void> struct raw_data_reader
   {
      template<typename item_t, class enabled_ir = void> struct item_reader
      {
         template<typename iterator_t>static inline std::size_t get_read_data_size_static(std::size_t size, iterator_t & it)
         {
            return 1;
         }         
         template<typename iterator_t>static inline bool can_read_data_static(std::size_t size, iterator_t &it)
         {
            return *it++ == item_t::value;
         }         
         template<typename iterator_t>static inline std::size_t get_read_data_size(that_type* that, std::size_t size, iterator_t & it)
         {
            return get_read_data_size_static(size, it);
         }         
         template<typename iterator_t>static inline bool can_read_data(that_type* that, std::size_t size, iterator_t &it)
         {
            return can_read_data_static(size, it);
         }         
         template<typename iterator_t>static inline bool read_data(that_type* that, std::size_t size, iterator_t &it)
         {
            return *it++ == item_t::value;
         }         
      };
      template<typename item_t>struct item_reader<item_t
         , typename boost::enable_if<typename boost::is_base_of<parameter, item_t>::type>::type>
      {
         template<typename iterator_t>static inline std::size_t get_read_data_size(that_type* that, std::size_t size, iterator_t & it)
         {
            return that->template get<item_t>().get_read_data_size(size, it);
         }         
         template<typename iterator_t>static inline bool can_read_data(that_type* that, std::size_t size, iterator_t & it)
         {
            return that->template get<item_t>().can_read_data(size, it);
         }         
         template<typename iterator_t>static inline std::size_t get_read_data_size_static(std::size_t size, iterator_t & it)
         {
            return item_t::get_read_data_size(size, it);
         }         
         template<typename iterator_t>static inline bool can_read_data_static(std::size_t size, iterator_t & it)
         {
            return item_t::can_read_data(size, it);
         }         
         template<typename iterator_t>static inline bool read_data(that_type* that, std::size_t size, iterator_t & it)
         {
            return that->template get<item_t>().read_data(size, it);
         }         
      };

      template<typename iterator_t>static inline std::size_t get_read_data_size(that_type* that, std::size_t size, iterator_t & it)
      {
         std::size_t counted_size;
         if(unknown_size == (counted_size = item_reader<typename boost::mpl::front<command_template_tt>::type>::get_read_data_size(that, size, it)))
         {
            if(unknown_size == (counted_size = raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size(that, size, it)))
               return unknown_size;
            if(counted_size > size)
               return unknown_size;
            return size;
         }
         return counted_size + raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size(that, size -= counted_size, it);
      }
      template<typename iterator_t>static inline std::size_t get_read_data_size_static(std::size_t size, iterator_t & it)
      {
         std::size_t counted_size;
         if(unknown_size == (counted_size = item_reader<typename boost::mpl::front<command_template_tt>::type>::get_read_data_size_static(size, it)))
         {
            if(unknown_size == (counted_size = raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size_static(size, it)))
               return unknown_size;
            if(counted_size > size)
               return unknown_size;
            return size;
         }
         return counted_size + raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size_static(size -= counted_size, it);
      }
      template<typename iterator_t>static inline bool can_read_data(that_type* that, std::size_t size, iterator_t & it)
      {
         if(size != get_read_data_size(that, size, it))
         {
            return false;
         }
         std::size_t counted_size;
         if(unknown_size == (counted_size = item_reader<typename boost::mpl::front<command_template_tt>::type>::get_read_data_size(that, size, it)))
         {
            if(unknown_size == (counted_size = raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size(that, size, it)))
               return false;
            if(counted_size > size)
               return false;
         }
         if(!item_reader<typename boost::mpl::front<command_template_tt>::type>::can_read_data(that, counted_size, it))
            return false;
         return raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::can_read_data(that, size - counted_size, it);
      }

      template<typename iterator_t>static inline bool can_read_data_static(std::size_t size, iterator_t & it)
      {
         if(size != get_read_data_size_static(size, it))
         {
            return false;
         }
         std::size_t counted_size;
         if(unknown_size == (counted_size = item_reader<typename boost::mpl::front<command_template_tt>::type>::get_read_data_size_static(size, it)))
         {
            if(unknown_size == (counted_size = raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size_static(size, it)))
               return false;
            if(counted_size > size)
               return false;
         }
         if(!item_reader<typename boost::mpl::front<command_template_tt>::type>::can_read_data_static(counted_size, it))
            return false;
         return raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::can_read_data_static(size - counted_size, it);
      }

      template<typename iterator_t>static inline bool read_data(that_type* that, std::size_t size, iterator_t & it)
      {
         if(size != get_read_data_size(that, size, it))
         {
            return false;
         }
         std::size_t counted_size;
         if(unknown_size == (counted_size = item_reader<typename boost::mpl::front<command_template_tt>::type>::get_read_data_size(that, size, it)))
         {
            if(unknown_size == (counted_size = raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::get_read_data_size(that, size, it)))
               return false;
            if(counted_size > size)
               return false;
         }
         if(!item_reader<typename boost::mpl::front<command_template_tt>::type>::read_data(that, counted_size, it))
            return false;
         return raw_data_reader<that_type, typename boost::mpl::pop_front<command_template_tt>::type>::read_data(that, size - counted_size, it);
      }
   };

   template<typename that_type, typename command_template_tt> struct raw_data_reader<that_type, command_template_tt, typename boost::enable_if<typename boost::mpl::empty<command_template_tt>::type>::type>
   {
	  template<typename iterator_t>static inline std::size_t get_read_data_size(that_type* , std::size_t , iterator_t & )
      {
         return 0;
      }
      template<typename iterator_t>static inline std::size_t get_read_data_size_static(std::size_t size, iterator_t & it)
      {
         return 0;
      }
      template<typename iterator_t>static inline bool can_read_data_static(std::size_t size, iterator_t &it)
      {
         return true;
      }
      template<typename iterator_t>static inline bool can_read_data(that_type* that, std::size_t size, iterator_t & it)
      {
         return true;
      }
	  template<typename iterator_t>static inline bool read_data(that_type* , std::size_t , iterator_t &)
      {
         return true;
      }
   };


}//namespace si
