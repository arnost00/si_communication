// SI.cpp : Defines the entry point for the console application.
//


//#define _GLIBCXX_USE_WCHAR_T

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <tchar.h>

#include "startup_sequence.h"
#include "si_static_command.h"
#include "channel_std_out.h"
#include "channel_loopback.h"
#include "channel_io_serial_port.h"
#include "command_parameter.h"
#include "create_parameter_sequence.h"
#include "response.h"
#include "chip_readout.h"
#include <boost/mpl/list_c.hpp>
#include <iostream>
#include <fstream>
#include <string>


#include <boost/tuple/tuple.hpp>
#include "tuple_type.h"
#include "crc529.h"
#include "fixed_command.h"

void outchar(unsigned char ch)
{
   std::cout << std::hex << (int)ch << ',';
}

/*
struct read_si_after_punch: public si::parameter_t<read_si_after_punch>{};
struct access_with_password_only: public si::parameter_t<access_with_password_only>{};
struct handshake: public si::parameter_t<handshake>{};
struct auto_send_out: public si::parameter_t<auto_send_out>{};
struct extended_protocol: public si::parameter_t<extended_protocol>{};
*/
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

void out_time_duration(std::ostream &fs, boost::posix_time::time_duration const& duration, const bool enable_ms = false)
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
		unsigned ms = duration.total_milliseconds() - (duration.total_seconds() * 1000);

		fs << '.' << std::setw(3) << ms;
	}
}
void punch_arrived(si::extended::responses::transmit_record::pointer record_msg)
{
	std::cout << "record arrived: " << record_msg->get<si::extended::si>().value << " ";
	out_time_duration(std::cout, boost::posix_time::millisec(1000*(record_msg->get<si::extended::t>().value) + (record_msg->get<si::extended::tss>().value*1000/256)), true);
	std::cout << std::endl;

}
void punch_read(ofstream_pointer of, si::extended::responses::transmit_record::pointer record)
{
	std::ostream &fs = *of.get();

	fs << std::setfill(' ') << std::setw(8) << record->get<si::extended::si>().value;
	fs << ':' << ' ';
	out_time_duration(fs, boost::posix_time::millisec(1000*(record->get<si::extended::t>().value) 
		+ (record->get<si::extended::tss>().value*1000/256)), true);
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
	out_time_duration(fs, record->get<si::card_record::START_TIME>());
	fs << ' ';
	fs << "F/";
	out_time_duration(fs, record->get<si::card_record::FINISH_TIME>());
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

int _tmain(int argc, _TCHAR* argv[])
{
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("device", boost::program_options::value<std::string>(), "set communication device")
		("output", boost::program_options::value<std::string>(), "set output file");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);    

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		 return 1;
	}

	if (vm.count("device"))
	{
		 std::cout << "Device was set to " << vm["device"].as<std::string>() << std::endl;
	}
	else
	{
		 std::cout << "Device was not set.\n";
	}

	ofstream_pointer output_file;
	if (vm.count("output"))
	{
		std::cout << "Output was set to " << vm["output"].as<std::string>() << std::endl;
		output_file.reset(new std::fstream());
		output_file->open(vm["output"].as<std::string>().c_str(), std::ios_base::app| std::ios_base::out);
		if(!output_file->is_open())
		{
			output_file->open(vm["output"].as<std::string>().c_str(), std::ios_base::trunc| std::ios_base::out);
			if(!output_file->is_open())
			{
				std::cout << "failed to open output file" << std::endl;
				output_file.reset();
			}
		}
	}
	else
	{
		 std::cout << "Output was not set.\n";
	}
   si::channel_io_serial_port::pointer siport(new si::channel_io_serial_port::pointer::element_type());
   si::chip_readout::callback_chip_read chip_read_cb;
   si::startup_sequence startup_sequence;
   si::chip_readout chip_readout;
   try
   {
		siport->open(vm["device"].as<std::string>());
		std::cout << (siport->is_open()? "port opened": "port closed") << std::endl;

		if(output_file)
			chip_read_cb = boost::bind(&chip_read, output_file, _1);
	   startup_sequence.start(siport
		   , boost::bind(&si::chip_readout::start, &chip_readout, siport
		   , si::control_sequence_base<>::callback_type()
		   , si::control_sequence_base<>::callback_type()
		   , chip_read_cb));
/*///----------------------
		siport->set_protocol(si::channel_protocol_interface::pointer(new si::channel_protocol<si::protocols::extended>()));

		si::response_interface::pointer read_responses = si::response<>::create(si::response<
			boost::mpl::deque<si::extended::responses::transmit_record>
			, si::response_live_control::permanent>::reactions_type
			(boost::bind(&punch_read, output_file, _1)));

		siport->register_response_expectation(read_responses);
//-----------------------*/
      
   }
   catch(...)
   {
   }
	BOOL bRet;
	MSG msg;

	while( (bRet = GetMessage( &msg, 0, 0, 0 )) != 0)
	{ 
		 if (bRet == -1)
		 {
			 break;
		 }
		 else
		 {
			  TranslateMessage(&msg); 
			  DispatchMessage(&msg); 
		 }
	}

	return 0;
}

