//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "crc529.h"
#include "si_constants.h"
#include "si_auxilary.h"

#include <boost/cstdint.hpp>

namespace si
{
	template<typename protocol = void> struct protocol_encoder
	{
		template<typename encoder_type> static inline void encode(command_interface::pointer command, std::size_t &size, channel_protocol_interface::data_type& result)
		{
			command_interface::data_type data;
			std::size_t data_size;
			boost::uint8_t command_id = command->get_id();
			command->get_data(data_size, data);

			size = protocol_encoder<encoder_type>::get_size(command_id, data_size, data.get());
			result.reset(new boost::uint8_t[size]);
			protocol_encoder<encoder_type>::write_data(command_id, data_size, data.get(), size, result.get());
		}
		template<typename iterator> static void detect_command(std::size_t& size, iterator &it)
		{
			BOOST_MPL_ASSERT_MSG(false, PROTOCOL_HAS_TO_BE_SPECIFIED, (types<protocol>));
		}
	};

	template<> struct protocol_encoder<protocols::basic>
	{
		inline static bool is_prefixed_data_byte(boost::uint8_t data_byte)
		{
			return data_byte <= 0x1F;
		}
		template<typename iterator> static std::size_t get_size(boost::uint8_t command, std::size_t in_size, iterator it)
		{
			std::size_t size = sizeof(command) + sizeof(STX::value) + sizeof(ETX::value) + in_size;
			for(std::size_t i = 0; i < in_size; size++, i++)
			{
				if(is_prefixed_data_byte(*it))
				{
					size += sizeof(DLE::value);
				}
			}
			return size;
		}
		template<typename iterator_in, typename iterator_out> static void write_data(boost::uint8_t command
																											  , std::size_t in_size, iterator_in it
																											  , std::size_t , iterator_out out_it)
		{
			*out_it++ = STX::value;
			*out_it++ = command;

			for(std::size_t i = 0; i < in_size; i++)
			{
				if(is_prefixed_data_byte(*it))
				{
					*out_it++ = DLE::value;
				}
				*out_it++ = *it++;
			}
			*out_it++ = ETX::value;

			return;
		}

		template<typename iterator> static std::size_t detect_command_data_size(std::size_t& size, iterator &it)
		{
			iterator work_it = it;
			bool prefixed_char = false;
			std::size_t data_size = 0;
			std::size_t processed_size = 0;
			for(;processed_size < size; work_it++, processed_size++)
			{
				if(prefixed_char || (0x20 <= (*work_it)))
				{
					data_size++;
					prefixed_char = false;
					continue;
				}
				else
				{
					switch(*work_it)
					{
					case DLE::value:
						prefixed_char = true;
						continue;
					case ETX::value:
						return data_size;
					default:
						throw std::invalid_argument("channel_protocol<protocols::basic>::detect_command_data_size: invalid input");
					}
				}
			}
			return unknown_size;
		}
		template<typename iterator, typename out_iterator> static void read_command_data(std::size_t& size, iterator &it, std::size_t& data_size,  out_iterator out_it)
		{
			bool prefixed_char = false;

			for(std::size_t processed_size = 0; processed_size < data_size; it++)
			{
				size --;
				if(prefixed_char || (0x20 <= (*it)))
				{
					*out_it++ = *it;
					prefixed_char = false;
					processed_size++;
					continue;
				}
				else
				{
					switch(*it)
					{
					case DLE::value:
						prefixed_char = true;
						continue;
					case ETX::value:
						throw std::invalid_argument("channel_protocol<protocols::basic>::read_command_data: invalid input: ETX not supposed to be part of data");
					default:
						throw std::invalid_argument("channel_protocol<protocols::basic>::read_command_data: invalid input: unkonwn control charachter");
					}
				}
			}
		}
		template<typename iterator> static bool extract_command(std::size_t& size, iterator &it, channel_protocol_interface::callback_type callback)
		{
			iterator backup_it = it;
			std::size_t backup_size = size;
			iterator subcall_it;
			std::size_t subcall_size;

			bool control_commands_allowed(true);

//			LOG << "Basic extract size: " << size << std::endl;
			command_interface::id_type command_id;
			bool retval(false);

			std::size_t data_size;


			for(; 3 <= size; it++, size --)//STX, command, ETX
			{
				if(STX::value != (*it))
				{
					continue;
				}
				control_commands_allowed = false;

				it++;
				size--;
				command_id = *it;


				if((command_id >= 0x80 ) && (command_id != 0xC4))
					continue;

				it++;
				size--;

				try
				{
					subcall_it = it;
					subcall_size = size;
					data_size = detect_command_data_size(size, it);
				}
				catch(std::exception &e)
				{
					LOG << e.what();
					size = subcall_size;
					it = subcall_it;
					continue;
				}

				if((unknown_size == data_size) ||(data_size + 1 > size))
				{
					it = backup_it;
					size = backup_size;
//					LOG << "Basic extract size on unkown data size: " << size << std::endl;
					return retval;
				}

				command_interface::data_type data(new boost::uint8_t[data_size]);
				read_command_data(size, it, data_size, data.get());

				if(ETX::value != *it)
					continue;
				backup_it = it;
				backup_size = size;

				backup_it++;
				backup_size--;

				callback(command_id, data_size, data, false);

				control_commands_allowed = true;
				retval  = true;
			}
			size = backup_size;
			it = backup_it;
//			LOG << "Basic extract size on return: " << size << std::endl;
			return retval;
		}
		template<typename iterator>static bool detect_command(std::size_t& size, iterator &it, channel_protocol_interface::callback_type callback)
		{
			std::size_t processed_size = 0;
			while(processed_size < size)
			{
				try
				{
					switch(*it)
					{
					case ACK::value:
					case NAK::value:
						callback(*it, 0, command_interface::data_type(), true);
						return true;
					case STX::value:
						if(extract_command(size, it, callback))
						{
							return true;
						}
						else
						{
							LOG << "Extract command failed" << std::endl;
//							log_data(it, size);
							return false;
						}
					default:
						it++;
						processed_size++;
					}
				}
				catch(std::exception const& e)
				{
					LOG << e.what() << std::endl;
//					LOG << "Data:" << std::endl;
//					log_data(it, size);

					it++;
					processed_size++;
				}
			}
			return false;
		}
		static void encode(command_interface::pointer command, std::size_t &size, channel_protocol_interface::data_type& result)
		{
			command_interface::data_type data;
			std::size_t data_size;
			boost::uint8_t command_id = command->get_id();
			command->get_data(data_size, data);

			size = get_size(command_id, data_size, data.get());
			result.reset(new boost::uint8_t[size]);
			write_data(command_id, data_size, data.get(), size, result.get());
		}

	};

	template<> struct protocol_encoder<protocols::extended>
	{
		template<typename iterator> static std::size_t get_size(boost::uint8_t command, std::size_t in_size, iterator , bool ff_start = false, bool extra_stx = false)
		{
			return sizeof(command) + sizeof(STX::value) + sizeof(ETX::value) + 3/*size+crc*/ + in_size + (ff_start? 1: 0) + (extra_stx? 1: 0);
		}
		template<typename iterator_in, typename iterator_out> static void write_data(boost::uint8_t command
																											  , std::size_t in_size, iterator_in it
																											  , std::size_t , iterator_out out_it
																											  , bool ff_start = false, bool extra_stx = false)
		{
			if(ff_start)
				*out_it++ = 0xFF;
			if(extra_stx)
				*out_it++ = STX::value;

			*out_it++ = STX::value;

			iterator_out crc_it = out_it;

			*out_it++ = command;
			*out_it++ = boost::uint8_t(in_size);

			for(std::size_t i = 0; i < in_size; i++)
			{
				*out_it++ = *it++;
			}
			/*
		boost::crc_16_type crc_value;
		crc_value.process_block(crc_it, out_it);
*/
			boost::uint16_t crc_value = crc(in_size + 2, crc_it);

			*out_it++ = crc_value>>8 & 0xFF;
			*out_it++ = crc_value & 0xFF;
			*out_it++ = ETX::value;

			return;
		}
		template<typename iterator> static std::size_t detect_command_data_size(std::size_t& size, iterator &it)
		{
			if(0 < size)
			{
				size--;
				return *it++;
			}
			return unknown_size;
		}

		template<typename iterator, typename out_iterator> static void read_command_data(std::size_t& size, iterator &it, std::size_t& out_size,  out_iterator out_it)
		{
			for(std::size_t i = 0; i < out_size; i++)
			{
				*out_it++ = *it++;
				size--;
			}
		}


		template<typename iterator> static bool extract_command(std::size_t& size, iterator &it, channel_protocol_interface::callback_type callback)
		{
			iterator backup_it = it;
			std::size_t backup_size = size;
			command_interface::id_type command_id;

//			LOG << "Extended extract size: " << size << std::endl;

			if(3 >= size)//STX, command, ETX
			{
				return false;
			}
			if(STX::value != (*it))
			{
				throw std::invalid_argument("channel_protocol<protocols::extended>::extract_command: invalid input: missing STX");
			}
			it++;
			size--;

			iterator crc_it = it;

			command_id = *it++;
			size--;
			if((command_id < 0x80 ) || (command_id == 0xC4))
			{
				try
				{
					return protocol_encoder<protocols::basic>::extract_command(size = backup_size, it = backup_it, callback);
				}
				catch(std::exception const& e)
				{
					LOG << e.what() << std::endl;
//					LOG << "Data:" << std::endl;
//					log_data(it, size);

					it = backup_it;
					size = backup_size;
					return false;
//					LOG << "Extended extract size catch exit: " << size << std::endl;
				}

			}

			std::size_t data_size = detect_command_data_size(size, it);
			if(unknown_size == data_size)
			{
				it = backup_it;
				size = backup_size;
//				LOG << "Extended extract size unknown data size exit: " << size << std::endl;
				return false;
			}

			if(size < data_size + 3)//data size + CRC1 + CRC0 + ETX
			{
				it = backup_it;
				size = backup_size;
//				LOG << "Extended extract size short data exit: " << size << std::endl;
				return false;
			}
			command_interface::data_type data(new boost::uint8_t[data_size]);
			read_command_data(size, it, data_size, data.get());

			boost::uint16_t crc_value;
			crc_value = *it++;
			crc_value <<= 8;
			crc_value |= *it++;
			if(crc(data_size + 2, crc_it) != crc_value)
			{
				throw std::invalid_argument("channel_protocol<protocols::extended>::extract_command: invalid input: crc check failed");
			}
			if(ETX::value != *it++)
			{
				throw std::invalid_argument("channel_protocol<protocols::extended>::extract_command: invalid input: missing ETX");
			}
			size -= 3;
			callback(command_id, data_size, data, false);
			return true;
		}

		template<typename iterator> static bool detect_command(std::size_t& size, iterator &it, channel_protocol_interface::callback_type callback)
		{

			iterator backup_it = it;
			std::size_t backup_size = size;
			iterator subcall_it;
			std::size_t subcall_size;
			bool control_commands_allowed(true);


			command_interface::id_type command_id;
			bool retval(false);

			std::size_t data_size;

			for(; 0 < size; it++, size --)//STX, command, ETX
			{
				if(control_commands_allowed &&((ACK::value == (*it)) || (NAK::value == (*it))))
				{
//					LOG << "ACK/NACK command" << std::endl;
					callback(*it, 0, command_interface::data_type(), true);

					backup_it = it;
					backup_size = size;

					backup_it++ ;
					backup_size--;

					retval = true;
					continue;
				}

				if(STX::value != (*it))
					continue;

				control_commands_allowed = false; //we found stx, don't want to detect ack/nak from stream if parse of command fails

				it++;
				size--;

				iterator crc_it = it;

				command_id = *it;
//				LOG << "command: " << (unsigned)(command_id)<< std::endl;

				if((command_id < 0x80 ) || (command_id == 0xC4))
				{
					subcall_it = it;
					subcall_size = size;
					bool subcall_return = protocol_encoder<protocols::basic>::extract_command(size = backup_size, it = backup_it, callback);
					if(subcall_return)
					{
						backup_it = it;
						backup_size = size;
						retval = true;
						it --;
						size ++;
						control_commands_allowed = true;
						continue;
					}
					else
					{
						LOG << "basic command extract failed"<< std::endl;
//						log_data(backup_it, backup_size);
						size = subcall_size;
						it = subcall_it;
						continue;
					}

				}
				try
				{
					subcall_it = ++it;
					subcall_size = --size;
					data_size = detect_command_data_size(size, it);
				}
				catch(std::exception &e)
				{
					size = subcall_size;
					it = subcall_it;
//					LOG << "extended command detect size failed: " << e.what() << std::endl;
					continue;
				}

				if((unknown_size == data_size) ||(data_size + 3 > size))
				{
					it = backup_it;
					size = backup_size;
					return retval;
				}

				command_interface::data_type data(new boost::uint8_t[data_size]);
				read_command_data(size, it, data_size, data.get());

				boost::uint16_t crc_value;
				crc_value = *it++;
				crc_value <<= 8;
				crc_value |= *it++;
				if(crc(data_size + 2, crc_it) != crc_value)
				{
					throw std::invalid_argument("channel_protocol<protocols::extended>::extract_command: invalid input: crc check failed");
				}
				if(ETX::value != *it)
				{
					throw std::invalid_argument("channel_protocol<protocols::extended>::extract_command: invalid input: missing ETX");
				}
				size -= 2;

				if(ETX::value != *it)
					continue;


				backup_it = it;
				backup_size = size;

				backup_it++;
				backup_size--;

				callback(command_id, data_size, data, false);
				retval  = true;
			}
			size = backup_size;
			it = backup_it;
			return retval;
		}
		static void encode(command_interface::pointer command, std::size_t &size, channel_protocol_interface::data_type& result)
		{
			command_interface::data_type data;
			std::size_t data_size;
			boost::uint8_t command_id = command->get_id();
			command->get_data(data_size, data);

			size = get_size(command_id, data_size, data.get(), true, true);
			result.reset(new boost::uint8_t[size]);
			write_data(command_id, data_size, data.get(), size, result.get(), true, true);
		}

	};

	template<> struct protocol_encoder<control_sequence>
	{
		template<typename iterator> static std::size_t get_size(boost::uint8_t command, std::size_t , iterator )
		{
			return sizeof(command);
		}
		template<typename iterator_in, typename iterator_out> static void write_data(boost::uint8_t command
																											  , std::size_t , iterator_in
																											  , std::size_t , iterator_out out_it)
		{
			*out_it++ = command;
		}

	};

	template<typename protocols_tt = protocols::supported_set, class enabled = void>struct by_protocol
	{
		template<typename protocol_tt> struct encoder
		{
			template <typename iterator> static void inline apply(boost::uint8_t command, std::size_t size, iterator it)
			{
				std::size_t encoded_size = protocol_encoder<protocol_tt>::get_size(command, size, it);
				boost::shared_array<boost::uint8_t> data_binary(new boost::uint8_t[encoded_size]);
				protocol_encoder<protocol_tt>::write_data(command, size, it, encoded_size, data_binary.get());
			}
		};
		template <typename iterator> static void inline find_encoder(protocols::id<>::value_type protocol_id, boost::uint8_t command, std::size_t size, iterator it)
		{
			if(0 == protocol_id)
			{
				encoder<typename boost::mpl::front<protocols_tt>::type>::apply(command, size, it);
			}
			else
			{
				by_protocol<typename boost::mpl::pop_front<protocols_tt>::type>::find_encoder(protocol_id - 1, command, size, it);
			}
		}
		template <typename iterator> static void encode(protocols::id<>::value_type protocol_id, boost::uint8_t command, std::size_t size, iterator it)
		{
			if(protocol_id >= boost::mpl::size<protocols_tt>::value)
			{
				throw std::invalid_argument("invalid protocol id passed");
			}
			find_encoder(protocol_id, command, size, it);
		}

	};
	template<typename protocols_tt> struct by_protocol<protocols_tt, typename boost::enable_if<typename boost::mpl::empty<protocols_tt>::type>::type>
	{
		template <typename iterator> static void inline encode(protocols::id<>::value_type protocol_id, boost::uint8_t command, std::size_t size, iterator it)
		{
			throw std::invalid_argument("invalid protocol id passed");
		}
	};

}//namespace si
