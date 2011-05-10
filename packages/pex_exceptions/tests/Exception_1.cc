/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
 
#include "lsst/pex/exceptions/Exception.h"

#define BOOST_TEST_MODULE Exception_1
#include "boost/test/included/unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

#include "boost/format.hpp"

namespace test = boost::test_tools;
namespace pexExcept = lsst::pex::exceptions;

#include "Exception_1.h"

// These functions were created to avoid having the output strings, which
// include function names, depend on the boost::test implementation.  Note
// that the output strings also depend on the name of this file and line
// numbers within it, however.

void f1(void) {
    throw LSST_EXCEPT(pexExcept::Exception, "In f1");
}

void f2(void) {
    throw LSST_EXCEPT(ChildException, (boost::format("In f2 %1%") % 2008).str());
}

void f3(int i) {
    throw LSST_EXCEPT(DetailedException, (boost::format("In f3 %1%") % "foo").str(), i);
}

void f4(void) {
    try {
        f1();
    }
    catch (pexExcept::Exception& e) {
        LSST_EXCEPT_ADD(e, "In f4");
        throw;
    }
}

void f5(void) {
    try {
        f2();
    }
    catch (pexExcept::Exception& e) {
        LSST_EXCEPT_ADD(e, "In f5");
        throw;
    }
}

void f6(void) {
    try {
        f2();
    }
    catch (ChildException& e) {
        LSST_EXCEPT_ADD(e, "In f6");
        throw;
    }
}

void f7(int i) {
    try {
        f3(i);
    }
    catch (pexExcept::Exception& e) {
        LSST_EXCEPT_ADD(e, (boost::format("In f7 (%1% %2% %3%)") % i % 42 % 3.14159).str());
        throw;
    }
}

void f8(int i) {
    try {
        f3(i);
    }
    catch (DetailedException& e) {
        LSST_EXCEPT_ADD(e, "In f8");
        throw;
    }
}

void f9(int i) {
    try {
        f3(i);
    }
    catch (ChildException& e) {
        // Should not be executed
        LSST_EXCEPT_ADD(e, "In f9");
        throw;
    }
}

BOOST_AUTO_TEST_SUITE(ExceptionSuite)

BOOST_AUTO_TEST_CASE(base) {
    BOOST_CHECK_THROW(f1(), pexExcept::Exception);
    BOOST_CHECK_THROW(f2(), ChildException);
    BOOST_CHECK_THROW(f3(1), DetailedException);
    BOOST_CHECK_THROW(f4(), pexExcept::Exception);
    BOOST_CHECK_THROW(f5(), ChildException);
    BOOST_CHECK_THROW(f6(), ChildException);
    BOOST_CHECK_THROW(f7(2), DetailedException);
    BOOST_CHECK_THROW(f8(3), DetailedException);
    BOOST_CHECK_THROW(f9(4), DetailedException);
}

BOOST_AUTO_TEST_CASE(simple) {
    test::output_test_stream o;
    try {
        f1();
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: lsst::pex::exceptions::Exception thrown at tests/Exception_1.cc:42 in void f1()\n"
            "0: Message: In f1\n"));
}

BOOST_AUTO_TEST_CASE(child_as_base) {
    test::output_test_stream o;
    try {
        f2();
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"));
}

BOOST_AUTO_TEST_CASE(child_as_child) {
    test::output_test_stream o;
    try {
        f2();
    }
    catch (ChildException const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"));
}

BOOST_AUTO_TEST_CASE(detailed_as_base) {
    test::output_test_stream o;
    try {
        f3(5);
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(5)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(detailed_as_detailed) {
    test::output_test_stream o;
    try {
        f3(6);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), 6);
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(6)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(simple_rethrow) {
    test::output_test_stream o;
    try {
        f4();
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: lsst::pex::exceptions::Exception thrown at tests/Exception_1.cc:42 in void f1()\n"
            "0: Message: In f1\n"
            "1: Rethrown at tests/Exception_1.cc:58 in void f4()\n"
            "1: Message: In f4\n"));
}

BOOST_AUTO_TEST_CASE(child_rethrow_base_as_base) {
    test::output_test_stream o;
    try {
        f5();
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"
            "1: Rethrown at tests/Exception_1.cc:68 in void f5()\n"
            "1: Message: In f5\n"));
}

BOOST_AUTO_TEST_CASE(child_rethrow_base_as_child) {
    test::output_test_stream o;
    try {
        f5();
    }
    catch (ChildException const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"
            "1: Rethrown at tests/Exception_1.cc:68 in void f5()\n"
            "1: Message: In f5\n"));
}

BOOST_AUTO_TEST_CASE(child_rethrow_child_as_base) {
    test::output_test_stream o;
    try {
        f6();
    }
    catch (pexExcept::Exception const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"
            "1: Rethrown at tests/Exception_1.cc:78 in void f6()\n"
            "1: Message: In f6\n"));
}

BOOST_AUTO_TEST_CASE(child_rethrow_child_as_child) {
    test::output_test_stream o;
    try {
        f6();
    }
    catch (ChildException const& e) {
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "0: ChildException thrown at tests/Exception_1.cc:46 in void f2()\n"
            "0: Message: In f2 2008\n"
            "1: Rethrown at tests/Exception_1.cc:78 in void f6()\n"
            "1: Message: In f6\n"));
}

BOOST_AUTO_TEST_CASE(detailed_rethrow_base_as_detailed) {
    test::output_test_stream o;
    try {
        f7(7);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), 7);
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(7)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "1: Rethrown at tests/Exception_1.cc:88 in void f7(int)\n"
            "1: Message: In f7 (7 42 3.14159)\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(detailed_rethrow_detailed_as_detailed) {
    test::output_test_stream o;
    try {
        f8(8);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), 8);
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(8)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "1: Rethrown at tests/Exception_1.cc:98 in void f8(int)\n"
            "1: Message: In f8\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(detailed_rethrow_child_as_detailed) {
    test::output_test_stream o;
    try {
        f9(9);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), 9);
        o << e;
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(9)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(detailed_what_normal) {
    test::output_test_stream o;
    try {
        f9(10);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), 10);
        o << e.what();
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal(
            "Begin DetailedException(10)\n"
            "0: DetailedException thrown at tests/Exception_1.cc:50 in void f3(int)\n"
            "0: Message: In f3 foo\n"
            "End DetailedException\n"));
}

BOOST_AUTO_TEST_CASE(detailed_what_boobytrap) {
    test::output_test_stream o;
    try {
        f9(-1);
    }
    catch (DetailedException const& e) {
        BOOST_CHECK_EQUAL(e.getCount(), -1);
        o << e.what();
    }
    BOOST_CHECK(!o.is_empty(false));
    BOOST_CHECK(o.is_equal("DetailedException *"));
}

BOOST_AUTO_TEST_SUITE_END()
