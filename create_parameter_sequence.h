//  (C) Copyright 2009-2011 Vit Kasal
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

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
	template<typename parameters_sequence
				, typename parameter
				, typename parameter_found = typename boost::is_same<typename boost::mpl::find<parameters_sequence, typename parameter::parameter_type>::type, typename boost::mpl::end<parameters_sequence>::type>::type>
	struct add_new_parameter
	{
		typedef typename boost::mpl::push_back<parameters_sequence, typename parameter::parameter_type>::type type;
	};
	template<typename parameters_sequence
				, typename parameter>
	struct add_new_parameter<parameters_sequence, parameter, boost::false_type>
	{
		typedef parameters_sequence type;
	};

	template<typename parameters_sequence, typename result_sequence, typename is_parameter = typename boost::is_base_of<si::parameter, typename boost::mpl::front<parameters_sequence>::type>::type>
	struct process_item
	{
		typedef typename add_new_parameter<result_sequence, typename boost::mpl::front<parameters_sequence>::type >::type type;
	};
	template<typename parameters_sequence, typename result_sequence>
	struct process_item<parameters_sequence, result_sequence, boost::false_type>
	{
		typedef result_sequence type;
	};
	template<typename parameters_sequence, typename result_sequence = boost::mpl::deque<>, bool empty_input = boost::mpl::empty<parameters_sequence>::value > struct create_parameter_sequence
	{
		/*      typedef typename boost::mpl::if_<typename boost::is_base_of<si::parameter, typename boost::mpl::front<parameters_sequence>::type>::type
			, typename add_new_parameter<result_sequence, typename boost::mpl::front<parameters_sequence>::type >::type
			, result_sequence>::type*/
		typedef typename process_item<parameters_sequence, result_sequence>::type processed_result_sequence;
		typedef typename create_parameter_sequence<typename boost::mpl::pop_front<parameters_sequence>::type, processed_result_sequence>::type type;
		//      typedef boost::mpl::deque<> type;
	};

	template <typename parameters_sequence, typename result_sequence> struct create_parameter_sequence<parameters_sequence, result_sequence, true>
	{
		typedef result_sequence type;
	};
}//namespace si
