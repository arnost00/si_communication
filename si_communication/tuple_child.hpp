#pragma once

#include <boost/tuple/tuple.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

namespace boostext
{
#define TUPLE_CHILD_BASE_INTERNAL(count, name) name

#define TUPLE_CHILD_BASE(names)\
   BOOST_PP_IF(BOOST_PP_TUPLE_ELEM(3, 2, names),\
      BOOST_PP_TUPLE_REM_CTOR,\
      TUPLE_CHILD_BASE_INTERNAL)(BOOST_PP_TUPLE_ELEM(3, 2, names), BOOST_PP_TUPLE_ELEM(3, 1, names))

#define TUPLE_CHILD_PARAM_(z, n, data)\
   BOOST_PP_COMMA_IF(n) boost::tuples::element<n, data>::type param##n

   
#define TUPLE_CHILD_CONSTRUCT_ONE(z, n, names)\
   BOOST_PP_TUPLE_ELEM(3, 0, names)( BOOST_PP_REPEAT(n, TUPLE_CHILD_PARAM_, BOOST_PP_TUPLE_ELEM(3, 1, names)))\
   :TUPLE_CHILD_BASE(names)(BOOST_PP_ENUM_PARAMS(n, param))\
   {}
#define TUPLE_CHILD_CONSTRUCTS(n, childName, tupleName)\
   BOOST_PP_REPEAT(BOOST_PP_INC(n), TUPLE_CHILD_CONSTRUCT_ONE, (childName, tupleName, 0))\
   childName(childName const & src):tupleName(src){}

#define TUPLE_CHILD_CONSTRUCTS_NO_COPY(n, childName, tupleName)\
   BOOST_PP_REPEAT(BOOST_PP_INC(n), TUPLE_CHILD_CONSTRUCT_ONE, (childName, tupleName, 0))

   
#define TUPLE_CHILD_TEMPLATE_PARAM_(z, n, names)\
   BOOST_PP_COMMA_IF(n) typename boost::tuples::element<n, TUPLE_CHILD_BASE(names) >::type param##n

#define TUPLE_CHILD_TEMPLATE_CONSTRUCT_ONE(z, n, names)\
   BOOST_PP_TUPLE_ELEM(3, 0, names)( BOOST_PP_REPEAT(n, TUPLE_CHILD_TEMPLATE_PARAM_, names))\
   :TUPLE_CHILD_BASE(names)(BOOST_PP_ENUM_PARAMS(n, param))\
   {}
#define TUPLE_CHILD_TEMPLATE_CONSTRUCTS(n, childName, tupleName)\
   BOOST_PP_REPEAT(BOOST_PP_INC(n), TUPLE_CHILD_TEMPLATE_CONSTRUCT_ONE, (childName, tupleName, 0))


#define TUPLE_CHILD_DIRECT_CONSTRUCTS(n, childName, tupleName)\
   BOOST_PP_REPEAT(BOOST_PP_INC(n), TUPLE_CHILD_CONSTRUCT_ONE, (childName, tupleName, n))

#define TUPLE_CHILD_DIRECT_TEMPLATE_CONSTRUCTS(n, childName, tupleName)\
   BOOST_PP_REPEAT(BOOST_PP_INC(n), TUPLE_CHILD_TEMPLATE_CONSTRUCT_ONE, (childName, tupleName, n))

}//namespace boostext

