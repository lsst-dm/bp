#!/usr/bin/env python
import os
import sys

packages = [
    ("base", "base"),
    ("ndarray", "ndarray"),
    ("bputils", "bputils"),
    ("pex/exceptions", "pex_exceptions"),
    ("utils", "utils"),
    ("daf/base", "daf_base"),
    ("pex/logging", "pex_logging"),
    ("pex/policy", "pex_policy"),
    ("daf/persistence", "daf_persistence"),
    ("security", "security"),
    ("daf/data", "daf_data"),
    ]

def main(args):
    cmd = args[0]
    if cmd == "declare":
        template = "eups declare --force -r %s %s bp"
        for path, name in packages:
            s = template % (path, name)
            print s
            if os.system(s) != 0:
                sys.exit(1)
    elif cmd == "build" or cmd == "clean":
        root = os.path.abspath(os.curdir)
        scons = "scons"
        if cmd == "clean":
            scons += " --clean"
        for path, name in packages:
            abspath = os.path.join(root, path)
            print "cd %s" % abspath
            os.chdir(abspath)
            print scons
            if os.system(scons) != 0:
                os.chdir(root)
                sys.exit(1)
        os.chdir(root)
    elif cmd == "fix-import":
        paths = []
        for path, name in packages:
            subpaths = path.split("/")
            for n in range(0, len(subpaths)):
                paths.append(os.path.join(path, "python", "lsst", *subpaths[:n]))
        paths.append("sconsUtils/python/lsst")
        for path in paths:
            if not os.path.exists(path): continue
            print "fixing", path
            f = open(os.path.join(path, "__init__.py"), 'w')
            f.write("import pkgutil\n")
            f.write("__path__ = pkgutil.extend_path(__path__, __name__)\n")
            f.close()
    elif cmd == "env":
        os.system("env")
    else:
        print "Unknown command."

if __name__ == "__main__":
    main(sys.argv[1:])
