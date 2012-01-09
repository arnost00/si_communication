//  (C) Copyright 2011  Richard Patek
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>

class filelog
{
public:
	std::ofstream ofs;
	bool isopen;
	filelog():
		isopen(false)
	{}

	~filelog(void)
	{
		if (isopen)
		{
			ofs << "=== " << program_name << " " << program_version << " - log stop - " << get_timestamp() << " ===" << std::endl;
		}
		if (isopen)
			ofs.close();
	}

	bool open(const std::string filename)
	{
		ofs.open(filename.c_str(), std::ios::app );
		isopen = ofs.good();
		if (isopen)
		{
			ofs << "=== " << program_name << " " << program_version << " - log init - " << get_timestamp() << " ===" << std::endl;
		}
		return isopen;
	}

	filelog& operator<< (std::ostream& (*pfun)(std::ostream&))
	{
		if (isopen)
			pfun(ofs);
		pfun(std::cout);
		return *this;
	}

	std::string get_timestamp()
	{
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		return boost::posix_time::to_simple_string(now);
	}
};

template <class T>
inline filelog& operator<< (filelog& s, T v)
{
	if (s.isopen)
		s.ofs << v;
	std::cout << v;
	return s;
};

extern filelog multilog;

#define LOG multilog
