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

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace si
{

	class chip_readout: public control_sequence_base<channel_io_serial_port>
			, public boost::enable_shared_from_this<chip_readout>
	{
	public:
		typedef boost::shared_ptr<chip_readout> pointer;
		typedef boost::function<void()> callback_type;
		typedef boost::function<void(card_record::pointer)> callback_chip_read;

		typedef channel_io_serial_port::pointer io_arg;

		template<typename protocol_tt, typename card_tt> struct comm_switch
		{};

		template<typename protocol_tt, typename card_tt>
		void si_card_inserted(typename comm_switch<protocol_tt, card_tt>::card_inserted_response::pointer response);
		void start(channel_io_serial_port::pointer port = channel_io_serial_port::pointer()
				, callback_type success_cb = callback_type()
				, callback_type failure_cb = callback_type()
				, callback_chip_read chip_read_cb_ = callback_chip_read());
		~chip_readout()
		{
			stop();
		}

		void stop()
		{
			if(read_responses)
				channel->unregister_response_expectation(read_responses);
		}

		void log_card_5_confuscated_number(boost::uint32_t value)
		{
			LOG << ((boost::uint16_t) value + 100000 * (value >> 16)) << " ";
		}

		void si_card5_inserted(extended::responses::si_card5_inserted::pointer response)
		{
			LOG << "card 5 inserted, no: ";
			log_card_5_confuscated_number((response->get<extended::si>().value));
			LOG << std::endl;

			//LOG << "card 5 inserted, no: " << ((response->get<extended::si>().value)  << std::endl;

			si::response<boost::mpl::deque<extended::responses::si_card5_get
					, extended::responses::si_card_removed
					, common::nak> >::reactions_type
					reaction(boost::bind(&chip_readout::si_card5_read, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card5_removed_during_readout, shared_from_this(), _1)
								, boost::bind(&chip_readout::nak, shared_from_this(), _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::seconds(4)
						, boost::bind(&chip_readout::si_card5_read_timeout, shared_from_this()));
			make_pointer<extended::commands::si_card5_get> read_card5;
			channel->write_command(read_card5);
		}
		void si_card5_read(extended::responses::si_card5_get::pointer response)
		{
			LOG << "card 5 read" << std::endl;

			make_pointer<card_record> readout;
			make_pointer<common::ack> ack;
			channel->write_command(ack);

			si::response<extended::responses::si_card_removed>::reactions_type
					reaction( boost::bind(&chip_readout::si_card5_removed, shared_from_this(), _1));

			channel->register_response_expectation(si::response<>::create(reaction));

			card_reader<card_5>::read(*readout.get(), *response.get());
			if(chip_read_cb)
				chip_read_cb(readout);

			stdout_card_record(*readout.get());
		}
		void si_card5_read_basic(basic::responses::si_card5_get::pointer response)
		{
			LOG << "card 5 read" << std::endl;

			make_pointer<card_record> readout;
			make_pointer<common::ack> ack;
			channel->write_command(ack);

			stdout_card_record(*readout.get());

			card_reader<card_5>::read(*readout.get(), *response.get());
			if(chip_read_cb)
				chip_read_cb(readout);

		}
		void si_card5_read_timeout()
		{
			LOG << "card 5 read timeout" << std::endl;
		}
		template<typename protocol_tt, typename card_tt> void si_card_read(typename comm_switch<protocol_tt, card_tt>::block_read_response::pointer response
								 , blocks_read<common::read_out_data>::pointer blocks)
		{
			typedef card_reader<card_tt> card_reader_t;

			LOG << card_reader_t::get_description() << ": block: " << unsigned(response->template get<typename protocol_tt::bn>().value) << " read" << std::endl;
			if(0 == response->template get<extended::bn>().value)
			{
				blocks->card_id = card_reader_t::get_id(response->template get<common::read_out_data>());
				blocks->card_serie = card_reader_t::get_serie(response->template get<common::read_out_data>());
				card_reader_t::get_blocks_needed(blocks->card_serie, blocks->needed_blocks, response->template get<common::read_out_data>());
			}
			blocks->block_read(response);
			if(blocks->ready())
			{
				make_pointer<common::ack> ack;
				channel->write_command(ack);

				make_pointer<card_record> readout;

				card_reader<card_tt>::read(*readout.get(), *blocks.get());
				if(chip_read_cb)
					chip_read_cb(readout);

				stdout_card_record(*readout.get());
				return;
			}
			si_any_card_command_read<protocol_tt, card_tt>(blocks);
		}

		template<typename protocol_tt, typename card_tt> void si_any_card_command_read(blocks_read<common::read_out_data>::pointer blocks)
		{
			typedef boost::function<void()> void_fnc_void;
			typedef boost::shared_ptr<void_fnc_void> void_fnc_void_ptr;
			typedef typename si::response<boost::mpl::deque<typename comm_switch<protocol_tt, card_tt>::block_read_response
						, typename comm_switch<protocol_tt, card_tt>::card_removed_response
						, common::nak> >::reactions_type reactions_type;

			void_fnc_void_ptr retry_fnc_call(new void_fnc_void_ptr::element_type);
			boost::shared_ptr<unsigned> retry_count(new unsigned);
			*retry_count = 1;

			reactions_type reaction(
								boost::bind(&chip_readout::si_card_read<protocol_tt, card_tt>
									, shared_from_this(), _1, blocks)
								, boost::bind(&chip_readout::si_card_removed_during_readout<protocol_tt, card_tt>, shared_from_this(), _1)
								, boost::bind(&chip_readout::nak_retry, shared_from_this(), _1, retry_fnc_call, retry_count));

			*retry_fnc_call = (boost::bind(&chip_readout::si_card_command_read<typename comm_switch<protocol_tt, card_tt>::block_read_response
					, reactions_type>
				, shared_from_this(), reaction, blocks));

			si_card_command_read<typename comm_switch<protocol_tt, card_tt>::block_read_command, reactions_type>(reaction, blocks);

		}

		template<typename command_tt, typename reaction_tt> void si_card_command_read(reaction_tt const& reaction
					, blocks_read<common::read_out_data>::pointer blocks)
		{

			channel->register_response_expectation(si::response<>::create(reaction)
																, boost::posix_time::seconds(4)
																, boost::bind(&chip_readout::si_card_read_timeout, shared_from_this()));
			make_pointer<command_tt> read_card;

			read_card->template get<typename common::bn>().value = *(blocks->needed_blocks.begin());

			channel->write_command(read_card);

		}
		void si_card_read_timeout()
		{
			LOG << "card read timeout" << std::endl;
		}
		void si_card_removed(extended::responses::si_card_removed::pointer response)
		{
			LOG << "card removed, no: " << response->get<extended::si>().value << std::endl;
		}
		template <typename response_tt> void si_card_moved(response_tt response)
		{
			switch(response->template get<basic::cmd>().value)
			{
			case 'I':
				LOG << "card 5 series moved in" << std::endl;
				si_card5_inserted();
				return;
			case 'O':
				LOG << "card removed" << std::endl;
				return;
			default:
				LOG << "card moved in unkonwn direction" << std::endl;
			};
		}
		template <typename response_tt> void si_card_moved_during_readout_basic(response_tt response)
		{
			LOG << "card moved during readout: ";
			switch(response->template get<basic::cmd>().value)
			{
			case 'I':
				LOG << "in (seems like if two are inserted at the same time, wierd)" << std::endl;
				return;
			case 'O':
				LOG << "out" << std::endl;
				return;
			default:
				LOG << "card moved in unkonwn direction" << std::endl;
			};
		}

		void si_card5_inserted()
		{
			si::response<boost::mpl::deque<basic::responses::si_card5_get
					, basic::responses::si_card_moved
					, common::nak> >::reactions_type
					reaction(boost::bind(&chip_readout::si_card5_read_basic, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_moved_during_readout_basic<basic::responses::si_card_moved::pointer>, shared_from_this(), _1)
								, boost::bind(&chip_readout::nak, shared_from_this(), _1));
			channel->register_response_expectation(
						si::response<>::create(reaction)
						, boost::posix_time::seconds(4)
						, boost::bind(&chip_readout::si_card5_read_timeout, shared_from_this()));
			make_pointer<basic::commands::si_card5_get> read_card5;
			channel->write_command(read_card5);

		}
		template<typename protocol_tt, typename card_tt>
		void si_card_removed_during_readout(typename comm_switch<protocol_tt, card_tt>::card_removed_response::pointer response)
		{
			LOG << "card removed during readout, no: " << response->template get<extended::si>().value << std::endl;
		}

		void si_card5_removed(extended::responses::si_card_removed::pointer response)
		{
			LOG << "card removed, no: ";
			log_card_5_confuscated_number(response->get<extended::si>().value);
			LOG << std::endl;
		}
		void si_card5_removed_during_readout(extended::responses::si_card_removed::pointer response)
		{
			LOG << "card removed during readout, no: ";
			log_card_5_confuscated_number(response->get<extended::si>().value);
			LOG << std::endl;
		}

		void nak(common::nak::pointer )
		{
			LOG << "nak arrived" << std::endl;
		}
		void nak_retry(common::nak::pointer, boost::shared_ptr<boost::function<void()> > pfnc, boost::shared_ptr<unsigned> retry_count)
		{
			LOG << "nak arrived, retry count: " << retry_count << std::endl;
			if(--(*retry_count))
			{
				(*pfnc)();
			}
		}

		response_interface::pointer read_responses;
		callback_chip_read chip_read_cb;

		pointer ptr;

		static pointer create_shared()
		{
			pointer item(new chip_readout);
			return (item->ptr = item);
		}
	};

	template<> struct chip_readout::comm_switch<extended, card_8_family>
	{
		typedef extended::responses::si_card8_inserted card_inserted_response;
		typedef extended::commands::si_card8_get block_read_command;
		typedef extended::responses::si_card8_get block_read_response;
		typedef extended::responses::si_card_removed card_removed_response;
	};
	template<> struct chip_readout::comm_switch<extended, card_6>
	{
		typedef extended::responses::si_card6_inserted card_inserted_response;
		typedef extended::commands::si_card6_get block_read_command;
		typedef extended::responses::si_card6_get block_read_response;
		typedef extended::responses::si_card_removed card_removed_response;
	};
	template<> struct chip_readout::comm_switch<basic, card_6>
	{
		typedef basic::responses::si_card6_inserted card_inserted_response;
		typedef basic::commands::si_card6_get block_read_command;
		typedef basic::responses::si_card6_get block_read_response;
		typedef basic::responses::si_card_moved card_removed_response;
	};
/*
		template<> void chip_readout::si_card_removed_during_readout<basic, card_5>(typename comm_switch<basic, card_5>::card_removed_response::pointer response)
		{
			chip_readout::si_card_moved(response);
		}
		*/
		template<> void chip_readout::si_card_removed_during_readout<basic, card_6>(typename comm_switch<basic, card_6>::card_removed_response::pointer response)
		{
			chip_readout::si_card_moved(response);
		}

		template<typename protocol_tt, typename card_tt>
		void chip_readout::si_card_inserted(typename comm_switch<protocol_tt, card_tt>::card_inserted_response::pointer response)
		{
			typedef typename comm_switch<protocol_tt, card_tt>::card_inserted_response response_t;
//			BOOST_MPL_ASSERT_MSG((boost::is_same<typename extended::responses::si_card8_inserted, response_t>::value), card_inserted_mismatch, (types<protocol_tt, card_tt, response_t>));

			typedef card_reader<card_tt> card_reader_t;
			LOG << card_reader_t::get_type_description(response->template get<typename extended::si>()) << " inserted, no: "
				 << card_reader_t::get_id(response->template get<typename extended::si>().value) << std::endl;

			make_pointer<blocks_read<common::read_out_data> > blocks;
			blocks->needed_blocks.insert(0);
			si_any_card_command_read<protocol_tt, card_tt>(blocks);
		}

		void chip_readout::start(channel_io_serial_port::pointer port
				, callback_type success_cb
				, callback_type failure_cb
				, callback_chip_read chip_read_cb_)
		{
			chip_read_cb = chip_read_cb_;

			control_sequence_base<channel_io_serial_port>::start(port, success_cb, failure_cb);

			si::response<
					boost::mpl::deque<extended::responses::si_card5_inserted
					, extended::responses::si_card6_inserted
					, extended::responses::si_card8_inserted
					, extended::responses::si_card_removed
					, basic::responses::si_card_moved
					, basic::responses::si_card6_inserted>
					, response_live_control::permanent>::reactions_type
					reaction(boost::bind(&chip_readout::si_card5_inserted, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_inserted<extended, card_6>, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_inserted<extended, card_8_family>, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_removed, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_moved<basic::responses::si_card_moved::pointer>, shared_from_this(), _1)
//								, boost::bind(&chip_readout::si_card6_inserted_basic, shared_from_this(), _1)
								, boost::bind(&chip_readout::si_card_inserted<basic, card_6>, shared_from_this(), _1)
								);
			read_responses = si::response<>::create(reaction);

			channel->register_response_expectation(read_responses);

		}


}//namespace si
