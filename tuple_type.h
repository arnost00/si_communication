#pragma once

#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/greater.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>

namespace boostext
{
   template<typename sequence_tt> struct tuple_type
   {
	  typedef typename boost::mpl::size<sequence_tt>::value_type size_type;
      BOOST_STATIC_CONSTANT(size_type, max_size = 10);
      BOOST_MPL_ASSERT_MSG(boost::mpl::size<sequence_tt>::value <= max_size, SEQUENCE_TOO_LARGE_FOR_TUPLE_DEFINITION, (types<sequence_tt>, typename boost::mpl::integral_c<unsigned, max_size>));


	  template<typename position, typename enable = typename boost::mpl::greater<typename boost::mpl::size<sequence_tt>::type, position >::type > struct position_type
	  {
//		 BOOST_MPL_ASSERT_MSG(false, TUPLE_TYPE_OUT_OF_INPUT, (types< position, enable, sequence_tt >));
		 typedef boost::tuples::null_type type;
	  };

	  template<typename position> struct position_type<position, typename boost::mpl::bool_<true> >
      {
//		 BOOST_MPL_ASSERT_MSG(false, TUPLE_TYPE_OUT_OF_INPUT, (types<position>));
			typedef typename boost::mpl::at<sequence_tt, position>::type type;
      };

      typedef typename boost::tuple<
		 typename position_type<boost::mpl::integral_c<size_type, 0> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 1> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 2> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 3> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 4> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 5> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 6> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 7> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 8> >::type
		 , typename position_type<boost::mpl::integral_c<size_type, 9> >::type
      > type;
   };
}//namespace boostext
