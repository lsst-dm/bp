#!/usr/bin/env python
import os
import sys

packages = [
    ("base", "base"),
    ("bputils", "bputils"),
    ("pex/exceptions", "pex_exceptions"),
    ("utils", "utils"),
    ("daf/base", "daf_base"),
    ("pex/logging", "pex_logging"),
    ("pex/policy", "pex_policy"),
    ]

def main(args):
    cmd = args[0]
    if cmd == "declare":
        template = "eups declare --force -r {path} {name} {bp}"
        for path, name in packages:
            s = template.format(path=path, name=name)
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
            print "cd {0}".format(abspath)
            os.chdir(abspath)
            print scons
            if os.system(scons) != 0:
                os.chdir(root)
                sys.exit(1)
        os.chdir(root)
    elif cmd == "env":
        os.system("env")
    else:
        print "Unknown command."

if __name__ == "__main__":
    main(sys.argv[1:])