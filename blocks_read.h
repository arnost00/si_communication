#pragma once

#include "commands_definitions.h"

#include <set>
#include <map>

namespace si
{
	typedef std::set<unsigned> needed_blocks_container;

	template<typename block_message_tt> struct blocks_read: public std::map<unsigned, typename block_message_tt::pointer>
	{
		typedef boost::shared_ptr<blocks_read> pointer;
		typedef block_message_tt element_type;
		unsigned card_id;
		unsigned card_serie;
		needed_blocks_container needed_blocks;

		void block_read(typename block_message_tt::pointer block)
		{
			needed_blocks_container::iterator it = needed_blocks.find(block->get<extended::bn>().value);
			if(needed_blocks.end() == it)
			{
				return;
			}
			needed_blocks.erase(it);
			insert(value_type(block->get<extended::bn>().value, block));
		}
		bool ready()
		{
			return needed_blocks.empty();
		}
	};
}//namespace si
