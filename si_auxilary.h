//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_constants.h"
#include <boost/mpl/if.hpp>
#include <boost/mpl/deque.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/pop_back.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/limits.hpp>

namespace si
{
	struct parameter{};
	struct multiparameter{};
	struct has_bit_size{};
	struct has_bit_rw: public has_bit_size{};
	struct has_byte_size{};

	template<std::size_t size_tp= 1> struct don_t_care: public has_bit_size
	{
		typedef boost::mpl::integral_c<std::size_t, size_tp> bit_size;
		typedef boost::mpl::integral_c<std::size_t, size_tp> size;
	};

   template<typename t> struct parameter_t: public parameter{typedef t parameter_type;};

   template<typename first, typename second, unsigned second_size = boost::mpl::size<second>::value> struct append_sequences
   {
      BOOST_MPL_ASSERT_MSG(boost::mpl::is_sequence<first>::type::value, SEQUENCE_REQUIERED_PARAM_1, ());
      BOOST_MPL_ASSERT_MSG(boost::mpl::is_sequence<second>::type::value, SEQUENCE_REQUIERED_PARAM_2, ());
         typedef typename append_sequences<typename boost::mpl::push_back<first, typename boost::mpl::front<second>::type>::type
         , typename boost::mpl::pop_front<second>::type >::type type;
   };
   template<typename first, typename second> struct append_sequences<first, second, 0>
   {
      typedef first type;
   };

   typedef append_sequences<boost::mpl::deque<>, boost::mpl::list_c<byte, 0x80, 0x05> >::type polynom_type;

      template <typename command_tt> struct select_protocol_by_command
   {
      BOOST_STATIC_CONSTANT(bool, is_basic_first_step = command_tt::value < 0x80);
      BOOST_STATIC_CONSTANT(bool, is_basic = is_basic_first_step || command_tt::value == 0xC4);
      typedef typename boost::mpl::if_c<is_basic
         , protocols::basic
         , protocols::extended>::type type;
   };


   template<typename value_tt, typename add_flag_tt> struct inc_seqence_if_flag
   {
      typedef typename boost::mpl::if_<add_flag_tt
         , typename boost::mpl::push_back<typename boost::mpl::pop_back<value_tt>::type
            ,  typename boost::mpl::plus<typename boost::mpl::back<value_tt>::type , byte_type<1> >::type>::type
         , value_tt>::type type;
   };

   template<typename value_tt, typename add_flag_tt> struct inc_if_flag
   {
      typedef typename boost::mpl::if_<add_flag_tt
         , typename boost::mpl::plus<value_tt, byte_type<1> >::type
         , value_tt>::type type;
   };

   template<typename first, typename second, typename result = boost::mpl::deque<>, unsigned rest_size = boost::mpl::size<first>::value>
      struct meta_xor
   {
      typedef typename meta_xor<typename boost::mpl::pop_front<first>::type
                  , typename boost::mpl::pop_front<second>::type
                  , typename boost::mpl::push_back<result
                        , byte_type< boost::mpl::front<first>::type::value
                                    ^ boost::mpl::front<second>::type::value> >::type >::type type;
   };

   template<typename first, typename second, typename result>
      struct meta_xor<first, second, result, 0>
   {
      BOOST_MPL_ASSERT_MSG(boost::mpl::size<first>::value == boost::mpl::size<second>::value, EQUAL_LENGTH_SEQUENCES_REQUIRED_FOR_XOR, (first, second));
      typedef result type;
   };

   template<typename value_tt> struct shift_left_integral_c: public boost::mpl::integral_c<typename value_tt::value_type, value_tt::value << 1 >
   {
   };

   template<typename value, typename flag_tt = boost::mpl::false_, typename shifted_value = boost::mpl::deque<>, unsigned size = boost::mpl::size<value>::value> 
      struct shift_left
   {
      typedef typename si::shift_left<typename boost::mpl::pop_back<value>::type
         , typename boost::mpl::bool_<(boost::mpl::back<value>::type::value >> ((8 * sizeof(typename boost::mpl::back<value>::type::value_type))-1))>
         , typename boost::mpl::push_front<shifted_value
         , typename inc_if_flag<typename shift_left_integral_c<typename boost::mpl::back<value>::type>::type , flag_tt>::type>::type> type_base;
      typedef typename type_base::type type;
      typedef typename type_base::flag flag;
   };
   template<typename value, typename flag_tt, typename shifted_value> 
      struct shift_left<value, flag_tt, shifted_value, 0>
   {
      typedef shifted_value type;
      typedef flag_tt flag;
   };
   template<typename value, typename flag> struct xor_if_flag
   {
       typedef value type;
   };
   template<typename value> struct xor_if_flag<value, boost::mpl::true_>
   {
        typename meta_xor<value, polynom_type>::type type;
   };
   template<typename value_tt, typename add_tt, unsigned counter = 16> struct si_crc16_count_cycle
   {
      typedef shift_left<value_tt> shifted_value;
      typedef shift_left<add_tt> shifted_add;
      typedef typename inc_seqence_if_flag<typename shifted_value::type, typename shifted_add::flag>::type modified_value;
//      typedef typename boost::mpl::if_<typename shifted_value::flag, typename meta_xor<modified_value, polynom_type>::type, modified_value::type>::type recounted_value;
      typedef typename xor_if_flag<modified_value, polynom_type>::type recounted_value;
      typedef typename si_crc16_count_cycle<typename recounted_value::type, typename shifted_add::type, counter-1>::type type;
   };
   template<typename value_tt, typename add_tt> struct si_crc16_count_cycle<value_tt, add_tt, 0>
   {
      typedef value_tt type;
   };

   template<typename data_tt, typename start_value, unsigned data_size = boost::mpl::size<data_tt>::value> struct si_crc16_consume_cycle 
   {
      typedef typename boost::mpl::deque<typename boost::mpl::front<data_tt>::type
         , typename boost::mpl::front<typename boost::mpl::pop_front<data_tt>::type>::type>::type front_value;
      typedef typename boost::mpl::pop_front<typename boost::mpl::pop_front<data_tt>::type>::type rest_sequence;
      typedef typename si_crc16_count_cycle<start_value, front_value>::type new_value;

      typedef typename si_crc16_consume_cycle<rest_sequence, new_value>::type type;
   };
   template<typename data_tt, typename start_value> struct si_crc16_consume_cycle <data_tt, start_value, 0>
   {
      typedef start_value type;
   };

   template<typename data_tt> struct si_crc16_internal 
   {
      typedef typename boost::mpl::deque<typename boost::mpl::front<data_tt>::type
         , typename boost::mpl::front<typename boost::mpl::pop_front<data_tt>::type>::type>::type first_value;
      typedef typename boost::mpl::pop_front<typename boost::mpl::pop_front<data_tt>::type>::type rest_sequence;
      typedef typename boost::mpl::if_<typename boost::mpl::empty<rest_sequence>::type
         , first_value
         , typename si_crc16_consume_cycle<rest_sequence, typename first_value::type>::type >::type type;
   };

   template<typename data_tt> struct si_crc16 
   {
      BOOST_STATIC_CONSTANT(bool, are_data_shorter_than_two = boost::mpl::size<data_tt>::value < 2);
      typedef typename boost::mpl::if_<boost::mpl::bool_<are_data_shorter_than_two>
         , boost::mpl::deque<typename byte_type<0x00>::type, typename byte_type<0x00>::type>
         , typename boost::mpl::if_c<(boost::mpl::size<data_tt>::value >> 1 << 1 == boost::mpl::size<data_tt>::value)
         , typename si_crc16_internal<typename boost::mpl::push_back<typename boost::mpl::push_back<data_tt, si::byte_type<0x00>::type>::type, si::byte_type<0x00>::type>::type >::type
         , typename si_crc16_internal<typename boost::mpl::push_back<data_tt, si::byte_type<0x00>::type >::type >::type >::type
      >::type::type type;
   };


   template<typename command_tt, typename data_tt, typename protocol_tt> struct construct_sequence
   {
      BOOST_MPL_ASSERT_MSG(false, UNKNOWN_PROTOCOL, ());
   };

   template<typename command_tt, typename data_tt> struct construct_sequence<command_tt, data_tt, protocols::extended>
   {
      typedef typename append_sequences<boost::mpl::deque<>, data_tt>::type data_t;
      typedef typename boost::mpl::push_front<
         typename boost::mpl::push_front<data_t, typename byte_type<boost::mpl::size<data_t>::value>::type>::type, command_tt>::type crc_base_t;
      typedef typename boost::mpl::push_back<
         typename boost::mpl::push_front<typename append_sequences<crc_base_t, typename si_crc16<crc_base_t>::type>::type, STX>::type
         , ETX>::type type;
   };

}//namespace si
