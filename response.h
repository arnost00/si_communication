//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "response_interface.h"
#include "tuple_type.h"
#include <boost/function.hpp>
#include <boost/mpl/deque.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/not.hpp>
#include <boost/utility/enable_if.hpp>

namespace si
{
	struct response_live_control
	{
		struct one_shot
		{
			one_shot():remove(false){}
			void called()
			{
				remove = true;
			}
			bool check(){return remove;}
		protected:
			bool remove;
		};
		struct permanent
		{
			static void called(){}
			static bool check(){return false;}
		};
	};

	template<typename command_type_tt = boost::mpl::deque<>
				, typename removal_policy_tt = response_live_control::one_shot
				, typename enabled = void> struct response: public response_interface
	{
		typedef command_type_tt command_type;
		typedef boost::function<void()> response_call_type;
		typedef boost::shared_ptr<response_interface> pointer;

		template<typename T> struct type_info
		{
			typedef T type;
		};
		template<typename target = boost::mpl::deque<>, typename source = command_type_tt, typename enabled_rtt = void> struct responses_tuple_template
		{
			typedef boost::shared_ptr<typename boost::mpl::front<source>::type> command_pointer;
			typedef boost::function<void(command_pointer&)> reaction_type;
			typedef typename responses_tuple_template<typename boost::mpl::push_back<target, reaction_type>::type
			, typename boost::mpl::pop_front<source>::type
			>::type type;
		};
		template<typename target, typename source> struct responses_tuple_template<target, source, typename boost::enable_if<typename boost::mpl::empty<source>::type>::type>
		{
			//         typedef target type;
			typedef typename boost::mpl::push_back<target, type_info<response> >::type type;
		};

		typedef typename boostext::tuple_type<typename responses_tuple_template<>::type>::type reactions_type;
		response(reactions_type const& reactions_)
			: reactions(reactions_)
		{
		}
		template<typename commands_sequence = command_type, unsigned offset = 0, bool empty = boost::mpl::empty<commands_sequence>::value> struct sequence_checker
		{
			typedef typename boost::mpl::front<commands_sequence>::type command_type;
			static bool check_input_command(command_interface::id_type command_id
													  , std::size_t size
													  , command_interface::data_type data
													  , bool control_sequence
													  , reactions_type &reactions
													  , removal_policy_tt &removal_policy)
			{
				if(command_type::control_sequence == control_sequence)
					if(command_type::code == command_id)
						//                  if(command_type::can_accept_data_static(size, data))
					{
						typename command_type::pointer actual_command(new command_type());
						if(actual_command->accept_data(size, data))
						{
							reactions.template get<offset>()(actual_command);
							removal_policy.called();
							return true;
						}
					}
				return sequence_checker< typename boost::mpl::pop_front<commands_sequence>::type, offset + 1>
						::check_input_command(command_id, size, data, control_sequence, reactions, removal_policy);
			}
		};
		template<typename commands_sequence
					, unsigned offset> struct sequence_checker<commands_sequence, offset, true >
		{
			typedef typename boost::mpl::front<commands_sequence>::type command_type;
			inline static bool check_input_command(command_interface::id_type
																, std::size_t
																, command_interface::data_type
																, bool
																, reactions_type &
																, removal_policy_tt &)
			{
				return false;
			}
		};
		virtual ~response(){}
		virtual bool check_input_command(command_interface::id_type command_id
													, std::size_t size
													, command_interface::data_type data
													, bool control_sequence)
		{
			return sequence_checker<>::check_input_command(command_id, size, data, control_sequence, reactions, removal_policy);
		}
		virtual bool remove()
		{
			return removal_policy.check();
		}
		template<typename param_type> inline static typename response_interface::pointer create(param_type const& param)
		{\

			typedef typename boost::tuples::element<boost::tuples::length<param_type>::value - 1, param_type>::type::type response_type;
			//		BOOST_MPL_ASSERT_MSG(false, PARAM_TYPE_MSG, (types<boost::tuple<int, int> , response_type>));
			return response_interface::pointer(new response_type(param));
		}
	protected:
		removal_policy_tt removal_policy;
		reactions_type reactions;


	};
	template<typename command_type_tt, typename removal_policy_tt> struct response
			<command_type_tt, removal_policy_tt
			, typename boost::enable_if<typename boost::mpl::not_<typename boost::mpl::is_sequence<command_type_tt>::type>::type>::type>
			: public response<boost::mpl::deque<command_type_tt>, removal_policy_tt>
	{
		typedef response<boost::mpl::deque<command_type_tt>, removal_policy_tt> base_response;

		response(typename base_response::reactions_type &reactions_)
			: base_response(reactions_)
		{}

		template<typename param_type> inline static typename response_interface::pointer create(param_type const& param)
		{
			return response_interface::pointer(new response(base_response::reactions_type(param)));
		}
	};

}//namespace si
