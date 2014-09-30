//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/deque.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/end.hpp>
#include <boost/type_traits/is_same.hpp>

namespace si
{

	typedef boost::mpl::integral_c<boost::uint8_t, 0x02> STX;
	typedef boost::mpl::integral_c<boost::uint8_t, 0x03> ETX;
	typedef boost::mpl::integral_c<boost::uint8_t, 0x06> ACK;
	typedef boost::mpl::integral_c<boost::uint8_t, 0x15> NAK;
	typedef boost::mpl::integral_c<boost::uint8_t, 0x10> DLE;

	template <boost::uint8_t value> struct byte_type:public boost::mpl::integral_c<boost::uint8_t, value>{};

	struct control_sequence{};

	BOOST_STATIC_CONSTANT(std::size_t, unknown_size = std::size_t(-1));

	struct protocols
	{
		struct none{};
		struct basic{};
		struct extended{};

		typedef boost::mpl::deque<basic, extended> supported_set;
		template<typename protocol_tt = void, typename protocols_set = supported_set> struct id
				: public boost::mpl::if_<
				typename boost::is_same<
				typename boost::mpl::find<protocols_set, protocol_tt>::type
				, typename boost::mpl::end<protocols_set>::type>::type
				, typename boost::mpl::integral_c<typename boost::mpl::end<protocols_set>::type::pos::value_type, -1>
				, typename boost::mpl::find<protocols_set, protocol_tt>::type::pos>::type{};

		template<typename protocols::id<>::value_type id, typename protocols = supported_set> struct find_protocol
		{
			typedef typename protocols::template id<>::value_type id_type;
			typedef typename boost::mpl::if_<typename boost::mpl::empty<protocols>::type
			, void
			, typename boost::mpl::if_c< (id == 0)
			, typename boost::mpl::front<protocols>::type
			, typename find_protocol<id - 1, typename boost::mpl::pop_front<protocols>::type >::type >::type >::type type;
			//         BOOST_MPL_ASSERT_MSG((typename boost::is_same<type, void>::value), SUPPORTED_SET_DOESN_T_CONTAIN_PROTOCOL_ID, (types<typename boost::mpl::integral_c<id_type, id>::type, protocols>) );
		};
		template<typename protocol, typename protocols_tt = supported_set> struct less_or_equeal_protocol
		{
			template<typename checked_set = protocols_tt
						, typename supported_subset_part = boost::mpl::deque<>
						, typename matched = typename boost::is_same<typename boost::mpl::front<checked_set>::type, protocol>::type > struct create_supported_subset
			{
				BOOST_MPL_ASSERT_MSG(!boost::mpl::empty<checked_set>::value, SUPPORTED_SET_DOESN_T_CONTAIN_PROTOCOL, (types<protocol, checked_set, supported_subset_part>));

				typedef typename boost::mpl::push_back<supported_subset_part, protocol>::type type;
			};
			template<typename checked_set, typename supported_subset_part>struct create_supported_subset<checked_set, supported_subset_part, boost::false_type>
			{
				BOOST_MPL_ASSERT_MSG(!boost::mpl::empty<checked_set>::value, SUPPORTED_SET_DOESN_T_CONTAIN_PROTOCOL, (types<protocol, checked_set, supported_subset_part>));

				typedef typename create_supported_subset<typename boost::mpl::pop_front<checked_set>::type, typename boost::mpl::push_front<supported_subset_part, typename boost::mpl::front<checked_set>::type>::type>::type type;
			};
			typedef typename create_supported_subset<>::type supported_subset;

			template<typename protocols::id<>::value_type id, typename subset = supported_subset> struct is_protocol_id_in_sequence
					: public boost::mpl::not_<typename boost::is_same<typename boost::mpl::find<subset, typename find_protocol<id, protocols_tt>::type>::type, typename boost::mpl::end<protocols_tt>::type>::type>::type{};

			template<typename tested_set = supported_subset, typename empty = typename boost::mpl::empty<tested_set>::type>struct check_protocol
			{
				static inline bool check(typename protocols::id<>::value_type protocol_id)
				{
					if(id<typename boost::mpl::front<tested_set>::type, protocols_tt>::value == protocol_id){return true;}
					return check_protocol<typename boost::mpl::pop_front<tested_set>::type>::check(protocol_id);
				}
			};
			template<typename tested_set>struct check_protocol<tested_set, boost::mpl::true_>
			{
                static inline bool check(typename protocols::id<>::value_type )
				{
					return false;
				}
			};

			inline static bool check(typename protocols::id<>::value_type protocol_id)
			{
				return check_protocol<>::check(protocol_id);
			}
		};
	};
}//namespace si
