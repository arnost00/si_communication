#pragma once

#include "si_constants.h"
#include "command_parameter.h"
#include "fixed_command.h"

#include <boost/config/suffix.hpp>

namespace si
{
   namespace structures
   {
      struct control_number: public unsigned_integral_parameter<2, control_number>{};
      struct time_12h: public unsigned_integral_parameter<2, time_12h>{};
      struct am_pm: public unsigned_integral_parameter<1, am_pm>{};
      struct day_of_week: public unsigned_integral_parameter<1, day_of_week>{};
      struct week_counter: public unsigned_integral_parameter<1, week_counter>{};
      struct card5_id_part: public unsigned_integral_parameter<1, card5_id_part>{};
      struct card5_serie_part: public unsigned_integral_parameter<2, card5_serie_part>{};


      typedef boost::mpl::deque<control_number::bits_range<7,0>, time_12h::bits_range<15,0> > punch_3bytes_def;
      typedef boost::mpl::deque<control_number::bits_range<9,8>
         , day_of_week::bits_range<1,0>
         , week_counter::bits_range<2,0>
         , am_pm::bits_range<0>
         , control_number::bits_range<7,0>
         , time_12h::bits_range<15,0> > punch_4bytes_def;
      typedef boost::mpl::deque<card5_id_part, card5_serie_part> card5_id_def;

      struct punch_3bytes_type: public bit_array<punch_3bytes_def, punch_3bytes_type>{};
      struct punch_4bytes_type: public bit_array<punch_4bytes_def, punch_4bytes_type>{};

      struct card5_id_type: public bit_array<card5_id_def, card5_id_type>{};

   }//namespace structures

}//namespace si
