//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_io_serial_port.h"
#include "control_sequence_base.h"
#include "make_pointer.h"
#include "card_readouts.h"
#include "blocks_read.h"

namespace si
{
	class chip_readout: public control_sequence_base<channel_io_serial_port>
	{
	public:
		typedef boost::function<void()> callback_type;
		typedef boost::function<void(card_record::pointer)> callback_chip_read;

		void start(channel_io_serial_port::pointer port
			, callback_type success_cb = callback_type()
			, callback_type failure_cb = callback_type()
			, callback_chip_read chip_read_cb_ = callback_chip_read())
		{
			chip_read_cb = chip_read_cb_;

			control_sequence_base<channel_io_serial_port>::start(port, success_cb, failure_cb);

			si::response<
					boost::mpl::deque<extended::responses::si_card5_inserted
						, extended::responses::si_card6_inserted
						, extended::responses::si_card8_inserted
						, extended::responses::si_card_removed>
					, response_live_control::permanent>::reactions_type
				reaction(boost::bind(&chip_readout::si_card5_inserted, this, _1)
				, boost::bind(&chip_readout::si_card6_inserted, this, _1)
				, boost::bind(&chip_readout::si_card8_inserted, this, _1)
				, boost::bind(&chip_readout::si_card_removed, this, _1));
			read_responses = si::response<>::create(reaction);

			channel->register_response_expectation(read_responses);

		}
		~chip_readout()
		{
			stop();
		}

		void stop()
		{
			if(read_responses)
				channel->unregister_response_expectation(read_responses);
		}

		void si_card5_inserted(extended::responses::si_card5_inserted::pointer response)
		{
			LOG << "card 5 inserted, no: " << response->get<extended::si>().value << std::endl;

			si::response<boost::mpl::deque<extended::responses::si_card5_get
					, extended::responses::si_card_removed
					, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&chip_readout::si_card5_read, this, _1)
				, boost::bind(&chip_readout::si_card_removed_during_readout, this, _1)
				, boost::bind(&chip_readout::nak, this, _1));
			channel->register_response_expectation(
				si::response<>::create(reaction)
				, boost::posix_time::seconds(2)
				, boost::bind(&chip_readout::si_card5_read_timeout, this));
			make_pointer<extended::commands::si_card5_get> read_card5;
			channel->write_command(read_card5);
		}
		void si_card5_read(extended::responses::si_card5_get::pointer response)
		{
			LOG << "card 5 read" << std::endl;

			make_pointer<card_record> readout;
			make_pointer<extended::commands::ack> ack;
			channel->write_command(ack);

			card_reader<card_5>::read(*readout.get(), *response.get());
			if(chip_read_cb)
				chip_read_cb(readout);

			stdout_card_record(*readout.get());
		}
		void si_card5_read_timeout()
		{
			LOG << "card 5 read timeout" << std::endl;			
		}
		void si_card8_inserted(extended::responses::si_card8_inserted::pointer response)
		{
			LOG << card_reader<card_8_family>::get_type_description(response->get<extended::si>()) << " inserted, no: " 
				<< card_reader<card_8_family>::get_id(response->get<extended::si>().value) << std::endl;
			si::response<boost::mpl::deque<extended::responses::si_card8_get
					, extended::responses::si_card_removed
					, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&chip_readout::si_card8_read, this, _1, make_pointer<blocks_read<extended::responses::si_card8_get> >())
				, boost::bind(&chip_readout::si_card_removed_during_readout, this, _1)
				, boost::bind(&chip_readout::nak, this, _1));

			channel->register_response_expectation(si::response<>::create(reaction)
				, boost::posix_time::seconds(2)
				, boost::bind(&chip_readout::si_card8_read_timeout, this));
			make_pointer<extended::commands::si_card8_get> read_card8;
			read_card8->get<extended::bn>().value = 0;
			channel->write_command(read_card8);
		}
		void si_card8_read(extended::responses::si_card8_get::pointer response
			, blocks_read<extended::responses::si_card8_get>::pointer blocks)
		{
			LOG << "card 8: block: " << unsigned(response->get<extended::bn>().value) << " read" << std::endl;
			if(0 == response->get<extended::bn>().value)
			{
				blocks->card_id = card_reader<card_8_family>::get_id(*response.get());
				blocks->card_serie = card_reader<card_8_family>::get_serie(*response.get());
				card_reader<card_8_family>::get_blocks_needed(blocks->card_serie, blocks->needed_blocks, *response.get());
			}
			blocks->block_read(response);
			if(blocks->ready())
			{
				make_pointer<extended::commands::ack> ack;
				channel->write_command(ack);

				make_pointer<card_record> readout;

				card_reader<card_8_family>::read(*readout.get(), *blocks.get());
				if(chip_read_cb)
					chip_read_cb(readout);

				stdout_card_record(*readout.get());
				return;
			}
			si::response<boost::mpl::deque<extended::responses::si_card8_get
					, extended::responses::si_card_removed
					, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&chip_readout::si_card8_read, this, _1, blocks)
				, boost::bind(&chip_readout::si_card_removed_during_readout, this, _1)
				, boost::bind(&chip_readout::nak, this, _1));

			channel->register_response_expectation(si::response<>::create(reaction)
				, boost::posix_time::seconds(2)
				, boost::bind(&chip_readout::si_card8_read_timeout, this));
			make_pointer<extended::commands::si_card8_get> read_card8;
			read_card8->get<extended::bn>().value = *(blocks->needed_blocks.begin());
			channel->write_command(read_card8);

		}
		void si_card8_read_timeout()
		{
			LOG << "card 8 read timeout" << std::endl;			
		}
		void si_card6_inserted(extended::responses::si_card6_inserted::pointer response)
		{
			LOG << "card 6 inserted, no: " << response->get<extended::si>().value << std::endl;
			si::response<boost::mpl::deque<extended::responses::si_card6_get
					, extended::responses::si_card_removed
					, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&chip_readout::si_card6_read, this, _1, make_pointer<blocks_read<extended::responses::si_card6_get> >())
				, boost::bind(&chip_readout::si_card_removed_during_readout, this, _1)
				, boost::bind(&chip_readout::nak, this, _1));

			channel->register_response_expectation(si::response<>::create(reaction)
				, boost::posix_time::seconds(2)
				, boost::bind(&chip_readout::si_card6_read_timeout, this));
			make_pointer<extended::commands::si_card6_get> read_card6;
			read_card6->get<extended::bn>().value = 0;
			channel->write_command(read_card6);
		}
		void si_card6_read(extended::responses::si_card6_get::pointer response
			, blocks_read<extended::responses::si_card6_get>::pointer blocks)
		{
			LOG << "card 6: block: " << unsigned(response->get<extended::bn>().value) << " read" << std::endl;
			if(0 == response->get<extended::bn>().value)
			{
				blocks->card_id = card_reader<card_6>::get_id(*response.get());
				card_reader<card_6>::get_blocks_needed(blocks->needed_blocks, *response.get());
			}
			blocks->block_read(response);
			if(blocks->ready())
			{
				make_pointer<extended::commands::ack> ack;
				channel->write_command(ack);

				make_pointer<card_record> readout;

				card_reader<card_6>::read(*readout.get(), *blocks.get());
				if(chip_read_cb)
					chip_read_cb(readout);

				stdout_card_record(*readout.get());
				return;
			}
			si::response<boost::mpl::deque<extended::responses::si_card6_get
					, extended::responses::si_card_removed
					, extended::responses::nak> >::reactions_type
				reaction(boost::bind(&chip_readout::si_card6_read, this, _1, blocks)
				, boost::bind(&chip_readout::si_card_removed_during_readout, this, _1)
				, boost::bind(&chip_readout::nak, this, _1));

			channel->register_response_expectation(si::response<>::create(reaction)
				, boost::posix_time::seconds(2)
				, boost::bind(&chip_readout::si_card6_read_timeout, this));
			make_pointer<extended::commands::si_card6_get> read_card6;
			read_card6->get<extended::bn>().value = *(blocks->needed_blocks.begin());
			channel->write_command(read_card6);
		}
		void si_card6_read_timeout()
		{
			LOG << "card 6 read timeout" << std::endl;			
		}
		void si_card_removed(extended::responses::si_card_removed::pointer response)
		{
			LOG << "card removed, no: " << response->get<extended::si>().value << std::endl;
		}
		void si_card_removed_during_readout(extended::responses::si_card_removed::pointer response)
		{
			LOG << "card removed during readout, no: " << response->get<extended::si>().value << std::endl;
		}
		void nak(extended::responses::nak::pointer response)
		{
			LOG << "nak arrived" << std::endl;
		}

		response_interface::pointer read_responses;
		callback_chip_read chip_read_cb;
	};
}//namespace si
