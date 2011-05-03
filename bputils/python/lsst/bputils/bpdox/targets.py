import re
import logging

class Target(object):
    """A base class for target nodes in the Doxygen XML.

    Attributes:
      brief ----- Doxygen "brief" description.
      detailed -- Doxygen "detailed" description.
    """

    def parseParagraphs(self, xml, index, node):
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
                            parameteritem.find("parameterdescription"), index, node
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
                            if hasattr(node, "label"):
                                logging.warning(
                                    "    cannot apply label '{new}'; overload of '{name}' "
                                    "already already labeled '{old}'".format(
                                        new=label, name="::".join(node.name), old=node.label
                                        )
                                    )
                            node.label = label
                            logging.debug(
                                "    attaching label '{0}' to overload of '{1}'".format(
                                    label, "::".join(node.name)
                                    )
                                )
                        elif cmd.strip() == "ignore":
                            try:
                                overloads = node.parent.members[node.name[-1]]
                                overloads.ignore(node.refid)
                                logging.debug("    ignoring overload of '{0}'".format("::".join(node.name)))
                            except:
                                logging.warning(
                                        "    bpdox.ignore invalid for '{0}'".format("::".join(node.name))
                                        )
                        else:
                            logging.warning(
                                "    unrecognized @bpdox command '{0}' for object '{1}'".format(
                                    cmd, "::".join(node.name)
                                    )
                                )
                    if child.text is not None:
                        terms.append(child.text)
                elif child.text is not None:
                    terms.append(child.text)
                if child.tail is not None:
                    terms.append(child.tail)
            paragraphs.append("".join(terms))
        return paragraphs

    def __init__(self, xml, index, node):
        self.brief = self.parseParagraphs(xml.find("briefdescription"), index, node)
        self.detailed = self.parseParagraphs(xml.find("detaileddescription"), index, node)
        self.name = node.name
        if xml.get("prot") == "protected" or xml.get("prot") == "private":
            node.hidden = True

class Class(Target):
    """A Target subclass for classes and structs.

    Attributes:
      bases -------- A list of base classes (either string names or CompoundNode objects).
      is_template -- True if the class is a template.

    (see also Target).
    """

    def __init__(self, xml, index, node):
        Target.__init__(self, xml, index, node)
        bases = []
        for base_xml in xml.findall("basecompoundref"):
            try:
                base = index.by_refid[base_xml.get("refid")]
            except KeyError:
                base = base_xml.text
            bases.append(base)
        self.bases = tuple(bases)
        self.is_template = (xml.find("templateparamlist") != None)

class Callable(Target):

    def parseParameters(self, xml, index, node):
        params = []
        for param_xml in xml.findall("param"):
            type_xml = param_xml.find("type")
            default_xml = param_xml.find("defval")
            if default_xml is not None:
                default = CxxType(default_xml, index)
            else:
                default = None
            param = Parameter(
                name=param_xml.findtext("declname"),
                cxxtype=CxxType(type_xml, index),
                default=default,
                brief=self.parseParagraphs(param_xml.find("briefdescription"), index, node)
                )
            if param.name is None:
                if param.cxxtype.template.strip() == "void":
                    continue
            params.append(param)
        self.is_template = (xml.find("templateparamlist") != None)
        return tuple(params)

    def __init__(self, xml, index, node):
        self.params = self.parseParameters(xml, index, node)
        self.is_template = (xml.find("templateparamlist") != None)
        Target.__init__(self, xml, index, node)

class Variable(Target):

    def __init__(self, xml, index, node):
        self.cxxtype = CxxType(xml.find("type"), index)
        Target.__init__(self, xml, index, node)

class Function(Callable):

    def __init__(self, xml, index, node):
        Callable.__init__(self, xml, index, node)
        self.cxxtype = CxxType(xml.find("type"), index)
        if self.cxxtype.template == "def":  # It's a pure-Python function!
            node.hidden = True

class Method(Function):

    def __init__(self, xml, index, node):
        Function.__init__(self, xml, index, node)
        self.is_const = (xml.get("const") == "yes")
        self.is_static = (xml.get("static") == "yes")
        self.is_reimplementation = (xml.find("reimplementation") is not None)

class Constructor(Callable):

    def __init__(self, xml, index, node):
        Callable.__init__(self, xml, index, node)

    def _is_default(self):
        return len(self.params) == 0
    is_default = property(_is_default)

class Enum(Target):

    def __init__(self, xml, index, node):
        Target.__init__(self, xml, index, node)
        self.values = []
        for value_xml in xml.findall("enumvalue"):
            value_node = index.by_refid[value_xml.get("id")]
            value_node.target = EnumValue(value_xml, index, value_node)
            self.values.append(value_node.target)
        
class EnumValue(Target):

    def __init__(self, xml, index, node):
        Target.__init__(self, xml, index, node)
        self.value = xml.findtext("initializer")

class Parameter(object):

    def __init__(self, name, cxxtype, default=None, brief="", direction=None):
        self.name = name
        self.cxxtype = cxxtype
        self.default = default
        self.brief = brief
        self.direction = direction
        if self.default:
            if self.default.template.strip() == "0" and self.cxxtype.is_pointer:
                self.default.template = "bp::object()"

class CxxType(object):

    def __init__(self, xml, index):
        terms = []
        self.dictionary = {}
        def add_text(s):
            if s is not None:
                u = s.strip()
                if u:
                    terms.append(u)
        add_text(xml.text)
        for child_xml in xml:
            if child_xml.tag == "ref":
                refid = child_xml.get("refid")
                try:
                    self.dictionary[refid] = index.by_refid[refid]
                    terms.append("".join(("{", refid, "}")))
                except KeyError:
                    logging.warning("Could not resolve '{0}' - scope may be incorrect (include "
                                    "additional xml directories to resolve this problem)"
                                    ".".format(child_xml.text))
                    terms.append(child_xml.text)
            else:
                add_text(child_xml.text)
            add_text(child_xml.tail)
        self.template = " ".join(terms)
        if "*" in self.template:
            self.is_pointer = True
        else:
            self.is_pointer = False
