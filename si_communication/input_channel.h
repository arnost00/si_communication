//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace si
{
   class input_channel
   {
      void process_input(byte* data, std::size_t data_size)
      {
         input.insert(input.end(), data, data + data_size);
      }

/*      std::list<protocol_processor_interface::pointer> protocol_processors_container_type;
      protocol_processors_container_type processors;*/
      std::vector<byte> input_container_type;
      input_container_type input;
   };
}//namespace si
