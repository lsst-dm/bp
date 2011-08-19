import logging
from collections import OrderedDict
import os
from . import utils
from . import settings
import re

try:
    import xml.etree.cElementTree as ElementTree
except ImportError:
    import xml.etree.ElementTree

_kind_to_class = {}

class Index(object):

    def __init__(self, paths):
        self.by_refid = {}
        self.by_lscope = {}
        for path in paths:
            self.load(path)

    def load(self, path):
        xmlfile = os.path.join(path, "index.xml")
        logging.debug("Parsing xml index file '{0}'.".format(xmlfile))
        tree = ElementTree.parse(xmlfile)
        root_xml = tree.getroot()
        groups = []
        for compound_xml in root_xml.findall("compound"):
            logging.debug("  Creating compound index node for '{0}'.".format(compound_xml.findtext("name")))
            compound_class = self.by_refid.get("refid", None)
            if compound_class is None:
                compound_class = _kind_to_class.get(compound_xml.get("kind"), None)
            if compound_class is None:
                continue
            compound_node = compound_class(compound_xml, path)
            if isinstance(compound_node, ScopeNode):
                self.by_refid[compound_node.refid] = compound_node
                if compound_node.lscope in self.by_lscope:
                    self.by_lscope[compound_node.lscope].add(compound_node)
                else:
                    overloads = utils.OverloadSet(compound_node)
                    overloads.lscope = compound_node.lscope
                    self.by_lscope[compound_node.lscope] = overloads
            for member_xml in compound_xml.findall("member"):
                logging.debug("    Creating member index node for '{0}'.".format(member_xml.findtext("name")))
                member_node = self.by_refid.get(member_xml.get("refid"), None)
                if member_node is None:
                    member_class = _kind_to_class.get(member_xml.get("kind"), None)
                    if member_class is not None:
                        member_node = member_class(member_xml, compound_node)
                        self.by_refid[member_node.refid] = member_node
                    else:
                        continue
                compound_node.by_refid[member_xml.get("refid")] = member_node
            if isinstance(compound_node, GroupNode):
                groups.append(compound_node)
            if isinstance(compound_node, ScopeNode):
                for member_node in compound_node.by_refid.itervalues():
                    if member_node.name in compound_node.by_name:
                        compound_node.by_name[member_node.name].add(member_node)
                    else:
                        compound_node.by_name[member_node.name] = utils.OverloadSet(member_node)
                for name, overloads in compound_node.by_name.iteritems():
                    overloads.lscope = compound_node.lscope + (name,)
                    for node in overloads.all.itervalues():
                        node.lscope = overloads.lscope
                    self.by_lscope[overloads.lscope] = overloads
        for group in groups:
            # A member can be in both a group and some other compound, but it's
            # always defined in the group's xml file.
            for member in group.by_refid.itervalues():
                member.location = group

class Node(object):

    supports_auto_member = False

    def __init__(self, xml):
        """Construct a node from its XML index node."""
        self.refid = xml.get("refid")
        self.label = None
        self.is_template = False
        self.is_hidden = False
        self.is_loaded = False

    is_overloaded = property(lambda self: len(self.overloads.all) > 1)

    def hide(self):
        if not self.is_hidden:
            self.is_hidden = True
            del self.overloads.visible[self.refid]

    def __str__(self):
        if settings.print_ids:
            return "{0} / {1}".format("::".join(self.lscope), self.refid)
        else:
            return "::".join(self.lscope)

    def read(self, xml, index):
        """Construct a node from its XML detail node."""
        if xml.get("prot") == "protected" or xml.get("prot") == "private":
            self.hide()
            logging.debug("  hiding non-public member: '{0}'".format(self))
        if self.name.startswith("~") or self.name.startswith("operator"):
            self.hide()
            logging.debug("  hiding special member: '{0}'".format(self))
        if self.name.startswith("@"):
            self.hide()
            logging.debug("  hiding anonymous member: '{0}'".format(self))
        location = xml.find("location")
        if location is not None:
            if location.get("file").endswith(".py"):
                logging.debug("  hiding pure-python member: '{0}'".format(self))
                self.hide()
        self.brief = self.parseParagraphs(xml.find("briefdescription"), index)
        self.detailed = self.parseParagraphs(xml.find("detaileddescription"), index)
        self.is_loaded = True

    def parseParagraphs(self, xml, index):
        """Parse a doxygen XML element containing one or more "para" elements.  Any nested
        "param" elements (as is common with function documentation containing parameter documentation)
        will be parsed and used to update self.params (which must be populated before calling
        parseParagraphs).  A string representing the paragraph text is returned.
        """
        if xml is None: return ""
        paragraphs = []
        for paragraph in xml.findall("para"):
            terms = []
            if paragraph.text is not None: terms.append(paragraph.text)
            for child in paragraph:
                if child.tag == "parameterlist":
                    for parameteritem in child:
                        namelist = parameteritem.find("parameternamelist")
                        description = self.parseParagraphs(
                            parameteritem.find("parameterdescription"), index
                            )
                        for parametername in namelist.findall("parametername"):
                            for param in self.params:
                                if param.name == parametername.text:
                                    param.direction = parametername.get("direction")
                                    param.brief = description
                                    break
                elif child.tag == "programlisting":
                    paragraphs.append("".join(terms))
                    paragraphs.append(child)
                    terms = []
                elif child.tag == "ulink":
                    if child.get("url").startswith("bpdox."):
                        cmd = child.get("url")[6:]
                        if cmd.startswith("label:"):
                            label = cmd[6:].strip()
                            if self.label:
                                logging.warning(
                                    "  cannot apply label '{new}'; overload of '{name}' "
                                    "already already labeled '{old}'".format(
                                        new=label, name=self, old=self.label
                                        )
                                    )
                            self.label = label
                            logging.debug("  attaching label '{0}' to overload of '{1}'".format(label, self))
                        elif cmd.strip() == "ignore":
                            try:
                                self.hide()
                                logging.debug("  ignoring overload of '{0}'".format(self))
                            except:
                                logging.warning("  bpdox.ignore invalid for '{0}'".format(self))
                        else:
                            logging.warning("  unrecognized @bpdox command '{0}' for '{1}'".format(cmd, self))
                    if child.text is not None:
                        terms.append(child.text)
                elif child.text is not None:
                    terms.append(child.text)
                if child.tail is not None:
                    terms.append(child.tail)
            paragraphs.append("".join(terms))
        return paragraphs

class MemberNode(Node):

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        Node.__init__(self, xml)
        self.name = xml.findtext("name")
        self.location = parent
        if isinstance(parent, ScopeNode):
            self.fscope = parent
        else:
            self.fscope = None

    def load(self, index):
        if not self.is_loaded:
            self.location.load(index)
        if not self.is_loaded:
            raise RuntimeError("Failed to load object '{0}'.".format(self))

class CompoundNode(Node):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        Node.__init__(self, xml)
        self.lscope = tuple(xml.findtext("name").split("::"))
        self.xmlfile = os.path.join(path, "{0}.xml".format(self.refid))
        self.name = self.lscope[-1]
        self.by_refid = OrderedDict()
        self.fscope = None

    def load(self, index):
        if self.is_loaded: return
        logging.debug("Parsing xml compound file '{0}'.".format(self.xmlfile))
        root_xml = ElementTree.parse(self.xmlfile).getroot()
        for compound_xml in root_xml.findall("compounddef"):
            compound_node = index.by_refid[compound_xml.get("id")]
            compound_node.read(compound_xml, index)
            for section_xml in compound_xml.findall("sectiondef"):
                for member_xml in section_xml.findall("memberdef"):
                    member_node = index.by_refid.get(member_xml.get("id"), None)
                    if member_node is not None:
                        member_node.read(member_xml, index)
                        if section_xml.get("kind") == "related":
                            logging.warning("Moving member {0} from {1} to {2}".format(member_node, compound_node, compound_node.fscope))
                            member_node.fscope = compound_node.fscope
                            member_node.lscope = compound_node.lscope[:-1] + (member_node.name,)
                            member_node.overloads.remove(member_node)
                            overloads = index.by_lscope.get(member_node.lscope, None)
                            if overloads is None:
                                overloads = utils.OverloadSet()
                                overloads.lscope = member_node.lscope
                            overloads.add(member_node)
                            
        if len(self.lscope) > 1 and self.fscope is None:
            try:
                overloads = index.by_lscope[self.lscope[:-1]]
            except KeyError:
                logging.warning("Could not locate enclosing scope object for '{0}'".format(self))
            else:
                try:
                    self.fscope = overloads.get()
                except LookupError:
                    logging.warning("Could not resolve enclosing scope object for '{0}'".format(self))

class ScopeNode(CompoundNode):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        CompoundNode.__init__(self, xml, path)
        self.lscope = tuple(xml.findtext("name").split("::"))
        self.by_name = OrderedDict()

    def read(self, xml, index):
        """Construct a node from its XML detail node."""
        CompoundNode.read(self, xml, index)
        for child_xml in xml.findall("innerclass"):
            child_node = index.by_refid[child_xml.get("refid")]
            if child_node.lscope[-1] in self.by_name:
                overloads = self.by_name[child_node.name]
                overloads.add(child_node)
            else:
                self.by_name[child_node.name] = index.by_lscope[child_node.lscope]
            self.by_refid[child_node.refid] = child_node
            child_node.fscope = self

    def members(self, include_list=(), include_regex=None, exclude_list=(), exclude_regex=None, **kw):
        result = OrderedDict()
        def include_iter(iterable):
            for node in iterable:
                result[node.refid] = node
        def exclude_iter(iterable):
            for node in iterable:
                try:
                    del result[node.refid]
                except KeyError:
                    pass
        if include_regex:
            include_regex = re.compile(include_regex)
            if exclude_regex:
                exclude_regex = re.compile(exclude_regex)
                match = lambda x: include_regex.match(x) and not exclude_regex.match(x)
            else:
                match = lambda x: include_regex.match(x)
            for name, overloads in self.by_name.iteritems():
                if match(name):
                    include_iter(overloads.iterate())
        for name, labels in exclude_list:
            overloads = self.by_name[name]
            exclude_iter(overloads.iterate(labels))
        for name, labels in include_list:
            overloads = self.by_name[name]
            include_iter(overloads.iterate(labels))
        return result.values()
            
class ClassNode(ScopeNode):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        ScopeNode.__init__(self, xml, path)

    def read(self, xml, index):
        """Construct a node from its XML detail node."""
        ScopeNode.read(self, xml, index)
        bases = []
        for base_xml in xml.findall("basecompoundref"):
            try:
                base = index.by_refid[base_xml.get("refid")]
            except KeyError:
                bases.append((base_xml.text, None))
            else:
                if base.is_template:
                    i = base_xml.text.find(base.name)
                    if i == -1:
                        bases.append((base_xml.text, None))
                    else:
                        tparams = base_xml.text[i + len(base.name):]
                        bases.append((base, tparams))
                else:
                    bases.append((base, None))
        self.bases = tuple(bases)
        self.is_template = (xml.find("templateparamlist") != None)

    def formatDeclare(self, processor, indent, name=None, pyname=None, doc=None, bases=None, 
                      wrapper="wrapper", builder=None, const_aware=True, noncopyable=False, 
                      enable_shared_ptr=True, accept_pyname=False, **kw):
        customized = processor.getActiveCustomizedSet()
        template = \
            "\n{indent1}static void declare({func_args}) {{\n"\
            "{indent2}{bp}::class_< {tbody} > {wrapper}{builder};\n"\
            "{indent2}{bp}::scope in_{wrapper}({scope_arg});\n"\
            "{indent2}{lines};\n{indent1}}}"
        v = {"bp": settings.bp, "bpx": settings.bpx, "indent": indent, "wrapper": wrapper}
        v["indent1"] = v["indent"] + (" " * settings.indent)
        v["indent2"] = v["indent1"] + (" " * settings.indent)
        v["indent3"] = v["indent2"] + (" " * settings.indent)
        if name is None: name = processor.formatNode(self)
        if bases is None: 
            v["bases"] = list(processor.formatNode(base, tparams=tparams) for base, tparams in self.bases)
            bases = ", ".join(v["bases"])
        lines = []
        tbody = []
        v["name"] = name
        if const_aware:
            tbody.append("{bpx}::const_aware< {name} >".format(**v))
            v["scope_arg"] = "{wrapper}.main_class()".format(**v)
            if enable_shared_ptr:
                lines.append("{wrapper}.enable_shared_ptr()".format(**v))
        else:
            tbody.append(name)
            v["scope_arg"] = wrapper
            if enable_shared_ptr:
                lines.append("{bp}::register_ptr_to_python< boost::shared_ptr< {name} > >()".format(**v))
        if bases: tbody.append("{bp}::bases< {0} >".format(bases, **v))
        if noncopyable: tbody.append("boost::noncopyable")
        v["tbody"] = ", ".join(tbody)
        if doc is None: doc = processor.formatDocumentation(self, indent=v['indent3'])
        if builder is None:
            pbody = []
            if accept_pyname:
                v["func_args"] = "char const * pyname"
                pbody.append("pyname")
            else:
                if pyname is None: pyname = self.name
                v["func_args"] = ""
                pbody.append('"{0}"'.format(pyname))
            if doc and doc.strip('"\\n').strip(): pbody.append(doc)
            pbody.append("{bp}::no_init".format(**v))
            v["pbody"] = ",\n{indent3}".format(**v).join(pbody)
            v["builder"] = "(\n{indent3}{pbody}\n{indent2})".format(**v)
        else:
            v["pyname"] = pyname if pyname is not None else self.name
            v["doc"] = doc
            v["func_args"] = ""
            v["builder"] = builder.format(**v)
        for member in self.members(**kw):
            if member.is_template: continue
            if member.refid in customized: continue
            mbody = member.formatMember(processor, v['indent2'], wrapper=wrapper)
            if mbody is not None: lines.append(mbody)
        lines.append("customize({wrapper})".format(**v))
        for name in processor.getActiveStaticMethods():
            lines.append('{wrapper}.staticmethod("{0}")'.format(name, **v))
        v["lines"] = ";\n{indent2}".format(**v).join(lines)
        return template.format(**v)

    def formatMember(self, processor, indent, wrapper=None, pyname=None, **kw):  
        return None

_kind_to_class["class"] = ClassNode
_kind_to_class["struct"] = ClassNode

class NamespaceNode(ScopeNode):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        ScopeNode.__init__(self, xml, path)

    def read(self, xml, index):
        """Construct a node from its XML detail node."""
        ScopeNode.read(self, xml, index)
        for child_xml in xml.findall("innernamespace"):
            child_node = index.by_refid[child_xml.get("refid")]
            if child_node.lscope[-1] in self.by_name:
                overloads = self.by_name[child_node.lscope[-1]]
                overloads.add(child_node)
            else:
                self.by_name[child_node.lscope[-1]] = index.by_lscope[child_node.lscope]
            self.by_refid[child_node.refid] = child_node
            child_node.fscope = self

_kind_to_class["namespace"] = NamespaceNode

class GroupNode(CompoundNode):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        CompoundNode.__init__(self, xml, path)

_kind_to_class["group"] = GroupNode

class FileNode(CompoundNode):

    def __init__(self, xml, path):
        """Construct a node from its XML index node."""
        CompoundNode.__init__(self, xml, path)

_kind_to_class["file"] = FileNode

class EnumNode(MemberNode):

    supports_auto_member = True

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        MemberNode.__init__(self, xml, parent)
    
    is_anonymous = property(lambda self: self.name.startswith("@"))

    def read(self, xml, index):
        MemberNode.read(self, xml, index)
        self.values = []
        for value_xml in xml.findall("enumvalue"):
            value_node = index.by_refid[value_xml.get("id")]
            value_node.enum = self
            value_node.read(value_xml, index)
            self.values.append(value_node)

    def formatEnum(self, processor, indent, pyname=None, doc=None, export_values=True, **kw):
        indent1 = indent + (" " * settings.indent)
        if doc is None: doc = processor.formatDocumentation(self, indent=indent1)
        if pyname is None: pyname = self.name
        name = processor.formatNode(self)
        lines = []
        if doc is not None and doc.strip('"\\n').strip():
            head = '{bp}::enum_< {name} >(\n{indent1}"{pyname}",\n{indent1}{doc}\n{indent0})'
        else:
            head = '{bp}::enum_< {name} >("{pyname}")'
        lines.append(
            head.format(bp=settings.bp, name=name, indent0=indent, indent1=indent1, pyname=pyname, doc=doc)
            )
        for value in self.values:
            lines.append(
                '{indent1}.value("{name}", {value})'.format(
                    indent1=indent1, name=value.name, value=processor.formatNode(value)
                    )
                )
        if export_values:
            lines.append(indent1 + ".export_values()")
        return "\n".join(lines)

    def formatMember(self, processor, indent, wrapper=None, pyname=None, **kw):
        if wrapper is None:
            raise RuntimeError("Use @Enum, not @Member to wrap enums.")
        return self.formatEnum(processor, indent, pyname=pyname, **kw)

_kind_to_class["enum"] = EnumNode

class EnumValueNode(MemberNode):

    supports_auto_member = False

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        MemberNode.__init__(self, xml, parent)

    def read(self, xml, index):
        MemberNode.read(self, xml, index)
        self.value = xml.findtext("initializer")

    def formatMember(self, processor, indent, wrapper=None, pyname=None, **kw):  
        if not self.enum.is_anonymous: return None
        if wrapper is None:
            head = "setattr"
        else:
            head = wrapper + ".setattr"
        name = processor.formatNode(self)
        if pyname is None: pyname = self.name
        template = '{head}("{pyname}", int({name}))'
        return template.format(bp=settings.bp, head=head, pyname=pyname, name=name)

_kind_to_class["enumvalue"] = EnumValueNode

class TypeDefNode(MemberNode):

    supports_auto_member = True

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        MemberNode.__init__(self, xml, parent)

    def read(self, xml, index):
        MemberNode.read(self, xml, index)
        self.target = utils.CxxType(xml.find("type"), index)

    def formatMember(self, processor, indent, wrapper=None, pyname=None, **kw):  
        name = processor.formatNode(self)
        if pyname is None: pyname = self.name
        if wrapper is None:
            head = "add_static_property"
        else:
            head = wrapper + ".add_static_property"
        name = processor.formatNode(self)
        template = '{head}("{pyname}", &{bpx}::lookup_type< {name} >)'
        return template.format(bp=settings.bp, bpx=settings.bpx, head=head, pyname=pyname, name=name)

_kind_to_class["typedef"] = TypeDefNode

class VariableNode(MemberNode):

    supports_auto_member = True

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        MemberNode.__init__(self, xml, parent)

    def read(self, xml, index):
        MemberNode.read(self, xml, index)
        self.cxxtype = utils.CxxType(xml.find("type"), index)
        self.is_static = (xml.get("static") == "yes")

    def formatMember(self, processor, indent, wrapper=None, pyname=None, pointer=None, doc=None, **kw):
        indent1 = indent + (" " * settings.indent)
        indent2 = indent1 + (" " * settings.indent)
        name = processor.formatNode(self)
        if doc is None: doc = processor.formatDocumentation(self, indent=indent2)
        if pyname is None: pyname = self.name
        if wrapper is None:
            head = "def"
        else:
            head = wrapper + ".def"
        if self.is_static:
            template = '{head}({bpx}::data_member("{pyname}", &{name}))'
        else:
            template = '{head}(\n{indent1}{bpx}::data_member(\n' \
                '{indent2}"{pyname}",\n{indent2}&{name},\n' \
                '{indent2}{doc}\n{indent1})\n{indent0})'
        return template.format(
            bp=settings.bp, bpx=settings.bpx, head=head, indent0=indent, indent1=indent1, indent2=indent2, 
            pyname=pyname, name=name
            )

_kind_to_class["variable"] = VariableNode

class FunctionNode(MemberNode):

    supports_auto_member = True

    def __init__(self, xml, parent):
        """Construct a node from its XML index node."""
        MemberNode.__init__(self, xml, parent)

    def read(self, xml, index):
        params = []
        for param_xml in xml.findall("param"):
            type_xml = param_xml.find("type")
            default_xml = param_xml.find("defval")
            if default_xml is not None:
                default = utils.CxxType(default_xml, index)
            else:
                default = None
            param = utils.Parameter(
                name=param_xml.findtext("declname"),
                cxxtype=utils.CxxType(type_xml, index),
                default=default,
                brief=self.parseParagraphs(param_xml.find("briefdescription"), index)
                )
            if param.name is None:
                if param.cxxtype.template.strip() == "void":
                    continue
            params.append(param)
        self.params = tuple(params)
        MemberNode.read(self, xml, index)
        self.cxxtype = utils.CxxType(xml.find("type"), index)
        self.is_template = (xml.find("templateparamlist") != None)
        self.is_const = (xml.get("const") == "yes")
        self.is_static = (xml.get("static") == "yes")
        self.is_reimplementation = (xml.find("reimplementation") is not None)
    
    is_method = property(lambda self: isinstance(self.fscope, ClassNode))
    is_constructor = property(lambda self: self.is_method and self.name == self.fscope.lscope[-1])

    def formatKeywordList(self, processor):
        """Generate a parenthesized list of bp::arg() calls for the given Callable target.
        Returns None if the callable has no parameters.
        """
        if not self.params or any(p.name is None for p in self.params):
            return None
        terms = []
        for param in self.params:
            if param.default is not None:
                default = "={0}".format(processor.formatCxxType(param.default))
            else:
                default = ""
            terms.append(
                '{bp}::arg("{name}"){default}'.format(bp=settings.bp, name=param.name, default=default)
                )
        return "({0})".format(", ".join(terms))

    def formatParameterTypes(self, processor):
        """Generate a comma-separated list of C++ parameter types, suitable for use in a template
        argument or function pointer type.
        """
        result = []
        for param in self.params:
            result.append(processor.formatCxxType(param.cxxtype))
        return ", ".join(result)

    def formatPointer(self, processor, tparams=None):
        """Generate a C++ function pointer, casted to its exact type to resolve
        overloads.
        """
        assert(not self.is_constructor)
        if tparams is None and self.is_template:
            raise RuntimeError("Cannot generate pointer for {0} without template parameters".format(self))
        params = self.formatParameterTypes(processor)
        ret = processor.formatCxxType(self.cxxtype)
        name = processor.formatNode(self, tparams)
        if not self.is_overloaded:
            return "&{name}".format(name=name)
        if self.is_method and not self.is_static:
            const = " const" if self.is_const else ""
            return '({ret} ({cls}::*)({params}){const})&{name}'.format(
                bp=settings.bp, params=params, ret=ret, name=name, const=const, 
                cls=processor.formatNode(self.fscope)
                )
        else:
            return '({ret} (*)({params}))&{name}'.format(bp=settings.bp, params=params, ret=ret, name=name)

    def formatInitVisitor(self, processor, indent, policies=None, args=None, doc=None, **kw):
        """Generate a bp::init<>() visitor for use inside a bp::class_::def or bp::class_ constructor call.
        """
        assert(self.is_constructor)
        indent1 = indent + (" " * settings.indent)
        if doc is None: doc = processor.formatDocumentation(self, indent=indent1)
        if args is None: args = self.formatKeywordList(processor)
        if self.params and args:
            params = self.formatParameterTypes(processor)
            template = '{bp}::init< {params} >(\n{indent1}{args},\n{indent1}{doc}\n{indent0})'
        else:
            params = None
            template = '{bp}::init<>(\n{indent1}{doc}\n{indent0})'
        if policies is not None:
            template += '[{policies}]'
        return template.format(
            bp=settings.bp, args=args, params=params, indent0=indent, indent1=indent1, 
            policies=policies, doc=doc
            )

    def formatFunction(self, processor, indent, wrapper=None, pyname=None, tparams=None, pointer=None, 
                       policies=None, args=None, doc=None, **kw):
        indent1 = indent + (" " * settings.indent)
        if pyname is None: pyname = self.name
        if args is None: args = self.formatKeywordList(processor)
        if doc is None: doc = processor.formatDocumentation(self, indent=indent1)
        if pointer is None: pointer = self.formatPointer(processor, tparams)
        terms = ['"{0}"'.format(pyname), pointer]
        if args: terms.append(args)
        if policies: terms.append(policies)
        if doc and doc.strip('"\\n').strip(): terms.append(doc)
        if self.is_method and self.is_static:
            processor.markStaticMethod(pyname)
        if wrapper is None:
            head = "def"
        else:
            head = wrapper + ".def"
        return "{head}(\n{indent1}{body}\n{indent0})".format(
            head=head, 
            body=",\n{indent1}".format(indent1=indent1).join(terms), 
            indent0=indent,
            indent1=indent1,
            )

    def formatMember(self, processor, indent, wrapper=None, pyname=None, tparams=None, pointer=None, **kw):
        assert(self.is_method)
        indent1 = indent + (" " * settings.indent)
        if self.is_constructor:
            if pyname is not None:
                raise RuntimeError("Cannot rename constructor {0} in Python".format(self))
            if pointer is not None:
                raise RuntimeError("Cannot set function pointer for constructor {0}".format(self))
            if tparams is not None:
                raise RuntimeError("Cannot generate wrapper for templated constructor {0}".format(self))
            if wrapper is None:
                head = "def"
            else:
                head = wrapper + ".def"
            return "{head}(\n{indent1}{body}\n{indent0})".format(
                head=head,
                body=self.formatInitVisitor(processor, indent1, **kw), 
                indent0=indent, 
                indent1=indent1
                )
        else:
            return self.formatFunction(processor, indent, wrapper=wrapper, pyname=pyname, 
                                       tparams=tparams, pointer=pointer, **kw)
            
_kind_to_class["function"] = FunctionNode
