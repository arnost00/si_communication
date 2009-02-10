#pragma once

#include <set>
#include "commands_definitions.h"
#include "card_record.h"
#include "blocks_read.h"

namespace si
{

	struct one_byte_type: public unsigned_integral_parameter<1, one_byte_type>{};
	struct two_bytes_type: public unsigned_integral_parameter<2, two_bytes_type>{};
	struct three_bytes_type: public unsigned_integral_parameter<3, three_bytes_type>{};
	struct four_bytes_type: public unsigned_integral_parameter<4, four_bytes_type>{};


	struct card_type{};
	struct card_5: public card_type{};
	struct card_6: public card_type{};
	struct card_6star: public card_6{};
	struct card_8: public card_type{};
	struct card_9: public card_type{};
	struct card_p: public card_type{};
	struct card_t: public card_type{};
	struct card_8_family: public card_8, public card_9, public card_p, public card_t{};


	template<typename card_type = void> struct card_reader
	{
		struct control_number: public unsigned_integral_parameter<2, control_number>{};
		struct time_12h: public unsigned_integral_parameter<2, time_12h>{};
		struct am_pm: public unsigned_integral_parameter<1, am_pm>{};
		struct day_of_week: public unsigned_integral_parameter<1, day_of_week>{};
		struct week_counter: public unsigned_integral_parameter<1, week_counter>{};
		struct card5_id_part: public unsigned_integral_parameter<1, card5_id_part>{};
		struct card5_serie_part: public unsigned_integral_parameter<2, card5_serie_part>{};

		typedef boost::mpl::deque<typename control_number::bits_range<7,0> > punch_1byte_def;
		typedef boost::mpl::deque<typename control_number::bits_range<7,0>, typename time_12h::bits_range<15,0> > punch_3bytes_def;
		typedef boost::mpl::deque<typename control_number::bits_range<9,8>
			, typename day_of_week::bits_range<1,0>
			, typename week_counter::bits_range<2,0>
			, typename am_pm::bits_range<0>
			, typename control_number::bits_range<7,0>
			, typename time_12h::bits_range<15,0> > punch_4bytes_def;
		typedef boost::mpl::deque<card5_id_part, card5_serie_part> card5_id_def;

		template<typename T> struct punch_1byte_type: public bit_array<punch_1byte_def, T>{};
		template<typename T> struct punch_3bytes_type: public bit_array<punch_3bytes_def, T>{};
		template<typename T> struct punch_4bytes_type: public bit_array<punch_4bytes_def, T>{};

		struct card5_id_type: public bit_array<card5_id_def, card5_id_type>{};

		static inline boost::posix_time::time_duration get_duration(unsigned __int16 source)
		{
			return 0xEEEE == source
				? boost::posix_time::time_duration(boost::posix_time::not_a_date_time)
				: boost::posix_time::seconds(source);
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

		struct card_5_header_type: public parameters_array<card_5_header_def, card_5_header_type>{};
		struct punch_type: public punch_3bytes_type<punch_type>{};
		struct extra_punch_type: public punch_1byte_type<extra_punch_type>{};


		template <typename iterator> static bool internal_read(card_record &readout, iterator datablock)
		{

			iterator item;
			iterator data_item = datablock;

			std::size_t max_size = 128;


			card_5_header_type card_5_header;
			
			card_5_header.read_data(max_size, data_item);
			readout.get<card_record::CARD_ID>() = card_5_header.get<card_id>() + (1 == card_5_header.get<card_serie>()? 0: 100000 * card_5_header.get<card_serie>());

			readout.get<card_record::START_NO>() = card_5_header.get<card_id>();
			readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_5_header.get<start_time>());
			readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_5_header.get<finish_time>());
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(card_5_header.get<check_time>());
			readout.get<card_record::CLEAR_TIME>() = boost::posix_time::not_a_date_time;
			std::size_t records_count = card_5_header.get<record_counter>() - 1;


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
			return internal_read(readout, data.get<extended::responses::read_out_data>().begin());
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

		typedef boost::mpl::deque<
			device_uid, type_identifier, check_time, start_time, finish_time, last_visited_control, record_counter
			, don_t_care<0x01>
			, don_t_care<0x01>, card_id> card_8_header_def;

		struct card_8_header_type: public parameters_array<card_8_header_def, card_8_header_type>{};
		struct punch_type: public punch_4bytes_type<punch_type>{};

		typedef boost::mpl::deque<
			  boost::mpl::pair<byte_type<0x01>, card_9>
			, boost::mpl::pair<byte_type<0x02>, card_8>
			, boost::mpl::pair<byte_type<0x04>, card_p>
			, boost::mpl::pair<byte_type<0x06>, card_t>
		> known_card_types;
      template<typename card_types = known_card_types, bool enabled = boost::mpl::empty<card_types>::value>struct card_type_getter
		{
         BOOST_MPL_ASSERT_MSG(!boost::mpl::empty<card_types>::value, NOT_A_SEQUENCE, (types<card_types>));
			static inline bool read(si::byte param, card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
			{
				if(param == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::read(readout, data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::read(param, readout,data);
			}
			static inline bool get_blocks_needed(si::byte param, needed_blocks_container &blocks, extended::responses::si_card8_get& data)
			{
				if(param == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::get_blocks_needed(blocks, data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::get_blocks_needed(param, blocks,data);
			}
			static inline std::string& get_type_description(si::byte param, extended::si::value_type& data)
			{
				if(param == boost::mpl::first<typename boost::mpl::front<card_types>::type>::type::value)
				{
					return card_reader<typename boost::mpl::second<typename boost::mpl::front<card_types>::type>::type>::get_type_description(data);
				}
				return card_type_getter<typename boost::mpl::pop_front<card_types>::type>::get_type_description(param, data);
			}
/*			typedef typename boost::mpl::front<card_types>::type checked_type;
			typedef typename boost::mpl::if_c<boost::mpl::second<checked_type>::type::value == serie
				, typename boost::mpl::second<checked_type>
				, card_type_getter<typename boost::mpl::pop_front<card_types>::type, serie> >::type::type type;
				*/
		};
		template<typename card_types>struct card_type_getter<card_types
			, true>
		{
			typedef card_8_family type;
			static inline bool read(si::byte parameter, card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
			{
				return false;
			}
			static inline bool get_blocks_needed(si::byte parameter, needed_blocks_container &blocks, extended::responses::si_card8_get& data)
			{
				return false;
			}
			static std::string& get_type_description(si::byte parameter, extended::si::value_type& data)
			{
				static std::string unsupported_card_type("unsupported card type");
				return unsupported_card_type;
			}
		};
		template <typename iterator> static bool internal_read(card_record &readout, iterator datablock)
		{
			return false;
		}
		static bool read(card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
		{
			return card_type_getter<>::read(data.card_serie, readout, data);
		}
		static unsigned __int32 get_id(extended::si::value_type& data)
		{
			extended::si::value_type value = 
				data & ~(extended::si::value_type(0xFF) << (8 * ( sizeof(extended::si::value_type) - 1)));

			return value;
		}
		static unsigned __int32 get_id(extended::responses::si_card8_get& data)
		{
			four_bytes_type four_bytes;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<extended::responses::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.get<extended::responses::read_out_data>().begin() + 0x18);
			four_bytes.read_data(max_size, item);

			return get_id(four_bytes.value);
		}

		static si::byte get_serie(extended::si::value_type& data)
		{
			return data >> (8 * (sizeof(extended::si::value_type) - 1));
		}
		static si::byte get_serie(extended::responses::si_card8_get& data)
		{
			four_bytes_type four_bytes;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<extended::responses::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.get<extended::responses::read_out_data>().begin() + 0x18);
			four_bytes.read_data(max_size, item);
			return get_serie(four_bytes.value);
		}
		static std::string& get_type_description(extended::si::value_type& data)
		{
			extended::si::value_type value = data >> (8 * (sizeof(extended::si::value_type) - 1));

			return card_type_getter<>::get_type_description(value, data);
		}
		static std::string& get_type_description(extended::responses::si_card8_get& data)
		{
			four_bytes_type four_bytes;

			std::size_t max_size = 128;

			boost::tuples::element<extended::responses::si_card8_get::element<extended::responses::read_out_data>::type::value
					, extended::responses::si_card8_get>::type::iterator item(data.get<extended::responses::read_out_data>().begin() + 0x24);
			four_bytes.read_data(max_size, item);
			return get_type_description(four_bytes.value);
		}
		static bool get_blocks_needed(si::byte card_serie, needed_blocks_container &blocks, extended::responses::si_card8_get& data)
		{
			return card_type_getter<>::get_blocks_needed(card_serie, blocks, data);
		}
	};
	template<> struct card_reader<card_8>: public card_reader<>
	{
		template <typename iterator> static si::byte get_punches_count(iterator datablock)
		{
			one_byte_type one_byte;

			std::size_t max_size = 128;
			iterator item;

			one_byte.read_data(max_size, item = datablock + 0x18);

			return one_byte.value;
		}
		static inline bool read(card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
		{
			two_bytes_type two_bytes;
			three_bytes_type three_bytes;
			one_byte_type one_byte;

			typedef boost::tuples::element<extended::responses::si_card8_get::element<extended::responses::read_out_data>::type::value
				, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0]->get<extended::responses::read_out_data>().begin();
			iterator item;

			std::size_t max_size = 128;

			three_bytes.read_data(max_size, item = datablock + 0x19);
			one_byte.read_data(max_size, item = datablock + 0x18);
			readout.get<card_record::CARD_ID>() = three_bytes.value;
			two_bytes.read_data(max_size, item = datablock + 0x0E);
			readout.get<card_record::START_TIME>() = card_reader<>::get_duration(two_bytes.value);
			two_bytes.read_data(max_size, item = datablock + 0x12);
			readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(two_bytes.value);
			two_bytes.read_data(max_size, item = datablock + 0x0A);
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(two_bytes.value);
			readout.get<card_record::CLEAR_TIME>() = boost::posix_time::not_a_date_time;
			one_byte.read_data(max_size, item = datablock + 0x16);
			unsigned records_count = one_byte.value;
			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			if(0 == records_count)
				return true;

			datablock = data[1]->get<extended::responses::read_out_data>().begin();

			iterator current_record = datablock + 0x08;
			for(unsigned i = 0; 0 < records_count; i++, records_count--)
			{
				current_record++;
				one_byte.read_data(max_size, current_record);
				two_bytes.read_data(max_size, current_record);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(one_byte.value
					, card_reader<>::get_duration(two_bytes.value));
			}
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, extended::responses::si_card8_get& data)
		{
			si::byte punches_count(get_punches_count(data.get<extended::responses::read_out_data>().begin()));
			blocks.insert(0);
			if(0 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type& data)
		{
			static std::string unsupported_card_type("card 8");
			return unsupported_card_type;
		}
	};
	template<> struct card_reader<card_6>: public card_reader<>
	{
		struct punch_type: public punch_4bytes_type<punch_type>{};

		static unsigned __int32 get_id(extended::responses::si_card6_get& data)
		{
			typedef boost::tuples::element<extended::responses::si_card6_get::element<extended::responses::read_out_data>::type::value
				, extended::responses::si_card6_get>::type::iterator iterator;

			four_bytes_type four_bytes;

			std::size_t max_size = 128;
			iterator item;

			four_bytes.read_data(max_size, item = data.get<extended::responses::read_out_data>().begin() + 0x0A);

			return four_bytes.value;
		}

		template <typename iterator> static si::byte get_punches_count(iterator datablock)
		{
			one_byte_type one_byte;

			std::size_t max_size = 128;
			iterator item;

			one_byte.read_data(max_size, item = datablock + 0x12);
			return one_byte.value;
		}
		static inline bool read(card_record &readout, blocks_read<extended::responses::si_card6_get>& data)
		{
			static const unsigned sectors[] = {6, 7, 2, 3, 4, 5};

			two_bytes_type two_bytes;
			four_bytes_type four_bytes;
			one_byte_type one_byte;

			typedef boost::tuples::element<extended::responses::si_card6_get::element<extended::responses::read_out_data>::type::value
				, extended::responses::si_card6_get>::type::iterator iterator;
			iterator datablock = data[0]->get<extended::responses::read_out_data>().begin();
			iterator item;

			std::size_t max_size = 128;

			four_bytes.read_data(max_size, item = datablock + 0x0A);
			readout.get<card_record::CARD_ID>() = four_bytes.value;
			four_bytes.read_data(max_size, item = datablock + 0x18);
			readout.get<card_record::START_NO>() = four_bytes.value;
			two_bytes.read_data(max_size, item = datablock + 0x1A);
			readout.get<card_record::START_TIME>() = card_reader<>::get_duration(two_bytes.value);
			two_bytes.read_data(max_size, item = datablock + 0x16);
			readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(two_bytes.value);
			two_bytes.read_data(max_size, item = datablock + 0x1E);
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(two_bytes.value);
			two_bytes.read_data(max_size, item = datablock + 0x22);
			readout.get<card_record::CLEAR_TIME>() = card_reader<>::get_duration(two_bytes.value);
			one_byte.read_data(max_size, item = datablock + 0x12);
			unsigned records_count = one_byte.value;
			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			if(0 == records_count)
				return true;

			datablock = data[6]->get<extended::responses::read_out_data>().begin();

			iterator punch_base = datablock;
			iterator current_record = punch_base;

			punch_type punch;

			for(unsigned i = 0; 0 < records_count; i++, records_count--)
			{
				if(0 == (i % 32))
				{
					datablock = data[sectors[i / 32]]->get<extended::responses::read_out_data>().begin();
					max_size = 128;

					punch_base = datablock;
					current_record = punch_base;
				}
            punch.read_data(max_size, current_record);
            readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>().value
               , card_reader<>::get_duration(punch.get<time_12h>().value));

			}
			
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, extended::responses::si_card6_get& data)
		{
			static const unsigned sectors[] = {6, 7, 2, 3, 4, 5};

			si::byte punches_count(get_punches_count(data.get<extended::responses::read_out_data>().begin()));
			blocks.insert(0);
			unsigned sectors_needed = punches_count >> 5;
			if(0 != (punches_count % 32))
				sectors_needed++;
			for(; sectors_needed > 0; blocks.insert(sectors[--sectors_needed]));
			return true;
		}
	};
	template<> struct card_reader<card_p>: public card_reader<>
	{
		template <typename iterator> static si::byte get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;

			one_byte_type one_byte;
			iterator item;

			one_byte.read_data(max_size, item = datablock + 0x18);
			return one_byte.value;
		}
		static inline bool read(card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
		{
			return false;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, extended::responses::si_card8_get& data)
		{
			si::byte punches_count(get_punches_count(data.get<extended::responses::read_out_data>().begin()));
			blocks.insert(0);
			if(0 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type& data)
		{
			static std::string card_type("card p");
			return card_type;
		}
	};
	template<> struct card_reader<card_t>: public card_reader<>
	{
		template <typename iterator> static si::byte get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;

			one_byte_type one_byte;
			iterator item;
			one_byte.read_data(max_size, item = datablock + 0x18);
			return one_byte.value;
		}
		static inline bool read(card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
		{
			return false;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, extended::responses::si_card8_get& data)
		{
			si::byte punches_count(get_punches_count(data.get<extended::responses::read_out_data>().begin()));
			blocks.insert(0);
			if(19 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type& data)
		{
			static std::string card_type("card t");
			return card_type;
		}
	};
	template<> struct card_reader<card_9>: public card_reader<card_8_family>
	{
		template <typename iterator> static si::byte get_punches_count(iterator datablock)
		{
			std::size_t max_size = 128;
			one_byte_type one_byte;
			iterator item;

			one_byte.read_data(max_size, item = datablock + 0x16);
			return one_byte.value;
		}
		static inline bool read(card_record &readout, blocks_read<extended::responses::si_card8_get>& data)
		{
			typedef boost::tuples::element<extended::responses::si_card8_get::element<extended::responses::read_out_data>::type::value
				, extended::responses::si_card8_get>::type::iterator iterator;
			iterator datablock = data[0]->get<extended::responses::read_out_data>().begin();

			card_8_header_type card_8_header;
			std::size_t max_size = 128;
			
			card_8_header.read_data(max_size, datablock);
			readout.get<card_record::CARD_ID>() = card_8_header.get<card_id>();

			readout.get<card_record::START_TIME>() = card_reader<>::get_duration(card_8_header.get<start_time>().get<time_12h>());
			readout.get<card_record::FINISH_TIME>() = card_reader<>::get_duration(card_8_header.get<finish_time>().get<time_12h>());
			readout.get<card_record::CHECK_TIME>() = card_reader<>::get_duration(card_8_header.get<check_time>().get<time_12h>());
			readout.get<card_record::CLEAR_TIME>() = boost::posix_time::not_a_date_time;
			std::size_t records_count = card_8_header.get<record_counter>();

			readout.get<card_record::PUNCH_RECORDS>().resize(records_count);

			if(0 == records_count)
				return true;

			datablock = data[0]->get<extended::responses::read_out_data>().begin();

			datablock += 0x38;
			max_size = 0x80 - 0x38;

			punch_type punch;

			for(std::size_t i = 0; 0 < records_count; i++, records_count--)
			{
				if(18 == i)
				{
					datablock = data[1]->get<extended::responses::read_out_data>().begin();
					max_size = 128;
				}
				punch.read_data(max_size, datablock);
				readout.get<card_record::PUNCH_RECORDS>()[i] = punch_record(punch.get<control_number>()
					, card_reader<>::get_duration(punch.get<time_12h>()));
			}			
			return true;
		}
		static inline bool get_blocks_needed(needed_blocks_container &blocks, extended::responses::si_card8_get& data)
		{
			si::byte punches_count(get_punches_count(data.get<extended::responses::read_out_data>().begin()));
			blocks.insert(0);
			if(19 < punches_count)
			{
				blocks.insert(1);
			}
			return true;
		}
		static std::string& get_type_description(extended::si::value_type& data)
		{
			static std::string unsupported_card_type("card 9");
			return unsupported_card_type;
		}
	};
}//namespace si
