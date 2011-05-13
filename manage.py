#!/usr/bin/env python
import os
import sys
import glob

def main(args):
    cmd = args[0]
    if cmd == "declare":
        template = "eups declare --force -r %s %s bp"
        for tablePath in glob.glob("packages/*/ups/*.table"):
            filePath, ext = os.path.splitext(tablePath)
            upsDir, package = os.path.split(filePath)
            rootDir = os.path.normpath(os.path.join(upsDir, ".."))
            if package == "sconsDistrib": #rootDir.split("/")[-1]:
                continue
            s = template % (rootDir, package)
            print s
            if os.system(s) != 0:
                sys.exit(1)
    else:
        print "Unknown command."

if __name__ == "__main__":
    main(sys.argv[1:])
