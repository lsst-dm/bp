from .formatter import Formatter
from .lookup import Index
from .generator import Generator
import os.path
import logging

def searchRoot(root, paths, matched):
    todo = []
    for path in paths:
        if os.path.isabs(path):
            if os.path.exists(os.path.join(path, "index.xml")):
                matched.append(path)
            else:
                raise IOError("No index.xml file in absolute path {0}.".format(path))
        elif os.path.exists(os.path.join(root, path, "index.xml")):
            matched.append(os.path.join(root, path))
        else:
            todo.append(path)
    return todo

def expandPaths(paths):
    matched = []
    todo = searchRoot(".", paths, matched)
    for root in os.environ.get("BPDOX_PATH", "").split(":"):
        todo = searchRoot(root, todo, matched)
    if todo:
        raise IOError("No index.xml file in {0}.".format(todo[0]))
    return matched

def main(*args):
    # TODO: command line parsing for formatter arguments
    target = args[0]
    basetarget, garbage = os.path.splitext(target)
    paths = expandPaths(args[1:])
    for path in paths:
        print "Using Doxygen XML output in {0}".format(path)
    index = Index(paths)
    formatter = Formatter()
    generator = Generator(formatter, index)
    input = open(target, 'r')
    output = open(basetarget, 'w')
    generator(input, output)
