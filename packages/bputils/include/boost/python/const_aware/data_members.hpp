// Copyright 2011 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_CONST_AWARE_DATA_MEMBERS_HPP
#define BOOST_PYTHON_CONST_AWARE_DATA_MEMBERS_HPP

#include "boost/python.hpp"
#include "boost/python/const_aware/as_const.hpp"
#include "boost/type_traits/is_array.hpp"
#include "boost/type_traits/is_pointer.hpp"

namespace boost { namespace python { namespace const_aware {

template <typename W, typename T1, typename T2, typename T3> class exposer;

template <typename T>
class static_data_member_visitor : public def_visitor< static_data_member_visitor<T> > {

    BOOST_STATIC_ASSERT(
        ((!boost::is_pointer<T>::value && !boost::is_array<T>::value) 
         || boost::is_same<T,char const *>::value)
    );

    friend class def_visitor_access;

    typedef typename detail::default_datum_getter_policy<T>::type base_policies;

    typedef typename detail::value_is_shared_ptr<T>::type is_shared_ptr;

    typedef typename mpl::if_< is_shared_ptr, base_policies, as_const<base_policies> >::type const_policies;

    typedef typename mpl::if_< is_const<T>, const_policies, base_policies >::type policies;

public:

    explicit static_data_member_visitor(char const * name, T * ptr) : _name(name), _ptr(ptr) {}

    template <typename Wrapper>
    void visit(Wrapper & wrapper) const {
        this->do_visit(wrapper, (typename is_const<T>::type*)0);
    }

    char const * get_name() const { return _name; }

private:

    template <typename Wrapper>
    void do_visit(Wrapper & wrapper, boost::mpl::true_ *) const {
        wrapper.add_static_property(_name, make_getter(_ptr, policies()));
    }

    template <typename Wrapper>
    void do_visit(Wrapper & wrapper, boost::mpl::false_ *) const {
        wrapper.add_static_property(_name, make_getter(_ptr, policies()), make_setter(_ptr));
    }

    char const * _name;
    T * _ptr;
};

template <typename C, typename T>
class data_member_visitor : public def_visitor< data_member_visitor<C,T> > {

    BOOST_STATIC_ASSERT(
        ((!boost::is_pointer<T>::value && !boost::is_array<T>::value)
         || boost::is_same<T,char const *>::value)
    );

    friend class def_visitor_access;

    typedef typename detail::default_member_getter_policy<T>::type base_policies;

    typedef typename detail::value_is_shared_ptr<T>::type is_shared_ptr;

    typedef typename mpl::if_< is_shared_ptr, base_policies, as_const<base_policies> >::type const_policies;

    typedef typename mpl::if_< is_const<T>, const_policies, base_policies >::type default_policies;

public:

    explicit data_member_visitor(char const * name, T C::* ptr, char const * doc=0) :
        _name(name), _doc(doc), _ptr(ptr)
    {}

    template <typename W, typename T1, typename T2, typename T3>
    void visit(class_<W,T1,T2,T3> & wrapper) const {
        this->do_visit(wrapper, default_policies(), (typename is_const<T>::type*)0);
    }

    template <typename W, typename T1, typename T2, typename T3>
    void visit(exposer<W,T1,T2,T3> & wrapper) const {
        this->do_visit(wrapper.main_class(), default_policies(), (typename is_const<T>::type*)0);
        this->do_visit(wrapper.const_proxy(), const_policies(), (mpl::true_*)0);
    }

    char const * get_name() const { return _name; }

private:

    template <typename Wrapper, typename Policies>
    void do_visit(Wrapper & wrapper, Policies const & policies, mpl::true_ *) const {
        wrapper.add_property(_name, make_getter(_ptr, policies), _doc);
    }

    template <typename Wrapper, typename Policies>
    void do_visit(Wrapper & wrapper, Policies const & policies, mpl::false_ *) const {
        wrapper.add_property(_name, make_getter(_ptr, policies), make_setter(_ptr), _doc);
    }

    template <typename Wrapper, typename Policies>
    void do_visit(Wrapper & wrapper, Policies const & policies) const {
        wrapper.add_property(_name, make_getter(_ptr, policies), make_setter(_ptr), _doc);
    }

    char const * _name;
    char const * _doc;
    T C::* _ptr;
};

template <typename T>
static_data_member_visitor<T> data_member(char const * name, T * ptr) {
    return static_data_member_visitor<T>(name, ptr);
}

template <typename C, typename T>
data_member_visitor<C,T> data_member(char const * name, T C::*ptr, char const * doc = 0) {
    return data_member_visitor<C,T>(name, ptr, doc);
}

}}} // namespace boost::python::const_aware

#endif // !BOOST_PYTHON_CONST_AWARE_DATA_MEMBERS_HPP
