//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "si_constants.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/shared_array.hpp>

#include <cstddef>

namespace si
{
   struct command_interface
   {
      typedef boost::shared_ptr<command_interface> pointer;
      typedef boost::function<void(const byte*,const std::size_t)> writer_type;
      typedef byte id_type;
      typedef boost::shared_array<byte> data_type;

      virtual id_type get_id(protocols::id<>::value_type protocol = protocols::id<>::value) = 0;
      virtual void get_data(std::size_t &size, data_type& data, protocols::id<>::value_type protocol = protocols::id<>::value) = 0;
//      virtual void write(writer_type writer, protocols::id<>::value_type protocol) = 0;
      virtual std::size_t get_size(protocols::id<>::value_type protocol = protocols::id<>::value) = 0;
      virtual bool is_control_sequence() = 0;
//      virtual bool can_accept_data(std::size_t size, data_type data) = 0;
      virtual bool accept_data(std::size_t size, data_type data) = 0;
      virtual ~command_interface(){}

   };
}//namespace si
