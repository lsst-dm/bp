#!/usr/bin/env python
import os
import subprocess
import sys
import shutil
import eups

root = os.path.abspath(os.path.dirname(__file__))

packages = [
    ("boost", "1.46.1+bp", "boost_1_46_1.tar.gz", "boost_1_46_1"),
    ("sconsDistrib", "2.0.1+bp", "scons-2.0.1.tar.gz", "scons-2.0.1"),
    ("xpa", "2.1.13+bp", "xpa-2.1.13.tar.gz", "xpa-2.1.13"),
    ("pyfits", "2.4.0+bp", "pyfits-2.4.0.tar.gz", "pyfits-2.4.0"),
    ("doxygen", "1.7.4", "doxygen-1.7.4.src.tar.gz", "doxygen-1.7.4"),
    ("ipython", "0.10.2+bp", "ipython-0.10.2.tar.gz", "ipython-0.10.2"),
    ]

def main(todo):
    for name, version, tar_file, tar_dir in packages:
        if name not in todo:
            continue
        if os.path.isdir(os.path.join(root, "src", tar_dir)):
            print "rm -rf %s" % os.path.join(root, "src", tar_dir)
            shutil.rmtree(os.path.join(root, "src", tar_dir))
        print "tar -xzf %s" % tar_file
        result = subprocess.call("tar -xzf %s" % tar_file, cwd=os.path.join(root, "src"), shell=True)
        if result: sys.exit(result)
        install = os.path.join(root, "install", name, version)
        extras = os.path.join(root, "src", "build", name)
        script = os.path.join(root, "src", "build", "%s.sh" % name)
        cmd = "%(script)s %(name)s %(version)s %(install)s %(extras)s" % locals()
        result = subprocess.call(cmd, cwd=os.path.join(root, "src", tar_dir), shell=True)
        if result: sys.exit(result)
        cmd = "eups declare --force %(name)s %(version)s -r %(install)s" % locals()
        print cmd
        result = subprocess.call(cmd, shell=True)
        if result: sys.exit(result)

if __name__ == "__main__":
    main(sys.argv[1:])
