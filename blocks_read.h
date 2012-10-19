//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "commands_definitions.h"

#include <set>
#include <map>

namespace si
{
	typedef std::set<unsigned> needed_blocks_container;

	template<typename block_type_tt> struct blocks_read: public std::map<unsigned, block_type_tt>
	{
		typedef std::map<unsigned, block_type_tt> base_container;
		typedef typename base_container::value_type base_value_type;
		typedef boost::shared_ptr<blocks_read> pointer;
		typedef block_type_tt element_type;
		unsigned card_id;
		unsigned card_serie;
		needed_blocks_container needed_blocks;

		template<typename block_holder_tt> void block_read(block_holder_tt &block)
		{
			needed_blocks_container::iterator it = needed_blocks.find(block->template get<common::bn>().value);
			if(needed_blocks.end() == it)
			{
				return;
			}
			needed_blocks.erase(it);
			base_value_type insertion_value(block->template get<common::bn>().value, block->template get<common::read_out_data>());
			base_container::insert(insertion_value);
		}
		bool ready()
		{
			return needed_blocks.empty();
		}
	};
}//namespace si
