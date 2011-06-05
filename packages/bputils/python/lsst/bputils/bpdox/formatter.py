import targets
import lookup
import textwrap

try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO

def formatName(obj, scope=(), delimiter="::"):
    if hasattr(obj, "name"):
        n = 0
        while n < len(scope) and scope[n] == obj.name[n]:
            n += 1
        return delimiter.join(obj.name[n:])
    else:
        return obj

def formatType(obj, scope=()):
    if scope:
        dictionary = {}
        for k, v in obj.dictionary.iteritems():
            dictionary[k] = formatName(v, scope=scope)
    else:
        dictionary = obj.dictionary
    return obj.template.format(**dictionary)

def formatCode(xml):
    """Format a doxygen XML "programlisting" node as plain text."""
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

class Formatter(object):

    templates = {
        "ClassWrapper": '{bp}::class_< {args} >', 
        "ClassDeclaration": \
            '{wrapper}{variable}(\n{indent1}"{name}",\n{indent1}{doc},\n{indent1}{init}\n{indent0})',
        "EnumDeclaration": \
            '{bp}::enum_< {ctype} >(\n{indent1}"{name}",\n{indent1}{doc}\n{indent0})',
        "EnumValue": 'value("{name}", {value})',
        "DataMemberVisitor":
            'def(\n{indent1}{bp}::const_aware::data_member(\n' \
            '{indent2}"{name}",\n{indent2}&{class_type}::{name},\n' \
            '{indent2}{doc}\n{indent1})\n{indent0})',
        }

    def __init__(self, codewidth=120, docwidth=80, bp="bp", indent="    "):
        self.codewidth = codewidth
        self.docwidth = docwidth
        self.bp = bp
        self.indent = indent

    def getClassWrapper(self, target, scope=(), bases=None, noncopyable=False, tparams="", 
                        templates=None, **kw):
        """Generate the Boost.Python wrapper type (class_<> instantiation) for the given Class target.

        Arguments:
          target -------- A Class target.
          scope --------- The C++ namespace considered active in qualifying names
          bases --------- A sequence of base classes to pass to bp::bases, or None to
                          use all base classes known to doxygen.
          noncopyable --- If True, add 'boost::noncopyable' to the wrapper template args.
          tparams ------- A '<>'-bracketed string of template parameters for the class.
          templates------ Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        if tparams is None: tparams = ""
        if bases is None:
            if target.bases:
                bases = [formatName(base, scope) for base in target.bases]
            else:
                bases = []
        args = [formatName(target, scope=scope) + tparams]
        if bases:
            args.append("{bp}::bases< {bases} >".format(bp=self.bp, bases=", ".join(bases)))
        if noncopyable:
            args.append("boost::noncopyable")
        return templates["ClassWrapper"].format(bp=self.bp, args=", ".join(args), **kw)

    def getClassDeclaration(self, target, scope=(), indent="", name=None, doc=None, tparams="",
                            variable=None, init=None, templates=None, **kw):
        """Generate a complete Boost.Python wrapper declaration.

        Arguments:
          target -------- A Class target.
          scope --------- The C++ namespace considered active in qualifying names.
          indent -------- Characters to add before all but the first line.
          name ---------- Python name for the class.
          doc ----------- Doc string for the class.  Should not include quotes.
          tparams ------- A '<>'-bracketed string of template parameters for the class.
          variable ------ C++ variable name for the wrapper.
          init ---------- A Constructor object, bp::init string, or None for no_init.
          templates ----- Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to getInitVisitor if init is a Constructor
        object, and to the string format calls, where they can be used by customized templates.
        """
        if templates is None: templates = self.templates
        if name is None: name = formatName(target, scope=scope)
        if variable is not None:
            variable = " " + variable
        else:
            variable = ""
        indent1 = indent + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, templates=templates, indent=indent1, **kw)
        if init is None: init = "{bp}::no_init".format(bp=self.bp)
        if isinstance(init, targets.Constructor):
            init = self.getInitVisitor(init, scope, indent=indent1, **kw)
        wrapper = self.getClassWrapper(target, scope, tparams=tparams, **kw)
        return templates["ClassDeclaration"].format(
            bp=self.bp, wrapper=wrapper, name=name, init=init, doc=doc, variable=variable,
            indent0=indent, indent1=indent1, **kw
            )

    def getEnum(self, target, scope=(), indent="", tscope=None, doc=None, templates=None, **kw):
        """Generate a bp::enum_ declaration for the given Enum object.

        Arguments:
          target -------- An Enum target.
          scope --------- The C++ namespace considered active in qualifying names (as a sequence of names).
          indent -------- Characters to add before all but the first line.
          tscope -------- Full C++ scope that encloses the enum (as a string).  Usually this is automatic,
                          but it can be provided manually for enums inside template classes.
          doc ----------- Doc string for the target, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        indent1 = indent + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, indent=indent1, templates=templates, **kw)
        if tscope is None:
            ctype = formatName(target, scope=scope)
        else:
            ctype = "::".join((tscope, target.name[-1]))
        name = target.name[-1]
        lines = [
            templates["EnumDeclaration"].format(
                bp=self.bp, ctype=ctype, indent0=indent, indent1=indent1, name=name, doc=doc, **kw
                )
            ]
        for value in target.values:
            if tscope is None:
                cvalue = formatName(value, scope=scope)
            else:
                cvalue = "::".join((tscope, target.name[-1]))
            lines.append(templates["EnumValue"].format(indent=indent1, name=value.name[-1], value=cvalue))
        return "\n{0}.".format(indent1).join(lines)
        
    def getVariable(self, target, scope=(), indent="", doc=None, class_type=None, templates=None, **kw):
        """Generate a C++ member function pointer, casted to its exact type to resolve
        overloads.

        Arguments:
          target -------- A Variable target.
          scope --------- The C++ namespace considered active in qualifying names.
          indent -------- Characters to add before all but the first line.
          class_type ---- The name of the class the data member belongs to.
          doc ----------- Doc string for the target, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        indent1 = indent + self.indent
        indent2 = indent1 + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, indent=indent2, templates=templates, **kw)
        name = target.name[-1]
        if target.is_static:
            template = self.templates["StaticDataMemberVisitor"]
        else:
            template = self.templates["DataMemberVisitor"]    
        return template.format(
            bp=self.bp, name=name, indent0=indent, indent1=indent1, indent2=indent2,
            class_type=class_type, doc=doc, **kw
            )
    
