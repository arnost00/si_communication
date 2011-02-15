//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_output.h"
#include "channel_input.h"
#include "io_base.h"

#include <boost/array.hpp>
#include <queue>
#include <iostream>

namespace si
{
	class channel_io_serial_port
		: public channel_output
		, public io_base<boost::mpl::deque<channel_input, boost::asio::serial_port> >
   {
	private:
        typedef io_base<boost::mpl::deque<channel_input, boost::asio::serial_port> > io_base_type;
   public:
		channel_io_serial_port()
			: outbytes_transfered(0)
		{}
		~channel_io_serial_port()
		{
		}
		typedef boost::shared_ptr<channel_io_serial_port> pointer;
      virtual void set_protocol(channel_protocol_interface::pointer protocol_)
      {
         channel_output::set_protocol(protocol_);
      }
      virtual channel_protocol_interface::pointer get_protocol()
      {
         return channel_output::get_protocol();
      }
		bool open(std::string const& device_name)
		{
            io_base_type::open(device_name);
/*			io_base_type::set_option(boost::asio::serial_port_base::baud_rate(38400));
            io_base_type::set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
            io_base_type::set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
            io_base_type::set_option(boost::asio::serial_port_base::character_size(8));
            io_base_type::set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
*/
            if(!io_base_type::is_open())
				return false;

            io_base_type::async_read_some(boost::asio::buffer(read_buffer)
				, boost::bind(&channel_io_serial_port::handle_read
					, this
					, boost::asio::placeholders::bytes_transferred
					, boost::asio::placeholders::error));
			return true;
		}
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			output_storage.push(output_storage_type::value_type(size, data));
			if(1 == output_storage.size())
			{
				boost::asio::async_write(*this
					, boost::asio::buffer(data.get(), size)
					, boost::bind(&channel_io_serial_port::handle_write
						, this
						, boost::asio::placeholders::bytes_transferred
						, boost::asio::placeholders::error));
			}
		}
		void handle_read(std::size_t bytes_transfered, const boost::system::error_code& ec)
		{
			if(0 < bytes_transfered)
			{
            try
            {
				   process_input(bytes_transfered, read_buffer.data());
            }
            catch (std::invalid_argument &e)
            {
               LOG << "Input processing failed: " << e.what() << std::endl;
            }
			}
			if(ec)
			{
				std::cerr << ec.message();
				return;
			}
                io_base_type::async_read_some(boost::asio::buffer(read_buffer)
				, boost::bind(&channel_io_serial_port::handle_read
					, this
					, boost::asio::placeholders::bytes_transferred
					, boost::asio::placeholders::error));
		}
		void handle_write(std::size_t bytes_transfered, const boost::system::error_code& ec)
		{
			if(ec)
			{
				std::cerr << ec.message();
				return;
			}
			mutal_exclusion_type::scoped_lock sl(mtx);
			outbytes_transfered += bytes_transfered;
			for(;(!output_storage.empty()) && (outbytes_transfered >= output_storage.front().first)
				; outbytes_transfered -= output_storage.front().first, output_storage.pop());
			if(output_storage.empty())
				return;
			if(ec)
			{
				std::cerr << ec.message();
				return;
			}
			boost::asio::async_write(*this
				, boost::asio::buffer(output_storage.front().second.get(), output_storage.front().first)
				, boost::bind(&channel_io_serial_port::handle_write
					, this
					, boost::asio::placeholders::bytes_transferred
					, boost::asio::placeholders::error));

		}
		typedef std::queue<std::pair<std::size_t, channel_protocol_interface::data_type> > output_storage_type;
		typedef boost::recursive_mutex mutal_exclusion_type;
		output_storage_type output_storage;
		std::size_t outbytes_transfered;
		boost::array<si::byte, 256> read_buffer;
   };

}//namespace si
