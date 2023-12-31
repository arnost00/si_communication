//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "commands_definitions.h"
#include "card_record.h"
#include "blocks_read.h"

#include <boost/cstdint.hpp>
#include <boost/mpl/string.hpp>
#include <set>

namespace si
{

	struct card_type{};
	struct card_5: public card_type{};
	struct card_6: public card_type{};
	struct card_6star: public card_6{};
	struct card_8: public card_type{};
	struct card_9: public card_type{};
	struct card_15: public card_type{};
	struct card_p: public card_type{};
	struct card_t: public card_type{};
	struct card_8_family: public card_8, public card_9, public card_15, public card_p, public card_t{};


	template<typename card_type = void> struct card_reader
	{
		struct control_number: public unsigned_integral_parameter<2, control_number>{};
		struct time_12h: public unsigned_integral_parameter<2, time_12h>{};
		struct am_pm: public unsigned_integral_parameter<1, am_pm>{};
		struct day_of_week: public unsigned_integral_parameter<1, day_of_week>{};
		struct week_counter: public unsigned_integral_parameter<1, week_counter>{};
		struct card5_id_part: public unsigned_integral_parameter<1, card5_id_part>{};
		struct card5_serie_part: public unsigned_integral_parameter<2, card5_serie_part>{};
		struct sub_second: public unsigned_integral_parameter<1, sub_second>{};
		struct si_station_id: public unsigned_integral_parameter<3, si_station_id>{};
		struct punch_year: public unsigned_integral_parameter<1, punch_year>{};
		struct punch_month: public unsigned_integral_parameter<1, punch_month>{};
		struct punch_day: public unsigned_integral_parameter<1, punch_day>{};
		struct punch_sub_second: public unsigned_integral_parameter<1, punch_sub_second>{};
		struct punch_sub_second_flag: public unsigned_integral_parameter<1, punch_sub_second>{};

		typedef boost::mpl::deque<typename control_number::template bits_range<7,0> > punch_1byte_def;
		typedef boost::mpl::deque<typename control_number::template bits_range<7,0>, typename time_12h::template bits_range<15,0> > punch_3bytes_def;
		typedef boost::mpl::deque<typename control_number::template bits_range<9,8>
		, typename day_of_week::template bits_range<1,0>
		, typename week_counter::template bits_range<2,0>
		, typename am_pm::template bits_range<0>
		, typename control_number::template bits_range<7,0>
		, typename time_12h::template bits_range<15,0> > punch_4bytes_def;

		typedef boost::mpl::deque<typename punch_sub_second_flag::template bits_range<0>
		, typename control_number::template bits_range<8>
		, typename day_of_week::template bits_range<1,0>
		, typename week_counter::template bits_range<2,0>
		, typename am_pm::template bits_range<0>
		, typename control_number::template bits_range<7,0>
		, typename time_12h::template bits_range<15,0> > punch_4bytes_subsec_def;


		typedef boost::mpl::deque<typename control_number::template bits_range<7,0>
		, typename si_station_id::template bits_range<17,0>
		, typename punch_year::template bits_range<3,0>
		, typename punch_month::template bits_range<3,0>
		, typename punch_day::template bits_range<4,0>
		, typename am_pm::template bits_range<0>
		, typename time_12h::template bits_range<15,0>
		, typename punch_sub_second::template bits_range<7,0> > punch_8bytes_def;
		typedef boost::mpl::deque<card5_id_part, card5_serie_part> card5_id_def;

		template<typename T> struct punch_1byte_type: public bit_array<punch_1byte_def, T>{};
		template<typename T> struct punch_3bytes_type: public bit_array<punch_3bytes_def, T>{};
		template<typename T> struct punch_4bytes_type: public bit_array<punch_4bytes_def, T>{};
		template<typename T> struct punch_8bytes_type: public bit_array<punch_8bytes_def, T>{};

		struct card5_id_type: public bit_array<card5_id_def, card5_id_type>{};

        static inline boost::posix_time::time_duration get_duration(boost::uint16_t source, boost::uint16_t subsecond = 0, bool = false)
		{
			if(0xEEEE == source)
					return boost::posix_time::time_duration(boost::posix_time::not_a_date_time);
			return boost::posix_time::milliseconds(boost::int64_t(source) * 1000 + subsecond * 1000 / 256);
		}
	};

	template<> struct card_reader<card_5>: public card_reader<>
	{

		struct card_id: public unsigned_integral_parameter<2, card_id>{};
		struct card_serie: public unsigned_integral_parameter<1, card_serie>{};
		struct start_no: public unsigned_integral_parameter<2, start_no>{};
		struct start_time: public unsigned_integral_parameter<2, start_time>{};
		struct finish_time: public unsigned_integral_parameter<2, finish_time>{};
		struct check_time: public unsigned_integral_parameter<2, check_time>{};
		struct record_counter: public unsigned_integral_parameter<1, record_counter>{};

		typedef boost::mpl::deque<
		don_t_care<4>, card_id, card_serie
		, don_t_care<0x0A>, start_no, start_time, finish_time, record_counter
		, don_t_care<1>, check_time> card_5_header_def;

		struct card_5_header_type: public parameters_array<card_5_header_def>{};
		struct punch_type: public punch_3bytes_type<punch_type>{};
		struct extra_punch_type: public punch_1byte_type<extra_punch_type>{};

		typedef boost::mpl::string<'card', ' 5'> description_t;

		inline static const char* get_description()
		{
			return boost::mpl::c_str<description_t>::value;
		}

		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string card_type("card 5");
			return card_type;
		}
		inline static boost::uint32_t get_id(extended::si::value_type data)
		{
			return ( (data & 0xFFFF) + 100000 * (data >> 16));
		}

		template <typename iterator> static bool internal_read(card_record &readout, iterator datablock)
		{

			iterator item;
			iterator data_item = datablock;

			std::size_t max_size = 128;


			card_5_header_type card_5_header;
			
			card_5_header.read_data(max_size, data_item);
			readout.get<card_record::CARD_ID>() = card_5_header.get<card_id>() + (1 == card_5_header.get<card_serie>()? 0: 100000 * card_5_header.get<card_serie>());

			readout.get<card_record::START_NO>() = card_5_header.get<start_no>();
			readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_5_header.get<start_time>());
			readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_5_header.get<finish_time>());
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(card_5_header.get<check_time>());
			readout.get<card_record::CLEAR_TIME>() = boost::posix_time::not_a_date_time;
			std::size_t records_count = card_5_header.get<record_counter>() - 1;

			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);
			iterator punch_base = datablock + 0x20;
			iterator current_record = punch_base;


			punch_type punch;
			extra_punch_type extra_punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				if(i < 30)
				{
					if(0 == (i % 5))
						current_record++;
					punch.read_data(max_size, current_record);
					readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>().value
																									, card_reader<>::get_duration(punch.get<time_12h>().value));
				}
				else
				{
					extra_punch.read_data(max_size, item = punch_base + (0x10 * (i - 30)));
					readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(extra_punch.get<control_number>(), boost::posix_time::time_duration(boost::posix_time::not_a_date_time));
				}
			}
			return true;
		}
		static bool read(card_record &readout, extended::responses::si_card5_get& data)
		{
			return internal_read(readout, data.get<common::read_out_data>().begin());
		}
		static bool read(card_record &readout, basic::responses::si_card5_get& data)
		{
			return internal_read(readout, data.get<common::read_out_data>().begin());
		}
	};
	template<> struct card_reader<card_8_family>: public card_reader<>
	{
		struct device_uid: public unsigned_integral_parameter<4, device_uid>{};
		struct type_identifier: public unsigned_integral_parameter<4, type_identifier>{};
		struct start_time: public punch_4bytes_type<start_time>{};
		struct finish_time: public punch_4bytes_type<finish_time>{};
		struct check_time: public punch_4bytes_type<check_time>{};
		struct last_visited_control: public unsigned_integral_parameter<2, last_visited_control>{};
		struct record_counter: public unsigned_integral_parameter<1, record_counter>{};
		struct card_serie: public unsigned_integral_parameter<1, card_serie>{};
		struct card_id: public unsigned_integral_parameter<3, card_id>{};

		typedef boost::mpl::deque<don_t_care<4>, card_serie::bits_range<3,0>, card_id::bits_range<23,0> > card_id_serie_def;
		struct card_id_serie: public bit_array<card_id_serie_def, card_id_serie>{};
		typedef boost::mpl::deque<
		device_uid, type_identifier, check_time, start_time, finish_time, last_visited_control, record_counter
		, don_t_care<0x01>
		, don_t_care<0x01>, card_id> card_8_header_def;

		struct card_8_header_type: public parameters_array<card_8_header_def>{};
		struct punch_type: public punch_4bytes_type<punch_type>{};

		typedef boost::mpl::deque<
		boost::mpl::pair<byte_type<0x01>, card_9>
		, boost::mpl::pair<byte_type<0x02>, card_8>
		, boost::mpl::pair<byte_type<0x04>, card_p>
		, boost::mpl::pair<byte_type<0x06>, card_t>
		, boost::mpl::pair<byte_type<0x0F>, card_15>
		> known_card_types;

		typedef boost::mpl::string<'card', ' 8/9', '/t/p'> description_t;

		inline static const char* get_description()
		{
			return boost::mpl::c_str<description_t>::value;
		}

		template<typename iterator>static bool read_header(card_record &readout, iterator datablock, std::size_t &punch_count)
		{
			card_8_header_type card_8_header;
			std::size_t max_size = 128;
			
			card_8_header.read_data(max_size, datablock);
			readout.get<card_record::CARD_ID>() = card_8_header.get<card_id>();
			readout.get<card_record::START_NO>() = 0;

			if(0 == (card_8_header.get<start_time>().get<control_number>() >> 9 ))
				readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_8_header.get<start_time>().get<time_12h>(), 0, card_8_header.get<start_time>().get<am_pm>());
			else
			{
				readout.get<card_record::START_SUBSECOND>() = true;
				readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_8_header.get<start_time>().get<time_12h>(), card_8_header.get<start_time>().get<control_number>() % 256, card_8_header.get<start_time>().get<am_pm>());
			}

			if(0 == (card_8_header.get<finish_time>().get<control_number>() >> 9 ))
				readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_8_header.get<finish_time>().get<time_12h>(), 0, card_8_header.get<finish_time>().get<am_pm>());
			else
			{
				readout.get<card_record::FINISH_SUBSECOND>() = true;
				readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_8_header.get<finish_time>().get<time_12h>(), card_8_header.get<finish_time>().get<control_number>() % 256, card_8_header.get<finish_time>().get<am_pm>());
			}

			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(card_8_header.get<check_time>().get<time_12h>(), 0, card_8_header.get<check_time>().get<am_pm>());
			readout.get<card_record::CLEAR_TIME>() = boost::posix_time::not_a_date_time;
			punch_count = card_8_header.get<record_counter>();
			return true;
		}
		template <typename iterator> static boost::uint8_t get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;

			record_counter rc;
			iterator item;

			rc.read_data(max_size, item = datablock + 0x18);
			return rc.value;
		}
		
		template<typename card_types = known_card_types, bool enabled = boost::mpl::empty<card_types>::value>struct card_type_getter
		{
			BOOST_MPL_ASSERT_MSG(!boost::mpl::empty<card_types>::value, NOT_A_SEQUENCE, (types<card_types>));
			static inline bool read(boost::uint8_t param, card_record &readout, blocks_read<common::read_out_data>& data)
			{
				if(param == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::read(readout, data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::read(param, readout,data);
			}
			static inline bool get_blocks_needed(boost::uint8_t param, needed_blocks_container &blocks, common::read_out_data& data)
			{
				if(param == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::get_blocks_needed(blocks, data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::get_blocks_needed(param, blocks,data);
			}
			static inline std::string& get_type_description(extended::si::value_type data)
			{
				if(data == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::get_type_description(data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::get_type_description(data);
			}
		};
		template<typename card_types>struct card_type_getter<card_types
				, true>
		{
			typedef card_8_family type;
			static inline bool read(boost::uint8_t , card_record &, blocks_read<common::read_out_data>& )
			{
				return false;
			}
			static inline bool get_blocks_needed(boost::uint8_t , needed_blocks_container &, common::read_out_data&)
			{
				return false;
			}
			static std::string& get_type_description(extended::si::value_type )
			{
				static std::string unsupported_card_type("unsupported card type");
				return unsupported_card_type;
			}
		};
        template <typename iterator> static bool internal_read(card_record &, iterator )
		{
			return false;
		}
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			return card_type_getter<>::read(data.card_serie, readout, data);
		}
		inline static boost::uint32_t get_id(extended::si::value_type data)
		{
			extended::si::value_type value =
					data & ~(extended::si::value_type(0xFF) << (8 * ( sizeof(extended::si::value_type) - 1)));

			return value;
		}
		static boost::uint32_t get_id(common::read_out_data& data)
		{
			card_id_serie id_serie;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.begin() + 0x18);
			id_serie.read_data(max_size, item);

			return id_serie.get<card_id>();
		}
		static boost::uint8_t get_serie(common::read_out_data& data)
		{
			card_id_serie id_serie;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.begin() + 0x18);
			id_serie.read_data(max_size, item);
			return id_serie.get<card_serie>();
		}
		static std::string& get_type_description(extended::si::value_type data)
		{
			extended::si::value_type value = data >> (8 * (sizeof(extended::si::value_type) - 1));

			return card_type_getter<>::get_type_description(value);
		}
		static std::string& get_type_description(extended::responses::si_card8_get& data)
		{
			card_id_serie id_serie;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.get<common::read_out_data>().begin() + 0x18);
			id_serie.read_data(max_size, item);
			return card_type_getter<>::get_type_description(id_serie.get<card_serie>());
		}
		static bool get_blocks_needed(boost::uint8_t card_serie, needed_blocks_container &blocks, common::read_out_data& data)
		{
			return card_type_getter<>::get_blocks_needed(card_serie, blocks, data);
		}
	};
	template<> struct card_reader<card_8>: public card_reader<card_8_family>
	{
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			std::size_t max_size = 128;
			std::size_t records_count;

			read_header(readout, datablock, records_count);
			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[1].begin();

			datablock += 0x08;
			max_size = 0x80 - 0x08;

			punch_type punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				punch.read_data(max_size, datablock);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>()
																								, card_reader<>::get_duration(punch.get<time_12h>(), 0, punch.get<am_pm>()));
			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			if(0 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string unsupported_card_type("card 8");
			return unsupported_card_type;
		}
	};
	template<> struct card_reader<card_9>: public card_reader<card_8_family>
	{
		template <typename iterator> static boost::uint8_t get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;
			record_counter rc;
			iterator item;

			rc.read_data(max_size, item = datablock + 0x16);
			return rc;
		}
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			std::size_t max_size = 128;
			std::size_t records_count;

			read_header(readout, datablock, records_count);
			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[0].begin();

			datablock += 0x38;
			max_size = 0x80 - 0x38;

			punch_type punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				if(18 == i)
				{
					datablock = data[1].begin();
					max_size = 128;
				}
				punch.read_data(max_size, datablock);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>()
																								, card_reader<>::get_duration(punch.get<time_12h>(), 0, punch.get<am_pm>()));
			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks,common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			if(18 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string unsupported_card_type("card 9");
			return unsupported_card_type;
		}
	};
	template<> struct card_reader<card_15>: public card_reader<card_8_family>
	{
		inline static unsigned get_sector_from_index(unsigned pos)
		{
			static const unsigned sectors[] = {4, 5, 6, 7};
			return sectors[pos];
		}
		template <typename iterator> static boost::uint8_t get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;
			record_counter rc;
			iterator item;

			rc.read_data(max_size, item = datablock + 0x16);
			return rc;
		}
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{

			typedef boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			std::size_t max_size = 128;
			std::size_t records_count;

			read_header(readout, datablock, records_count);
			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[6].begin();
			max_size = 128;

			iterator punch_base = datablock;
			iterator current_record = punch_base;

			punch_type punch;

			for(unsigned i = 0; 0 < records_count; i++, records_count--)
			{
				if(0 == (i % 32))
				{
					datablock = data[get_sector_from_index(i / 32)].begin();
					max_size = 128;

					punch_base = datablock;
					current_record = punch_base;
				}
				punch.read_data(max_size, current_record);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>().value
																								, card_reader<>::get_duration(punch.get<time_12h>().value, 0, punch.get<am_pm>()));

			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			unsigned sectors_needed = punches_count >> 5;
			if(0 != (punches_count % 32))
				sectors_needed++;

			for(; sectors_needed > 0; blocks.insert(get_sector_from_index(--sectors_needed)));
			return true;
		}
		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string card_type("card 10 or may be 11");
			return card_type;
		}
	};

	template<> struct card_reader<card_6>: public card_reader<>
	{
		struct start_no: public unsigned_integral_parameter<4, start_no>{};
		struct type_identifier: public unsigned_integral_parameter<4, type_identifier>{};
		struct start_time: public punch_4bytes_type<start_time>{};
		struct finish_time: public punch_4bytes_type<finish_time>{};
		struct check_time: public punch_4bytes_type<check_time>{};
		struct clear_time: public punch_4bytes_type<clear_time>{};
		struct last_visited_control: public unsigned_integral_parameter<2, last_visited_control>{};
		struct record_counter: public unsigned_integral_parameter<1, record_counter>{};
		struct next_record: public unsigned_integral_parameter<1, next_record>{};
		struct card_id: public unsigned_integral_parameter<4, card_id>{};
		struct last_punch: public punch_4bytes_type<last_punch>{};
		struct punch_type: public punch_4bytes_type<punch_type>{};


		typedef boost::mpl::deque<don_t_care<8>, don_t_care<2>
		, card_id, don_t_care<2>, last_visited_control, record_counter, next_record
		, finish_time, start_time, check_time, clear_time, don_t_care<4>, start_no
		> card_6_header_def;

		struct card_6_header_type: public parameters_array<card_6_header_def>{};

		typedef boost::mpl::string<'card', ' 6'> description_t;

		inline static const char* get_description()
		{
			return boost::mpl::c_str<description_t>::value;
		}
        inline static boost::uint8_t get_serie(common::read_out_data&)
		{
			return 6;
		}

		inline static unsigned get_sector_from_index(unsigned pos)
		{
			static const unsigned sectors[] = {6, 7, 2, 3, 4, 5};
			return sectors[pos];
		}
		inline static boost::uint32_t get_id(extended::si::value_type data)
		{
			return data;
		}

		static boost::uint32_t get_id(common::read_out_data& data)
		{
			typedef boost::tuples::element<extended::responses::si_card6_get::element<common::read_out_data>::type::value
					, extended::responses::si_card6_get>::type::iterator iterator;

			card_id id;

			std::size_t max_size = 128;
			iterator item;

			id.read_data(max_size, item = data.begin() + 0x0A);

			return id;
		}

		template <typename iterator> static boost::uint8_t get_punches_count(iterator datablock)
		{
			record_counter rc;

			std::size_t max_size = 128;
			iterator item;

			rc.read_data(max_size, item = datablock + 0x12);
			return rc;
		}
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card6_get::element<common::read_out_data>::type::value
					, extended::responses::si_card6_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			card_6_header_type card_6_header;

			std::size_t max_size = 128;

			card_6_header.read_data(max_size, datablock);
			readout.get<card_record::CARD_ID>() = card_6_header.get<card_id>();
			readout.get<card_record::START_NO>() = card_6_header.get<start_no>();
			if(0 == (card_6_header.get<start_time>().get<control_number>() >> 9 ))
				readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_6_header.get<start_time>().get<time_12h>(), 0, card_6_header.get<start_time>().get<am_pm>());
			else
			{
				readout.get<card_record::START_SUBSECOND>() = true;
				readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_6_header.get<start_time>().get<time_12h>(), card_6_header.get<start_time>().get<control_number>() % 256, card_6_header.get<start_time>().get<am_pm>());
			}

			if(0 == (card_6_header.get<finish_time>().get<control_number>() >> 9 ))
				readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_6_header.get<finish_time>().get<time_12h>(), 0, card_6_header.get<finish_time>().get<am_pm>());
			else
			{
				readout.get<card_record::FINISH_SUBSECOND>() = true;
				readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_6_header.get<finish_time>().get<time_12h>(), card_6_header.get<finish_time>().get<control_number>() % 256, card_6_header.get<finish_time>().get<am_pm>());
			}
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(card_6_header.get<check_time>().get<time_12h>(), 0, card_6_header.get<check_time>().get<am_pm>());
			readout.get<card_record::CLEAR_TIME>() = card_reader<>::get_duration(card_6_header.get<clear_time>().get<time_12h>(), 0, card_6_header.get<clear_time>().get<am_pm>());
			std::size_t records_count = card_6_header.get<record_counter>();

			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[6].begin();
			max_size = 128;

			iterator punch_base = datablock;
			iterator current_record = punch_base;

			punch_type punch;

			for(unsigned i = 0; 0 < records_count; i++, records_count--)
			{
				if(0 == (i % 32))
				{
					datablock = data[get_sector_from_index(i / 32)].begin();
					max_size = 128;

					punch_base = datablock;
					current_record = punch_base;
				}
				punch.read_data(max_size, current_record);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>().value
																								, card_reader<>::get_duration(punch.get<time_12h>().value, 0, punch.get<am_pm>()));

			}
			
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			unsigned sectors_needed = punches_count >> 5;
			if(0 != (punches_count % 32))
				sectors_needed++;
			for(; sectors_needed > 0; blocks.insert(get_sector_from_index(--sectors_needed)));
			return true;
		}

        static inline bool get_blocks_needed(boost::uint8_t , needed_blocks_container &blocks, common::read_out_data& data)
		{
			return get_blocks_needed(blocks, data);
		}

		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string card_type("card 6");
			return card_type;
		}


	};
	template<> struct card_reader<card_p>: public card_reader<card_8_family>
	{
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			std::size_t max_size = 128;
			std::size_t records_count;

			read_header(readout, datablock, records_count);
			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[1].begin() + 0x30;

			max_size = 0x80 - 0x30;

			punch_type punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				punch.read_data(max_size, datablock);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>()
																								, card_reader<>::get_duration(punch.get<time_12h>(), 0, punch.get<am_pm>()));
			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			if(0 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string card_type("card p");
			return card_type;
		}
	};
	template<> struct card_reader<card_t>: public card_reader<card_8_family>
	{
		struct punch_type: public punch_8bytes_type<punch_type>{};
		static bool read(card_record &readout, blocks_read<common::read_out_data>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card8_get::element<common::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0].begin();

			std::size_t max_size = 128;
			std::size_t records_count;

			read_header(readout, datablock, records_count);
			if(0 == records_count)
			{
				readout.get<card_record::PUNCH_RECORDS>().clear();
				return true;
			}

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			datablock = data[0].begin();

			datablock += 0x38;
			max_size = 0x80 - 0x38;

			punch_type punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				if(9 == i)
				{
					datablock = data[1].begin();
					max_size = 128;
				}
				punch.read_data(max_size, datablock);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>()
																								, card_reader<>::get_duration(punch.get<time_12h>(), 0, punch.get<am_pm>()));
			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, common::read_out_data& data)
		{
			boost::uint8_t punches_count(get_punches_count(data.begin()));
			blocks.insert(0);
			if(9 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type )
		{
			static std::string card_type("card t");
			return card_type;
		}
	};
}//namespace si
