import targets
import lookup
import textwrap

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

class Formatter(object):

    templates = {
        "ClassWrapper": '{bp}::class_< {args} >', 
        "ClassDeclaration": \
            '{wrapper}{variable}(\n{indent1}"{name}",\n{indent1}{doc},\n{indent1}{init}\n{indent0})',
        "InitVisitor0": '{bp}::init<>(\n{indent1}{doc}\n{indent0})',
        "InitVisitorN": '{bp}::init< {param_types} >(\n{indent1}{keyword_args},\n{indent1}{doc}\n{indent0})',
        "KeywordArg": '{bp}::arg("{name}"){default}',
        "MethodDeclaration": 'def(\n{args}\n{indent0})',
        "MemberFunctionPointer": \
            '({return_type} ({class_type}::*)({param_types}){const})&{class_type}::{name}',
        "StaticMemberFunctionPointer": \
            '({return_type} (*)({param_types}))&{class_type}::{name}',
        "FunctionPointer": '({return_type} (*)({param_types}))&{name}',
        "FunctionDeclaration": '{bp}::def(\n{args}\n{indent0})',
        "EnumDeclaration": '{bp}::enum_< {ctype} >(\n{indent1}"{name}",\n{indent1}{doc}\n{indent0})',
        "EnumValue": 'value("{name}", {value})',
        }

    def __init__(self, codewidth=120, docwidth=80, bp="bp", indent="    "):
        self.codewidth = codewidth
        self.docwidth = docwidth
        self.bp = bp
        self.indent = indent

    def getDocumentation(self, target, scope=(), indent='', **kw):
        """Generate a Python docstring from a Target.

        The returned value includes quotes begin and end double-quotes. 
        Multi-line documentation will be wrapped and combined with raw '\n' characters as
        separators (so newlines will be interpreted by the C++ compiler not the code generator).

        Arguments:
          target--------- The Target to document.
          scope --------- The C++ namespace considered active in qualifying names.
          indent -------- Characters to add before all but the first line.
          noncopyable --- If True, add 'boost::noncopyable' to the wrapper template args.
          templates------ Dictionary of format strings to use (default is self.templates).
        """
        lines = []
        if target.brief.strip():
            lines.extend(textwrap.wrap(target.brief, width=self.docwidth))
        if hasattr(target, "params") and target.params:
            name_width = 0
            for param in target.params:
                if param.name and param.brief and len(param.name) > name_width:
                    name_width = len(param.name)
            if name_width > 0:
                lines.append("")
                lines.append("Arguments:")
                wrapper = textwrap.TextWrapper(
                    initial_indent="  ",
                    subsequent_indent=(" " * (name_width + 5)),
                    width=self.docwidth
                    )
                for param in target.params:
                    if not param.name or len(param.name) == 0: continue
                    sep = "-" * (name_width + 1 - len(param.name))
                    lines.extend(
                        wrapper.wrap(
                            "{name} {sep} {descr}".format(
                                name=param.name, sep=sep, descr=param.brief)
                            )
                        )
        if target.detailed.strip():
            lines.append("")
            lines.extend(textwrap.wrap(target.detailed, width=self.docwidth))
        if not lines:
            return '""'
        template = '{indent}"{line}\\n"'
        return "\n".join(
            [template.format(indent="", line=lines[0])] 
            + [template.format(indent=indent, line=line) for line in lines[1:]]
            )

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

    def getKeywordArgs(self, target, scope=(), templates=None, **kw):
        """Generate a parenthesized list of bp::arg() calls for the given Callable target.

        Returns an empty string if the callable has no parameters

        Arguments:
          target -------- A Callable target.
          scope --------- The C++ namespace considered active in qualifying names.
          templates ----- Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        if not target.params:
            return None
        terms = []
        for param in target.params:
            if param.default is not None:
                default = "={0}".format(param.default)
            else:
                default = ""
            terms.append(templates["KeywordArg"].format(bp=self.bp, name=param.name, default=default, **kw))
        return "({0})".format(", ".join(terms))

    def getParamTypes(self, target, scope=(), templates=None, **kw):
        """Generate a comma-separated list of C++ parameter types, suitable for use in a template
        argument or function pointer type.

        Arguments:
          target -------- A Callable target.
          scope --------- The C++ namespace considered active in qualifying names.
          templates ----- Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        result = []
        for param in target.params:
            result.append(formatType(param.cxxtype, scope))
        return ", ".join(result)

    def getInitVisitor(self, target, scope=(), indent="", doc=None, templates=None, **kw):
        """Generate a bp::init<>() visitor for use inside a bp::class_::def or bp::class_ constructor call.

        Arguments:
          target -------- A Constructor target.
          scope --------- The C++ namespace considered active in qualifying names.
          doc ----------- Doc string for the constructor, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        indent1 = indent + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, indent=indent1, templates=templates, **kw)
        if target.params:
            param_types = self.getParamTypes(target, scope, templates=templates, **kw)
            keyword_args = self.getKeywordArgs(target, scope, templates=templates, **kw)
            return templates["InitVisitorN"].format(
                bp=self.bp, keyword_args=keyword_args, param_types=param_types, 
                indent0=indent, indent1=indent1, doc=doc, **kw
                )
        else:
            return templates["InitVisitor0"].format(
                bp=self.bp, doc=doc, indent0=indent, indent1=indent1, **kw
                )

    def getInitDeclaration(self, target, scope=(), indent="", doc=None, templates=None, **kw):
        """Generate a bp::init<>() visitor for use inside a bp::class_::def or bp::class_ constructor call.

        Arguments:
          target -------- A constructor target.
          scope --------- The C++ namespace considered active in qualifying names.
          doc ----------- Doc string for the constructor, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).

        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        indent1 = indent + self.indent
        visitor = self.getInitVisitor(target, scope=scope, indent=indent1, doc=doc, 
                                      templates=templates, **kw)
        return templates["MethodDeclaration"].format(
            bp=self.bp, args=indent1+visitor, indent0=indent, indent1=indent1, **kw
            )

    def getMemberFunctionPointer(self, target, scope=(), class_type=None, templates=None, **kw):
        """Generate a C++ member function pointer, casted to its exact type to resolve
        overloads.

        Arguments:
          target -------- A Method target.
          scope --------- The C++ namespace considered active in qualifying names.
          class_type ---- The name of the class the member function belongs to.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        const = " const" if target.is_const else ""
        return_type = formatType(target.cxxtype, scope)
        param_types = self.getParamTypes(target, scope, templates=templates)
        if target.is_static:
            return templates["StaticMemberFunctionPointer"].format(
                bp=self.bp, return_type=return_type, param_types=param_types, class_type=class_type,
                name=target.name[-1], **kw
                )    
        else:
            return templates["MemberFunctionPointer"].format(
                bp=self.bp, return_type=return_type, param_types=param_types, class_type=class_type,
                name=target.name[-1], const=const, **kw
                )

    def getFunctionPointer(self, target, scope=(), templates=None, **kw):
        """Generate a C++ function pointer, casted to its exact type to resolve
        overloads.

        Arguments:
          target -------- A Function target.
          scope --------- The C++ namespace considered active in qualifying names.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        return_type = formatType(target.cxxtype, scope)
        name = formatName(target, scope=scope)
        param_types = self.getParamTypes(target, scope, templates=templates)
        return templates["FunctionPointer"].format(
            bp=self.bp, return_type=return_type, param_types=param_types, name=name, **kw
            )

    def getMethodDeclaration(self, target, scope=(), indent="", name=None, class_type=None, 
                             call_policies=None, doc=None, templates=None, **kw):
        """Generate a bp::class_::def call for the given Method target.

        Arguments:
          target -------- A Method target.
          scope --------- The C++ namespace considered active in qualifying names.
          indent -------- Characters to add before all but the first line.
          name ---------- Python name for the method.
          class_type ---- The name of the class the member function belongs to.
          call_policies - A string to use as the Boost.Python call policies for the method.
          doc ----------- Doc string for the target, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        if name is None: name = target.name[-1]
        indent1 = indent + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, indent=indent1, templates=templates, **kw)
        args = ['"{0}"'.format(name), 
                self.getMemberFunctionPointer(target, scope, templates=templates, 
                                              class_type=class_type, **kw)]
        if call_policies is not None:
            args.append(call_policies)
        if target.params:
            args.append(self.getKeywordArgs(target, scope, templates=templates, **kw))
        args.append(doc)
        return templates["MethodDeclaration"].format(
            bp=self.bp, args=",\n".join((indent1 + arg) for arg in args),
            indent0=indent, indent1=indent1
            )

    def getFunctionDeclaration(self, target, scope=(), indent="", name=None,
                               call_policies=None, doc=None, templates=None, **kw):
        """Generate a bp::def call for the given Function target.

        Arguments:
          target -------- A Function target.
          scope --------- The C++ namespace considered active in qualifying names.
          indent -------- Characters to add before all but the first line.
          name ---------- Python name for the method.
          call_policies - A string to use as the Boost.Python call policies for the method.
          doc ----------- Doc string for the target, including extra quotes.
          templates ----- Dictionary of format strings to use (default is self.templates).
        
        Additional keyword arguments will be passed on to the string format calls and can be used
        by customized templates.
        """
        if templates is None: templates = self.templates
        if name is None: name = target.name[-1]
        indent1 = indent + self.indent
        if doc is None:
            doc = self.getDocumentation(target, scope, indent=indent1, templates=templates, **kw)
        args = ['"{0}"'.format(name), self.getFunctionPointer(target, scope, templates=templates, **kw)]
        if call_policies is not None:
            args.append(call_policies)
        if target.params:
            args.append(self.getKeywordArgs(target, scope, templates=templates, **kw))
        args.append(doc)
        return templates["FunctionDeclaration"].format(
            bp=self.bp, args=",\n".join((indent1 + arg) for arg in args), indent0=indent, indent1=indent1
            )

    def getEnum(self, target, scope=(), indent="", tscope=None, doc=None, templates=None, **kw):
        """Generate a bp::def call for the given Function target.

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
