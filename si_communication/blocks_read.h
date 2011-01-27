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

	template<typename block_message_tt> struct blocks_read: public std::map<unsigned, typename block_message_tt::pointer>
	{
		typedef std::map<unsigned, typename block_message_tt::pointer> base_container;
		typedef typename base_container::value_type base_value_type;
		typedef boost::shared_ptr<blocks_read> pointer;
		typedef block_message_tt element_type;
		unsigned card_id;
		unsigned card_serie;
		needed_blocks_container needed_blocks;

		void block_read(typename block_message_tt::pointer block)
		{
                        needed_blocks_container::iterator it = needed_blocks.find(block->template get<extended::bn>().value);
			if(needed_blocks.end() == it)
			{
				return;
			}
			needed_blocks.erase(it);
			base_value_type insertion_value(block->template get<extended::bn>().value, block);
			insert(insertion_value);
		}
		bool ready()
		{
			return needed_blocks.empty();
		}
	};
}//namespace si
