// Copyright 2010 Jim Bosch.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PYTHON_FROM_PYTHON_FILESYSTEM_HPP
#define BOOST_PYTHON_FROM_PYTHON_FILESYSTEM_HPP

#include <boost/python/from_python/iterator.hpp>
#include <boost/filesystem/path.hpp>

namespace boost {
namespace python {

/**
 *  @brief An rvalue from-python converter that creates a boost::filesystem::path
 *         from a Python string.
 */
struct filesystem_path_from_python_str {

    filesystem_path_from_python_str() {
        converter::registry::push_back(
            &convertible,
            &construct,
            type_id< boost::filesystem::path >()
        );
    }

    static void* convertible(PyObject * obj) {
        try {
            object s(handle<>(borrowed(obj)));
            if (len(s) > 1) {
                if (!extract<std::string>(s).check())
                    return NULL;
            }
            return obj;
        } catch (error_already_set & err) {
            handle_exception();
            PyErr_Clear();
            return NULL;
        }
    }

    static void construct(PyObject* obj, converter::rvalue_from_python_stage1_data* data) {
        object s(handle<>(borrowed(obj)));
        typedef converter::rvalue_from_python_storage<boost::filesystem::path> storage_t;
        storage_t* storage = reinterpret_cast<storage_t*>(data);
        void* bytes = storage->storage.bytes;
        new (bytes) boost::filesystem::path(extract<std::string>(s)());
        data->convertible = bytes;
    }

};

} // namespace boost::python
} // namespace boost

#endif // !BOOST_PYTHON_FROM_PYTHON_FILESYSTEM_HPP
