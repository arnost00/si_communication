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
