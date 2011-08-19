#define BOOST_PYTHON_EXTENSIONS_NUMPY_INTERNAL_MAIN
#include <boost/python/extensions/numpy/internal.hpp>

namespace boost { namespace python { namespace extensions { namespace numpy {

void initialize() {
    import_array();
    import_ufunc();
}

}}}} // namespace boost::python::extensions::numpy
