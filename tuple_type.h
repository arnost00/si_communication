#pragma once

#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>

namespace boostext
{
   template<typename sequence_tt> struct tuple_type
   {
      BOOST_STATIC_CONSTANT(unsigned, max_size = 10);
      BOOST_MPL_ASSERT_MSG(boost::mpl::size<sequence_tt>::value <= max_size, SEQUENCE_TOO_LARGE_FOR_TUPLE_DEFINITION, (types<sequence_tt>, typename boost::mpl::integral_c<unsigned, max_size>));

      template<unsigned position, bool enable = (position < boost::mpl::size<sequence_tt>::value) > struct position_type
      {
            typedef typename boost::mpl::at_c<sequence_tt, position>::type type;
      };

      template<unsigned position> struct position_type<position, false>
      {
         typedef boost::tuples::null_type type;
      };

      typedef typename boost::tuple<
         typename position_type<0>::type
         , typename position_type<1>::type
         , typename position_type<2>::type
         , typename position_type<3>::type
         , typename position_type<4>::type
         , typename position_type<5>::type
         , typename position_type<6>::type
         , typename position_type<7>::type
         , typename position_type<8>::type
         , typename position_type<9>::type
      > type;
   };
}//namespace boostext
