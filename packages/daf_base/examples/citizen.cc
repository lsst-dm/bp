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
 
#include <iostream>
#include <stdexcept>

#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "lsst/pex/exceptions.h"
#include "lsst/daf/base/Citizen.h"

//
// We'll fully qualify LsstBase here in the class definitions;
// when we come to exercise the classes we'll use lsst::fw

namespace rhl {

class Shoe : public lsst::daf::base::Citizen {
public:
    explicit Shoe(int i = 0) : Citizen(typeid(this)), _i(i) { };
    ~Shoe() { };
private:
    int _i;
};

}

using namespace rhl;

class MyClass : public lsst::daf::base::Citizen {
public:
    explicit MyClass(char const* typeName = 0);
    int addOne();
private:
    boost::scoped_ptr<int> _ptr;         // no need to track this alloc
};

MyClass::MyClass(char const* /* typeName */) :
    lsst::daf::base::Citizen(typeid(this)),
    _ptr(new int) {
    *_ptr = 0;
}

int MyClass::addOne() {
    return ++*_ptr;
}

using namespace lsst::daf::base;

MyClass *foo() {
    boost::scoped_ptr<Shoe> x(new Shoe(1));
    MyClass *myInstance = new MyClass();

    std::cout << "In foo\n";
    Citizen::census(std::cout);

    return myInstance;
}

Citizen::memId newCallback(Citizen const* ptr) {
    std::cout << boost::format("\tRHL Allocating memId %s\n") % ptr->repr();

    return 2;                           // trace every other subsequent allocs
}

Citizen::memId deleteCallback(Citizen const* ptr) {
    std::cout << boost::format("\tRHL deleting memId %s\n") % ptr->repr();

    return 0;
}

int main() {
#if 1
    (void)Citizen::setNewCallbackId(2);
    (void)Citizen::setDeleteCallbackId(3);
    (void)Citizen::setNewCallback(newCallback);
    (void)Citizen::setDeleteCallback(deleteCallback);
#endif
    Citizen::memId const firstId = Citizen::getNextMemId();
    Shoe x;

    // x isn't going to be deleted until main exists, so don't list as a leak
    x.markPersistent();
    
    boost::scoped_ptr<Shoe> y(new Shoe);
    boost::scoped_ptr<Shoe> z(new Shoe(10));
    
    MyClass *mine = ::foo();

    std::cout << boost::format("In main (%d objects)\n") % Citizen::census(0);

    boost::scoped_ptr<std::vector<Citizen const*> const> leaks(Citizen::census());
    for (std::vector<Citizen const*>::const_iterator cur = leaks->begin();
         cur != leaks->end(); cur++) {
        std::cerr << boost::format("    %s\n") % (*cur)->repr();
    }

    z.reset();                          // i.e. delete pointed-to object
    delete mine;

    ((int *)y.get())[0] = 0;            // deliberately corrupt the block
    try {
        std::cerr << "Checking corruption\n";
        (void)Citizen::hasBeenCorrupted();
    } catch(lsst::pex::exceptions::MemoryException& e) {
        std::cerr << "Memory check: " << e <<
            "Proceeding with trepidation\n";
        ((int *)y.get())[0] = 0xdeadbeef; // uncorrupt the block
    }

    y.reset();

    std::cout << boost::format("In main (%d objects)\n") % Citizen::census(0, firstId);
    Citizen::census(std::cout, firstId);
    
    return 0;
}
