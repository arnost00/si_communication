//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "response_interface.h"
#include <boost/function.hpp>

namespace si
{

	template<typename command_type = si::command_interface> struct response_dynamic: public response_interface
	{
		typedef boost::function<void()> response_call_type;
		typedef boost::function<void(typename command_type::pointer)> client_call_type;
		typedef boost::shared_ptr<response_interface> pointer;

		struct reaction_type
		{
			typedef boost::shared_ptr<command_type> command_pointer;
			typedef boost::function<void(command_pointer)> reaction_callback_type;
			reaction_type(command_pointer command_, reaction_callback_type reaction_)
				: command(command_)
				, reaction(reaction_)
			{
			}
			reaction_type(reaction_callback_type reaction_)
				: command()
				, reaction(reaction_)
			{
			}
			command_pointer command;
			reaction_callback_type reaction;
		};

		inline static typename response_interface::pointer create(typename reaction_type::reaction_callback_type reaction_callback)
		{
			return response_interface::pointer(new response(reaction_type(reaction_callback)));
		}
		inline static typename response_interface::pointer create(typename command_type::pointer& expected_command, typename reaction_type::reaction_callback_type reaction_callback)
		{
			return response_interface::pointer(new response(reaction_type(expected_command, reaction_callback)));
		}

		response_dynamic(reaction_type reaction_)
			: reaction(reaction_)
		{
		}

		virtual bool check_input_command(command_interface::id_type command_id
													, std::size_t size
													, command_interface::data_type data
													, bool control_sequence)
		{
			if(reaction.command->is_control_sequence() != control_sequence)
				return false;
			if(reaction.command->get_id() != command_id)
				return false;
			if(!reaction.command->can_accept_data(size, data))
				return false;
			typename reaction_type::command_pointer actual_command(new(command_type(*reaction.command)));
			reaction.reaction(actual_command);
			return true;
		}

		reaction_type reaction;
	};

	template<typename command_type_pointer>inline response_interface::pointer create_response(typename command_type_pointer &expected_command, typename response<typename command_type_pointer::element_type>::reaction_type::reaction_callback_type reaction_callback)
	{
		typedef response<command_type_pointer::element_type> response_type;
		return response_interface::pointer(new response_type(response_type::reaction_type(expected_command, reaction_callback)));
	}

}//namespace si
