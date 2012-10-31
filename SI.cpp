//  (C) Copyright 2009-2012 Vit Kasal,
//  (C) Copyright 2011 Richard Patek
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// SI.cpp : Defines the entry point for the console application.
//

//under mingw, boost_thread tends to link with dynamic library instead of static.
//#define BOOST_THREAD_USE_LIB in mingw project to correct

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/mpl/list_c.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <signal.h>
#include <boost/tuple/tuple.hpp>

#include <boost/system/windows_error.hpp>
#include <boost/system/linux_error.hpp>


#ifdef _WIN32
#include <tchar.h>
#endif


#include "program_info.h"
#include "filelog.h"

#include "startup_sequence.h"
#include "si_static_command.h"
#include "channel_std_out.h"
#include "channel_loopback.h"
#include "channel_io_serial_port.h"
#include "command_parameter.h"
#include "command_parameter2.h"
#include "create_parameter_sequence.h"
#include "response.h"
#include "chip_readout.h"
#include "tuple_type.h"
#include "crc529.h"
#include "fixed_command.h"

struct read_si_after_punch: public si::unsigned_integral_parameter<1, read_si_after_punch>{};
struct access_with_password_only: public si::unsigned_integral_parameter<1, access_with_password_only>{};
struct handshake: public si::unsigned_integral_parameter<1, handshake>{};
struct auto_send_out: public si::unsigned_integral_parameter<1, auto_send_out>{};
struct extended_protocol: public si::unsigned_integral_parameter<1, extended_protocol>{};

struct cpc: public si::bit_array<boost::mpl::deque<read_si_after_punch::bits_range<0>, si::don_t_care<2>, access_with_password_only::bits_range<0>, si::don_t_care<>, handshake::bits_range<0>, auto_send_out::bits_range<0>, extended_protocol::bits_range<0> >, cpc>{};

template<typename command_type>void send_to_channel(typename command_type::pointer command, si::channel_output &channel)
{
	channel.write_command(command);
}

typedef boost::shared_ptr<std::fstream> ofstream_pointer;

typedef std::list<si::channel_io_serial_port::pointer> ports_container_type;

template<class TOStream> void out_time_duration(TOStream &fs, boost::posix_time::time_duration const& duration, const bool enable_ms = false)
{
	if(duration.is_not_a_date_time())
	{
		fs << "No time ";
		return;
	}
	fs << std::setfill('0') << std::dec;
	fs << std::setw(2) << duration.hours();
	fs << ':' << std::setw(2) << duration.minutes();
	fs << ':' << std::setw(2) << duration.seconds();

	if(enable_ms)
	{
		boost::posix_time::time_duration::fractional_seconds_type ms = duration.total_milliseconds() - (duration.total_seconds() * 1000);

		fs << '.' << std::setw(3) << ms;
	}
}
void punch_arrived(si::extended::responses::transmit_record::pointer record_msg)
{
	LOG << "record arrived: " << record_msg->get<si::extended::si>().value << " ";
	out_time_duration(LOG, boost::posix_time::millisec(1000*(record_msg->get<si::extended::t>().value) + (record_msg->get<si::extended::tss>().value*1000/256)), true);
	LOG << std::endl;
}
void punch_arrived(si::basic::responses::transmit_record::pointer record_msg)
{
	LOG << "record arrived: " << record_msg->get<si::basic::sisn>().value + 0x10000 * record_msg->get<si::basic::sis>().value << " serie: " << (double)(record_msg->get<si::basic::sis>().value) << " ";
	out_time_duration(LOG, boost::posix_time::millisec(1000*(record_msg->get<si::basic::t>().value) + (record_msg->get<si::basic::tss>().value*1000/256)), true);
	LOG << std::endl;
}
void punch_read(ofstream_pointer of, si::extended::responses::transmit_record::pointer record)
{
	std::ostream &fs = *of.get();

	fs << std::setfill(' ') << std::setw(8) << record->get<si::extended::si>().value;
	fs << ':';
	fs << std::setw(4) << record->get<si::extended::cn>().value << "/";
	out_time_duration(fs, boost::posix_time::millisec(1000*(record->get<si::extended::t>().value)
																	  + (record->get<si::extended::tss>().value*1000/256)), true);
	fs << std::endl << std::flush;

	punch_arrived(record);

}
void punch_read_basic(ofstream_pointer of, si::basic::responses::transmit_record::pointer record)
{
	std::ostream &fs = *of.get();

	fs << std::setfill(' ') << std::setw(8) << (record->get<si::basic::sisn>().value + 0x10000 * record->get<si::basic::sis>().value);
	fs << ':';
	fs << std::setw(4) << (unsigned)(record->get<si::basic::cn>().value) << "/";
	out_time_duration(fs, boost::posix_time::millisec(1000*(record->get<si::basic::t>().value)
																	  + (record->get<si::basic::tss>().value*1000/256)), true);
	fs << std::endl << std::flush;

	punch_arrived(record);

}
void chip_read(ofstream_pointer of, si::card_record::pointer record)
{
	typedef boost::tuples::element<si::card_record::PUNCH_RECORDS, si::card_record>::type punches_container;
	std::size_t punches_count;
	std::ostream &fs = *of.get();

	fs << std::setfill(' ') << std::setw(8) << record->get<si::card_record::CARD_ID>();
	fs << ':' << ' ';
	fs << "C/";
	out_time_duration(fs, record->get<si::card_record::CHECK_TIME>());
	fs << ' ';
	fs << "S/";
	out_time_duration(fs, record->get<si::card_record::START_TIME>(), record->get<si::card_record::START_SUBSECOND>());
	fs << ' ';
	fs << "F/";
	out_time_duration(fs, record->get<si::card_record::FINISH_TIME>(), record->get<si::card_record::FINISH_SUBSECOND>());
	fs << ' ';

	punches_container &punches = record->get<si::card_record::PUNCH_RECORDS>();
	punches_count = punches.size();

	fs << std::setfill(' ') << std::setw(3) << punches_count;
	for(unsigned i = 0; i < punches_count; i++)
	{
		fs << std::setw(0) << ' ';
		fs << std::setfill(' ') << std::setw(3) << punches[i].control_number;
		fs << std::setw(0) << '/';
		out_time_duration(fs, punches[i].punch_time);
	}
	fs << std::endl << std::flush;
}

bool exit_predicate = false;
boost::condition exit_cond;
boost::recursive_mutex exit_mtx;
inline bool should_exit()
{
	return exit_predicate;
}
void notify_exit_condition(int )
{
	boost::recursive_mutex::scoped_lock sl(exit_mtx);
	exit_predicate = true;
	exit_cond.notify_all();
}
void startup_sequence_failed()
{
	throw std::runtime_error("Startup sequence failed.");
}
void log_device_param(std::string const & devname)
{
	LOG << "\t" << devname << std::endl;
}

si::channel_io_serial_port::pointer open_device(std::string const& device_name
															, si::io_base::pointer& io_base
															, ports_container_type *io_ports = NULL)
{

	si::channel_io_serial_port::pointer si_port(new si::channel_io_serial_port::pointer::element_type(io_base->service));
	try
	{
		si_port->open(device_name);
	}
	catch(boost::system::system_error &e)
	{
		switch (e.code().value())
		{
#ifdef BOOST_WINDOWS_API
		case boost::system::windows_error::file_not_found:
			LOG << "Communication device not found." << std::endl;
			break;
		case boost::system::windows_error::access_denied:
			LOG << "Communication device access denied (maybe open by another application)." << std::endl;
			break;
#else // linux
		case boost::system::errc::no_such_file_or_directory:
			LOG << "Communication device not found." << std::endl;
			break;
#endif
		case boost::asio::error::already_open:
			LOG << "Communication device already open (maybe by another application)." << std::endl;
			break;
		default:
			LOG << "Boost error, while opening input device : " << e.what() << ". Code : " << e.code().value()<< std::endl;
		}
		si_port.reset();
		return si_port;
	}
	if (NULL != io_ports)
		io_ports->insert(io_ports->end(), si_port);

	return si_port;
}
void start_startup_sequence(std::string const& device_name
									 , si::io_base::pointer &io_base
									 , si::chip_readout::callback_chip_read &chip_read_cb
									 , ports_container_type *io_ports)
{
	LOG << "Opening device: " << (io_base->service? "service provided " : "no service ") << device_name << std::endl;

	si::channel_io_serial_port::pointer si_port = open_device(device_name, io_base, io_ports);
	if(!si_port)
	{
		//ToDo: Handle device open failure
		LOG << "Failed to open device: " << device_name;
		return;
	}
	si::chip_readout::pointer chip_readout(new si::chip_readout);
	si::startup_sequence::pointer startup_sequence(new si::startup_sequence);


	startup_sequence->start(si_port
									, boost::bind(&si::chip_readout::start, chip_readout, si_port
													  , si::chip_readout::callback_type()
													  , si::chip_readout::callback_type()
													  , chip_read_cb)
									, boost::bind(&startup_sequence_failed));

}

void start_punch_wait(std::string const& device_name
							 , si::io_base::pointer &io_base
							 , ofstream_pointer &output_file
							 , ports_container_type *io_ports)
{
	LOG << "Opening device: " << (io_base->service? "service provided " : "no service ") << device_name << std::endl;
	si::channel_io_serial_port::pointer si_port = open_device(device_name, io_base, io_ports);
	if(!si_port)
	{
		//ToDo: Handle device open failure
		return;
	}
	std::cout << "Opening device: done" << (io_base->service? "service provided " : "no service ") << std::endl;


	si_port->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<si::protocols::extended>()));
	si::response_interface::pointer read_responses = si::response<>::create(si::response<
																									boost::mpl::deque<si::extended::responses::transmit_record, si::basic::responses::transmit_record>
																									, si::response_live_control::permanent>::reactions_type
																									(boost::bind(&punch_read, output_file, _1), boost::bind(&punch_read_basic, output_file, _1)));
	si_port->register_response_expectation(read_responses);
}

filelog multilog;

//#ifdef _WIN32

//#include <tchar.h>

//#define _CMD_ARG _TEXT

//int _tmain(int argc, _TCHAR* argv[])

//#else

#define _CMD_ARG(text) text

int main(int argc, char* argv[])

//#endif
{
	std::cout << program_name << " " << program_version << " - " << program_description << std::endl;
	std::cout << "Copyright (C) 2009-2012 Vit Kasal, Richard Patek" << std::endl;
	std::cout << "EOBSystem - system.eob.cz" << std::endl;
	boost::program_options::options_description desc("Allowed options");
	int prog_type;
	desc.add_options()
			("help,h", "produce help message with allowed options and return values")
			("device,d", boost::program_options::value<std::vector<std::string> >(), "set communication device (ex. com2 or /dev/sportident/reader0)")
			("output,o", boost::program_options::value<std::string>(), "set output file (ex. card.txt)")
			("type,t", boost::program_options::value<int>(&prog_type)->default_value(0), "set program mode (0 - card readout, 1 - station punch)")
			("logfile,l", boost::program_options::value<std::string>(), "set log file (ex. logfile.txt)");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (argc < 2 || vm.count("help"))
	{
		std::cout << desc << std::endl;
		if (vm.count("help"))
		{
			std::cout << "Possible application return values:" << std::endl;
			std::cout << "  " << 0 << " - normal application end" << std::endl;
			std::cout << "  " << 1 << " - help or no options set" << std::endl;
			std::cout << "  " << 2 << " - communication device not set" << std::endl;
			std::cout << "  " << 3 << " - output file not set" << std::endl;
			std::cout << "  " << 4 << " - failed to open output file" << std::endl;
			std::cout << "  " << 5 << " - unhandled boost error" << std::endl;
			std::cout << "  " << 6 << " - unhandled unknown error" << std::endl;
			std::cout << "  " << 7 << " - failed to open communication device" << std::endl;
			std::cout << "  " << 8 << " - unhandled std error" << std::endl;
		}
		return 1;
	}
	if (vm.count(_CMD_ARG("logfile")))
	{
		if (!multilog.open(vm[_CMD_ARG("logfile")].as<std::string>()))
		{
			std::cout << "Cannot open log file." << std::endl;
			return 8;
		}
	}

	if (0 == vm.count(_CMD_ARG("device")))
	{
		LOG << "Communication device was not set."<< std::endl;
		return 2;
	}

	ofstream_pointer output_file;
	if (vm.count("output"))
	{
		LOG << "Output was set to " << vm[_CMD_ARG("output")].as<std::string>() << std::endl;
		output_file.reset(new std::fstream());
		output_file->open(vm[_CMD_ARG("output")].as<std::string>().c_str(), std::ios_base::app| std::ios_base::out);
		if(!output_file->is_open())
		{
			output_file->open(vm[_CMD_ARG("output")].as<std::string>().c_str(), std::ios_base::trunc| std::ios_base::out);
			if(!output_file->is_open())
			{
				LOG << "Failed to open output file" << std::endl;
				output_file.reset();
				return 4;
			}
		}
	}
	else
	{
		LOG << "Output was not set."<< std::endl;
		return 3;
	}
	si::chip_readout::callback_chip_read chip_read_cb;
	si::startup_sequence startup_sequence;
	si::io_base::pointer io_base_instance;

	ports_container_type io_ports;

	try
	{
		io_base_instance.reset(new si::io_base);
		if(output_file)
			chip_read_cb = boost::bind(&chip_read, output_file, _1);

		LOG << "Program set to ";
		if (prog_type == 0)
		{
			LOG << "card readout mode." << std::endl;
			std::for_each(vm[_CMD_ARG("device")].as<std::vector<std::string> >().begin()
							  , vm[_CMD_ARG("device")].as<std::vector<std::string> >().end()
							  , boost::bind(&start_startup_sequence, _1, boost::ref(io_base_instance)
												 , boost::ref(chip_read_cb), &io_ports));
		}
		else
		{
			LOG << "station punch mode." << std::endl;
			std::for_each(vm[_CMD_ARG("device")].as<std::vector<std::string> >().begin()
							  , vm[_CMD_ARG("device")].as<std::vector<std::string> >().end()
							  , boost::bind(&start_punch_wait
													, _1, boost::ref(io_base_instance)
													, output_file, &io_ports));
		}
	}
	catch(boost::system::system_error &e)
	{
		LOG << "Boost error: " << e.what() << std::endl;
		return 5;
	}
	catch(std::exception &e)
	{
		LOG << "Std error: " << e.what() << std::endl;
		return 8;
	}
	catch(...)
	{
		LOG << "Unknown error." << std::endl;
		return 6;
	}

	signal(SIGINT, &notify_exit_condition);
	signal(SIGTERM, &notify_exit_condition);
#ifdef _WIN32
	signal(SIGBREAK, &notify_exit_condition);
#else
	signal(SIGQUIT, &notify_exit_condition);
#endif
	signal(SIGABRT, &notify_exit_condition);

	exit_cond.wait(exit_mtx, boost::bind(&should_exit));

	LOG << "exitting" << std::endl;
	//   siport->close();

	ports_container_type::iterator it = io_ports.begin();
	ports_container_type::iterator endit = io_ports.end();
	boost::system::error_code ec;
	for(;it != endit; it++)
	{
/*		std::cout << "canceling device, count: " << (*it).use_count() << std::endl;
		(*it)->cancel();
		if(ec)
		{
			std::cout << ec.message() << std::endl;
		}

		std::cout << "canceling device, count: " << (*it).use_count() << std::endl;
*/
		it->reset();
	}
	return 0;
}
