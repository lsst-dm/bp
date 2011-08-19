import unittest
import test_mod
import os
from lsst.bputils import std

class TestVector(unittest.TestCase):

    def testCopyToList(self):
        v = test_mod.return_vector_as_list()
        self.assertEqual(type(v), list)
        self.assertEqual(v, [5,3])

    def testCopyToTuple(self):
        v = test_mod.return_vector_as_tuple()
        self.assertEqual(type(v), tuple)
        self.assertEqual(v, (5,3))

    def testWrapped(self):
        v = test_mod.return_vector_wrapped()
        self.assertEqual(type(v), test_mod.vector)
        self.assert_(test_mod.accept_vector_cref(v))
        self.assert_(test_mod.accept_vector_ref(v))

    def testContainerFromPythonSequence(self):
        v = [5, 3]
        self.assert_(test_mod.accept_vector_cref(v))
        self.assertRaises(TypeError, test_mod.accept_vector_ref, v)

class TestMap(unittest.TestCase):

    def testCopyToDict(self):
        v = test_mod.return_map_as_dict()
        self.assertEqual(type(v), dict)
        self.assertEqual(v, {5:"five",3:"three"})

    def testWrapped(self):
        v = test_mod.return_map_wrapped()
        self.assertEqual(type(v), test_mod.map)
        self.assert_(test_mod.accept_map_cref(v))
        self.assert_(test_mod.accept_map_ref(v))

    def testContainerFromPythonMapping(self):
        v = {5:"five",3:"three"}
        self.assert_(test_mod.accept_map_cref(v))
        self.assertRaises(TypeError, test_mod.accept_map_ref, v)

class TestConstSharedPtr(unittest.TestCase):

    def testSharedPtr(self):
        # only relies on Boost.Python internals, not new code
        v = test_mod.shared_ptr_example_class.return_shared_ptr()
        self.assertEqual(type(v), test_mod.shared_ptr_example_class)

    def testConstSharedPtr(self):
        v = test_mod.shared_ptr_example_class.return_const_shared_ptr()
        self.assertEqual(type(v), test_mod.shared_ptr_example_class)

class TestStreams(unittest.TestCase):

    def test_ofstream(self):
        s1 = "test string 1 for ofstream"
        s2 = "test string 2 for ofstream"
        filename = "ofstream_test_buffer"
        ofs = std.ofstream(filename)
        ofs.write(s1)
        ofs.close()
        f = open(filename, "r")
        self.assertEqual(s1, f.read())
        f.close()
        ofs = std.ofstream(filename, 'a')
        ofs.write(s2)
        ofs.close()
        f = open(filename, "r")
        self.assertEqual(s1 + s2, f.read())
        f.close()
        ofs = std.ofstream(filename, 'w')
        ofs.write(s1)
        ofs.close()
        f = open(filename, "r")
        self.assertEqual(s1, f.read())
        f.close()
        try:
            os.remove(filename)
        except:
            print "WARNING: could not delete test file '%s'" % filename

    def test_ostringstream(self):
        s1 = "test string 1 for ostringstream"
        s2 = "test string 2 for ostringstream"
        oss = std.ostringstream()
        oss.write(s1)
        oss.flush()
        self.assertEqual(oss.str(), s1)
        oss.write(s2)
        oss.flush()
        self.assertEqual(oss.str(), s1 + s2)

if __name__=="__main__":
    unittest.main()
