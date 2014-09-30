//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "channel_output.h"
#include "channel_input.h"
#include "io_base.h"

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <iostream>
#include <memory>

namespace si
{
	class channel_io_serial_port: public io_base::pointer, public channel_input, public channel_output
	{
	private:
        static const char DELIMITER = ',';
	public:

		typedef boost::asio::serial_port port_type;
		typedef boost::shared_ptr<channel_io_serial_port> pointer;
		typedef boost::shared_ptr<port_type> port_type_ptr;
		typedef io_base::pointer ptr_io_base;

		channel_io_serial_port(io_base::pointer const& _service = io_base::pointer())
			: ptr_io_base(_service? _service: io_base::pointer(new io_base::pointer::element_type) )
			, channel_input(*(io_base::pointer*)this)
			, channel_output(*(io_base::pointer*)this)
			, out_port()
			, in_port()
			, outbytes_transfered(0)
        {
        }
		~channel_io_serial_port()
		{}
        virtual void set_protocol(channel_protocol_interface::pointer const& protocol_)
		{
            protocol = protocol_;
			channel_input::set_protocol(protocol_);
			channel_output::set_protocol(protocol_);
        }
		virtual channel_protocol_interface::pointer get_protocol()
		{
            return protocol;
		}
        bool open_ports(std::string const& device_name)
        {
            auto delimiter_position = device_name.find(DELIMITER);

			auto current_device = device_name.substr(0, delimiter_position);

			in_port.reset(new decltype(in_port)::element_type(*(get()->service)));
			out_port.reset();

			in_port->open(current_device);
			if(! in_port->is_open())
                return false;

            if(std::string::npos != delimiter_position)
            {
                current_device = device_name.substr(++delimiter_position);

				out_port.reset(new decltype(out_port)::element_type(*(get()->service)));
                out_port->open(current_device);

                if(! out_port->is_open())
                {
                    in_port->close();
					return false;
                }
            }
            else
            {
				out_port = in_port;
			}

            return true;
        }
		bool open(std::string const& device_name)
		{
			if(!open_ports(device_name))
                return false;

			in_port->async_read_some(boost::asio::buffer(read_buffer)
													, boost::bind(&channel_io_serial_port::handle_read
																	  , this
																	  , boost::asio::placeholders::bytes_transferred
																	  , boost::asio::placeholders::error));
			return true;
		}

		void cancel()
		{
			in_port->cancel();

            if(out_port != in_port)
                out_port->cancel();

		}
		virtual void write_raw_data(std::size_t size, channel_protocol_interface::data_type data)
		{
			mutal_exclusion_type::scoped_lock sl(mtx);
			output_storage.push(output_storage_type::value_type(size, data));
			if(1 == output_storage.size())
			{
				in_port->async_write_some(boost::asio::buffer(data.get(), size)
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
				if (ec.value() == boost::asio::error::eof // linux
					 || ec.value() == boost::asio::error::operation_aborted) // win
				{
					LOG << "Serial port connection was closed." << std::endl;
					// 			   TODO: call close callback
				}
				else
				{
					LOG << "Read error: " << ec.message() << "(" << ec.value() << ")" << std::endl;
				}
				return;
			}
			in_port->async_read_some(boost::asio::buffer(read_buffer)
													, boost::bind(&channel_io_serial_port::handle_read
																	  , this
																	  , boost::asio::placeholders::bytes_transferred
																	  , boost::asio::placeholders::error));
		}
		void handle_write(std::size_t bytes_transfered, const boost::system::error_code& ec)
		{
			if(ec)
			{
				if (ec.value() == boost::asio::error::eof // linux
					 || ec.value() == boost::asio::error::operation_aborted) // win
				{
					LOG << "Serial port connection was closed." << std::endl;
					// 			   TODO: call close callback
				}
				else
				{
					LOG << "Write error: " << ec.message() << "(" << ec.value() << ")" << std::endl;
				}
				return;
			}
			mutal_exclusion_type::scoped_lock sl(mtx);
			outbytes_transfered += bytes_transfered;
			for(;(!output_storage.empty()) && (outbytes_transfered >= output_storage.front().first)
				 ; outbytes_transfered -= output_storage.front().first, output_storage.pop());
			if(output_storage.empty())
				return;

			in_port->async_write_some(boost::asio::buffer(output_storage.front().second.get(), output_storage.front().first)
											 , boost::bind(&channel_io_serial_port::handle_write
																, this
																, boost::asio::placeholders::bytes_transferred
																, boost::asio::placeholders::error));

		}

        template<typename option_tt> void set_option(option_tt option)
        {
			in_port->set_option(option);
			if(in_port != out_port)
            {
                out_port->set_option(option);
            }

        }


		typedef std::queue<std::pair<std::size_t, channel_protocol_interface::data_type> > output_storage_type;
		typedef boost::recursive_mutex mutal_exclusion_type;


        channel_protocol_interface::pointer protocol;

		port_type_ptr out_port;
		port_type_ptr in_port;

        output_storage_type output_storage;
		std::size_t outbytes_transfered;

		boost::array<boost::uint8_t, 256> read_buffer;
	};

}//namespace si
