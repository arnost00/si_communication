#pragma once

//#include "command_parameter.h"
#include "si_auxilary.h"

#include <boost/mpl/deque.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/end.hpp>

namespace si
{
   template<typename parameters_sequence, typename parameter> struct add_new_parameter
   {
      typedef typename boost::mpl::if_<typename boost::is_same<typename boost::mpl::find<parameters_sequence, typename parameter::parameter_type>::type, typename boost::mpl::end<parameters_sequence>::type>::type
         , typename boost::mpl::push_back<parameters_sequence, typename parameter::parameter_type>::type
                  , parameters_sequence>::type type;
   };
   template<typename parameters_sequence, typename result_sequence = boost::mpl::deque<>, bool empty = boost::mpl::empty<parameters_sequence>::value> struct create_parameter_sequence
   {
      typedef typename boost::mpl::if_<typename boost::is_base_of<si::parameter, typename boost::mpl::front<parameters_sequence>::type>::type
         , typename add_new_parameter<result_sequence, typename boost::mpl::front<parameters_sequence>::type >
         , result_sequence>::type::type processed_result_sequence;
      typedef typename create_parameter_sequence<typename boost::mpl::pop_front<parameters_sequence>::type, processed_result_sequence>::type type;
   };

   template <typename parameters_sequence, typename result_sequence> struct create_parameter_sequence<parameters_sequence, result_sequence, true>
   {
      typedef result_sequence type;
   };
}//namespace si
