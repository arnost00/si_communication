//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_constants.h"
#include "command_parameter.h"
#include "fixed_command.h"

#include <boost/config/suffix.hpp>

#include <boost/cstdint.hpp>

namespace si
{
	struct common
	{
		struct si: public unsigned_integral_parameter<4, si>{};
		struct bn: public unsigned_integral_parameter<1, bn>{};
		struct tss: public unsigned_integral_parameter<1, tss>{};
		struct t: public unsigned_integral_parameter<2, t>{};

		struct am_pm: public unsigned_integral_parameter<1, am_pm>{};
		struct day_of_week: public unsigned_integral_parameter<1, day_of_week>{};
		struct week_counter: public unsigned_integral_parameter<1, week_counter>{};

		typedef boost::mpl::deque<don_t_care<2>
			, day_of_week::bits_range<1,0>
			, week_counter::bits_range<2,0>
			, am_pm::bits_range<0>
			> td_def;
		struct td: public bit_array<td_def, td>{};

		typedef boost::mpl::deque<> no_data;

	};
	struct basic: public common
	{	
		typedef boost::integral_constant<boost::uint8_t, 0x46> si_card_moved_code;
		typedef boost::integral_constant<boost::uint8_t, 0x66> si_card6_inserted_code;
		typedef boost::integral_constant<boost::uint8_t, 0x53> transmit_record_code;
		typedef boost::integral_constant<boost::uint8_t, 0x31> si_card5_get_code;
		typedef boost::integral_constant<boost::uint8_t, 0x61> si_card6_get_code;

		typedef boost::integral_constant<boost::uint8_t, 0x70> set_ms_mode_code;
		typedef boost::integral_constant<boost::uint8_t, 0x30> si_card5_set_card_number;
		typedef boost::integral_constant<boost::uint8_t, 0x54> time_send;

		struct m_s: public unsigned_integral_parameter<1, m_s>
		{
			BOOST_STATIC_CONSTANT(boost::uint8_t, master = 0x4D);
			BOOST_STATIC_CONSTANT(boost::uint8_t, slave = 0x53);
		};
		struct cn: public unsigned_integral_parameter<1, cn>{};
		struct cmd: public unsigned_integral_parameter<1, cmd>{};
		struct ti: public unsigned_integral_parameter<1, ti>{};
		struct tp: public unsigned_integral_parameter<1, tp>{};
		struct ql: public unsigned_integral_parameter<1, ql>{}; //queue length, extended [start|finish] only
		struct sisn: public unsigned_integral_parameter<2, sisn>{};
		struct sis: public unsigned_integral_parameter<1, sis>{};

		struct crc: public unsigned_integral_parameter<2, crc>{};
		struct cs: public unsigned_integral_parameter<1, cs>{};


		struct am_pm: public unsigned_integral_parameter<1, am_pm>{};
		struct day_of_week: public unsigned_integral_parameter<1, day_of_week>{};
		struct week_counter: public unsigned_integral_parameter<1, week_counter>{};

		typedef boost::mpl::deque<don_t_care<2>
			, day_of_week::bits_range<1,0>
			, week_counter::bits_range<2,0>
			, am_pm::bits_range<0>
			> td_def;
		struct td: public bit_array<td_def, td>{};


		struct commands
		{
			typedef boost::mpl::deque<m_s> set_ms_mode_data_def;
			typedef boost::mpl::deque<bn> si_card6_get_data_def;

			typedef fixed_command<ACK, no_data, true> ack;
			typedef fixed_command<NAK, no_data, true> nak;
			typedef fixed_command<set_ms_mode_code, set_ms_mode_data_def> set_ms_mode;
			typedef fixed_command<si_card5_get_code, no_data> si_card5_get;
			typedef fixed_command<si_card6_get_code, si_card6_get_data_def> si_card6_get;
		};
		struct responses
		{
			struct read_out_data: public byte_array<read_out_data, 128>{};

			typedef boost::mpl::deque<cmd> si_card_moved_data_def;
			typedef boost::mpl::deque<ti, tp, cn, si> si_card6_inserted_data_def;
			typedef boost::mpl::deque<cn, m_s> set_ms_mode_data_def;
			typedef boost::mpl::deque<td, cn, sis, sisn, ql, t, tss, crc> transmit_record_data_def;
			typedef boost::mpl::deque<cn, read_out_data, cs> si_card5_get_data_def;
			typedef boost::mpl::deque<cn, bn, read_out_data, cs> si_card6_get_data_def;

			typedef fixed_command<ACK, no_data, true> ack;
			typedef fixed_command<NAK, no_data, true> nak;
			typedef fixed_command<set_ms_mode_code, set_ms_mode_data_def> set_ms_mode;
			typedef fixed_command<si_card_moved_code, si_card_moved_data_def> si_card_moved;
			typedef fixed_command<si_card6_inserted_code, si_card6_inserted_data_def> si_card6_inserted;
			typedef fixed_command<transmit_record_code, transmit_record_data_def> transmit_record;
			typedef fixed_command<si_card5_get_code, si_card5_get_data_def> si_card5_get;
			typedef fixed_command<si_card6_get_code, si_card6_get_data_def> si_card6_get;
		};
	};
	struct extended: public common
	{
		typedef boost::integral_constant<boost::uint8_t, 0xF0> set_ms_mode_code;
		typedef boost::integral_constant<boost::uint8_t, 0xE5> si_card5_inserted_code;
		typedef boost::integral_constant<boost::uint8_t, 0xE6> si_card6_inserted_code;
		typedef boost::integral_constant<boost::uint8_t, 0xE8> si_card8_inserted_code;
		typedef boost::integral_constant<boost::uint8_t, 0xE7> si_card_removed_code;
		typedef boost::integral_constant<boost::uint8_t, 0xB1> si_card5_get_code;
		typedef boost::integral_constant<boost::uint8_t, 0xE1> si_card6_get_code;
		typedef boost::integral_constant<boost::uint8_t, 0xEF> si_card8_get_code;
		typedef boost::integral_constant<boost::uint8_t, 0xD3> transmit_record_code;
		struct m_s: public unsigned_integral_parameter<1, m_s>
		{
			BOOST_STATIC_CONSTANT(boost::uint8_t, master = 0x4D);
			BOOST_STATIC_CONSTANT(boost::uint8_t, slave = 0x53);
		};

		struct cn: public unsigned_integral_parameter<2, cn>{};
		struct mem: public unsigned_integral_parameter<3, mem>{};

		struct commands
		{
			typedef boost::mpl::deque<m_s> set_ms_mode_data_def;
			typedef boost::mpl::deque<bn> si_card_multiblock_get_data_def;
			typedef boost::mpl::deque<bn> si_card8_get_data_def;

			typedef fixed_command<ACK, no_data, true> ack;
			typedef fixed_command<NAK, no_data, true> nak;
			typedef fixed_command<set_ms_mode_code, set_ms_mode_data_def> set_ms_mode;
			typedef fixed_command<si_card5_get_code, no_data> si_card5_get;
			typedef fixed_command<si_card6_get_code, si_card_multiblock_get_data_def> si_card6_get;
			typedef fixed_command<si_card8_get_code, si_card_multiblock_get_data_def> si_card8_get;
		};
		struct responses
		{
			struct read_out_data: public byte_array<read_out_data, 128>{};

			typedef boost::mpl::deque<cn, m_s> set_ms_mode_data_def;
			typedef boost::mpl::deque<cn, si> card_move_data_def;
			typedef boost::mpl::deque<cn, read_out_data> si_card5_get_data_def;
			typedef boost::mpl::deque<cn, bn, read_out_data> si_card_multiblock_get_data_def;
			typedef boost::mpl::deque<cn, si, td, t, tss, mem> transmit_record_data_def;

			typedef fixed_command<ACK, no_data, true> ack;
			typedef fixed_command<NAK, no_data, true> nak;
			typedef fixed_command<set_ms_mode_code, set_ms_mode_data_def> set_ms_mode;
			typedef fixed_command<si_card5_inserted_code, card_move_data_def> si_card5_inserted;
			typedef fixed_command<si_card6_inserted_code, card_move_data_def> si_card6_inserted;
			typedef fixed_command<si_card8_inserted_code, card_move_data_def> si_card8_inserted;
			typedef fixed_command<si_card_removed_code, card_move_data_def> si_card_removed;

			typedef fixed_command<transmit_record_code, transmit_record_data_def> transmit_record;

			typedef fixed_command<si_card5_get_code, si_card5_get_data_def> si_card5_get;
			typedef fixed_command<si_card6_get_code, si_card_multiblock_get_data_def> si_card6_get;
			typedef fixed_command<si_card8_get_code, si_card_multiblock_get_data_def> si_card8_get;
		};
	};
/*		struct addr: public si::unsigned_integral_parameter<3, addr>{};
		struct num: public si::unsigned_integral_parameter<1, num>{};

		struct cn: public si::unsigned_integral_parameter<2, cn>{};

		struct read_si_after_punch: public si::parameter_t<read_si_after_punch>{};
		struct access_with_password_only: public si::parameter_t<access_with_password_only>{};
		struct handshake: public si::parameter_t<handshake>{};
		struct auto_send_out: public si::parameter_t<auto_send_out>{};
		struct extended_protocol: public si::parameter_t<extended_protocol>{};

		struct cpc: public si::bit_array<boost::mpl::deque<read_si_after_punch, si::don_t_care, si::don_t_care, access_with_password_only, si::don_t_care, handshake, auto_send_out, extended_protocol>, cpc>{};

		typedef boost::mpl::deque<addr, num> read_backup_data_def;
		typedef boost::mpl::deque<cn, boost::mpl::integral_c<si::boost::uint8_t, 0x74>, cpc> protocol_configuration_def;


		typedef si::fixed_command<0x81, read_backup_data_def> read_backup_data_command; 
		typedef si::fixed_command<0x83, protocol_configuration_def> protocol_configuration_response; 
		typedef si::fixed_command<0x83, boost::mpl::deque<boost::mpl::integral_c<si::boost::uint8_t, 0x72>, boost::mpl::integral_c<si::boost::uint8_t, 0x01> > > whatever_command_def;

	}//namespace commands*/
}//namespace si
