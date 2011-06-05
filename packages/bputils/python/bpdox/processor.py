from ._bpdox import ProcessorBase
from . import nodes
from . import settings
from . import macros
from . import utils
import os.path
import logging
import textwrap
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO


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
    inFile = open(target, 'r')
    outFile = open(basetarget, 'w')
    processor = Processor(paths)
    processor.process(inFile, outFile)

class MacroException(SyntaxError):

    def format(self):
        if settings.verbose:
            return "bpdox macro error on line {2}:\n{3}\n".format(*self.args)
        else:
            return "bpdox macro error on line {2}: {0.__name__}: {1}\n".format(*self.args)
        
class Processor(ProcessorBase):

    def __init__(self, paths):
        ProcessorBase.__init__(self)
        self._index = nodes.Index(paths)
        self._namespace = None
        self._classes = []
        self._lscope = ()
        self._tparams = {}
        self._staticmethods = []
        self._customized = []
        for macro in macros.registry:
            self.register(macro())

    def process(self, inFile, outFile):
        data = self._process(inFile.read())
        for i in range(len(data)):
            if isinstance(data[i], tuple):
                macro, method, indent, line, options = data[i]
                try:
                    data[i] = getattr(macro, method)(indent, options, self)
                except Exception, err:
                    import traceback
                    raise MacroException(type(err), err, line, traceback.format_exc())
        outFile.write("".join(data))

    def markStaticMethod(self, name):
        assert(self._staticmethods)
        self._staticmethods[-1].add(name)

    def setNamespace(self, node):
        assert(node is None or node.is_loaded)
        if self._classes:
            raise RuntimeError("Cannot set namespace scope inside class scope")
        self._namespace = node
        self._lscope = node.lscope if node else ()

    def pushClass(self, node, tparams=None):
        assert(node.is_loaded)
        assert(node.is_template == (tparams is not None))
        if self._classes:
            if node.fscope.refid != self._classes[-1].refid:
                raise RuntimeError("'{0}' is not an inner class of '{1}'".format(node, self._classes[-1]))
        elif self._namespace:
            if node.fscope.refid != self._namespace.refid:
                raise RuntimeError("'{0}' in not in namespace '{1}'".format(node, self._namespace))
        else:
            if node.fscope is not None:
                raise RuntimeError("'{0}' is not in the global namespace".format(node))
        self._classes.append(node)
        self._staticmethods.append(set())
        self._customized.append(set())
        if node.is_template:
            self._tparams[node.refid] = tparams
        self._lscope = self._classes[-1].lscope
        
    def popClass(self):
        assert(self._classes)
        if self._classes[-1].is_template:
            del self._tparams[self._classes[-1].refid]
        del self._classes[-1]
        del self._staticmethods[-1]
        del self._customized[-1]
        if self._classes:
            self._lscope = self._classes[-1].lscope
        elif self._namespace:
            self._lscope = self._namespace.lscope
        else:
            self._lscope = ()

    def getActiveClass(self):
        return self._classes[-1] if self._classes else None

    def getActiveNamespace(self):
        return self._namespace

    def getActiveStaticMethods(self):
        return self._staticmethods[-1] if self._classes else None

    def getActiveCustomizedSet(self):
        return self._customized[-1] if self._classes else None

    def lookup(self, ref, no_overloads=False, iterate=False):
        if isinstance(ref, tuple):
            name, labels = ref
            name = tuple(name.split("::"))
        else:
            m = utils.labeled_regex.match(ref)
            name = tuple(m.group("name").split("::"))
            labels = m.group("labels")
        for n in range(len(self._lscope), -1, -1):
            scope = self._lscope[:n]
            overloads = self._index.by_lscope.get(scope + name, None)
            if overloads is not None:
                overloads.load(self._index)
                if no_overloads:
                    if labels is None:
                        return overloads.get()
                    elif len(labels) == 1:
                        return overloads.get(labels[0])
                    else:
                        raise LookupError("Multiple labels provided for single-target argument.")
                elif iterate:
                    return overloads.iterate(labels)
                else:
                    return overloads.filtered(labels)
        else:
            raise LookupError("Name '{0}' not found".format("::".join(name)))

    def formatNode(self, node, tparams=None, terms=None, add_typename=False):
        if not isinstance(node, nodes.Node):
            return node
        node.load(self._index)
        if terms is None:
            terms = []
        if add_typename:
            prefix = "typename "
        else:
            prefix = ""
        current = node
        def finish():
            terms.reverse()
            return prefix + "::".join(terms)
        while current is not None:
            if isinstance(current, nodes.ClassNode):
                if current.is_template:
                    if current.refid == node.refid and tparams:
                        current_tparams = tparams
                    else:
                        try:
                            current_tparams = self._tparams[current.refid]
                        except KeyError:
                            current_tparams = ""
                    if (node != current and 
                        isinstance(node, (nodes.ClassNode, nodes.TypeDefNode, nodes.EnumNode))):
                        prefix = "typename "
                    terms.append(current.name + current_tparams)
                else:
                    terms.append(current.name)
            elif isinstance(current, nodes.NamespaceNode):
                current.load(self._index)
                scope = self._namespace
                while scope is not None:
                    if scope.refid == current.refid:
                        return finish()
                    scope = scope.fscope
                terms.append(current.name)
            else:
                if current.is_template and current.refid == node.refid and tparams:
                    terms.append(current.name + tparams)
                else:
                    terms.append(current.name)
            current = current.fscope
        return finish()

    def formatRef(self, ref):
        if ref.target is None:
            logging.warning("Could not resolve '{0}' - scope may be incorrect (include "
                            "additional xml directories to resolve this problem)".format(ref.text))
            return ref.text
        ref.target.load(self._index)
        if ref.target.is_template:
            if ref.has_tparams:
                return self.formatNode(ref.target.fscope, terms=[ref.target.name], 
                                       add_typename=ref.has_nested)
            if ref.target.name != ref.text.split("::")[-1]:
                try:
                    target = self.lookup(ref.text, no_overloads=True)
                except LookupError:
                    return ref.text
                else:
                    return self.formatNode(target)
        return self.formatNode(ref.target)

    def formatCxxType(self, cxxtype):
        result = cxxtype.template.format(*[self.formatRef(ref) for ref in cxxtype.refs])
        return result.replace("typename typename ", "typename ")

    def formatCode(self, xml):
        """Format a doxygen XML "programlisting" node as a list of strings."""
        lines = []
        for codeline in xml.findall("codeline"):
            buf = StringIO()
            for highlight in codeline.findall("highlight"):
                if highlight.text:
                    buf.write(highlight.text)
                for child in highlight:
                    if child.text: buf.write(child.text)
                    if child.tag == "sp":
                        buf.write(" ")
                    if child.tail: buf.write(child.tail)
            lines.append(buf.getvalue())
        return lines

    def formatDocumentation(self, node, indent=''):
        """Generate a Python docstring from a Node.

        The returned value includes quotes begin and end double-quotes. 
        Multi-line documentation will be wrapped and combined with raw '\n' characters as
        separators (so newlines will be interpreted by the C++ compiler not the code generator).
        """
        lines = []
        if node.brief:
            for item in node.brief:
                if hasattr(item, "tag"):
                    lines.extend(self.formatCode(item))
                else:
                    lines.extend(textwrap.wrap(item, width=settings.docwidth))
                lines.append("")
        if hasattr(node, "params") and node.params:
            name_width = 0
            for param in node.params:
                if param.name and param.brief and len(param.name) > name_width:
                    name_width = len(param.name)
            if name_width > 0:
                lines.append("Arguments:")
                wrapper = textwrap.TextWrapper(
                    initial_indent="  ",
                    subsequent_indent=(" " * (name_width + 5)),
                    width=settings.docwidth
                    )
                for param in node.params:
                    if not param.name or len(param.name) == 0: continue
                    sep = "-" * (name_width + 1 - len(param.name))
                    if param.brief:
                        lines.extend(
                            wrapper.wrap(
                                "{name} {sep} {descr}".format(
                                    name=param.name, sep=sep, descr=param.brief[0])
                                )
                            )
                    if len(param.brief) > 1:
                        for item in param.brief[1:]:
                            if hasattr(item, "tag"):
                                lines.extend(self.formatCode(item))
                            else:
                                lines.extend(textwrap.wrap(item, width=settings.docwidth))
                lines.append("")
        if node.detailed:
            for item in node.detailed:
                if hasattr(item, "tag"):
                    lines.extend(self.formatCode(item))
                else:
                    lines.extend(textwrap.wrap(item, width=settings.docwidth))
                lines.append("")
        if not lines:
            return '""'
        lines = [line.replace('\\', r'\\') for line in lines]
        lines = [line.replace('"', r'\"') for line in lines]
        template = '{indent}"{line}\\n"'
        return "\n".join(
            [template.format(indent="", line=lines[0])] 
            + [template.format(indent=indent, line=line) for line in lines[1:]]
            )
        
