import weakref
from xml.etree.ElementTree import tostring

class Target(object):
    """A base class for target nodes in the Doxygen XML.

    Attributes:
      brief ----- Doxygen "brief" description.
      detailed -- Doxygen "detailed" description.
    """

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
                        description = self.parseParagraphs(parameteritem.find("parameterdescription"), index)
                        for parametername in namelist.findall("parametername"):
                            for param in self.params:
                                if param.name == parametername.text:
                                    param.direction = parametername.get("direction")
                                    param.brief = description
                                    break
                if child.text is not None: terms.append(child.text)
                if child.tail is not None: terms.append(child.tail)
            paragraphs.append("".join(terms))
        return "\n\n".join(paragraphs).strip()

    def __init__(self, xml, index, name):
        self.brief = self.parseParagraphs(xml.find("briefdescription"), index)
        self.detailed = self.parseParagraphs(xml.find("detaileddescription"), index)
        self.name = name

class Class(Target):
    """A Target subclass for classes and structs.

    Attributes:
      bases -------- A list of base classes (either string names or CompoundNode objects).
      is_template -- True if the class is a template.

    (see also Target).
    """

    def __init__(self, xml, index, name):
        Target.__init__(self, xml, index, name)
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

    def parseParameters(self, xml, index):
        params = []
        for param_xml in xml.findall("param"):
            type_xml = param_xml.find("type")
            param = Parameter(
                name=param_xml.findtext("declname"),
                cxxtype=CxxType(type_xml, index),
                default=param_xml.findtext("defval"),
                brief=self.parseParagraphs(param_xml.find("briefdescription"), index)
                )
            params.append(param)
        self.is_template = (xml.find("templateparamlist") != None)
        return tuple(params)

    def __init__(self, xml, index, name):
        self.params = self.parseParameters(xml, index)
        self.is_template = (xml.find("templateparamlist") != None)
        Target.__init__(self, xml, index, name)

class Variable(Target):

    def __init__(self, xml, index, name):
        self.cxxtype = CxxType(xml.find("type"), index)
        Target.__init__(self, xml, index, name)

class Function(Callable):

    def __init__(self, xml, index, name):
        Callable.__init__(self, xml, index, name)
        self.cxxtype = CxxType(xml.find("type"), index)

class Method(Function):

    def __init__(self, xml, index, name):
        Function.__init__(self, xml, index, name)
        self.is_const = (xml.get("const") == "yes")
        self.is_static = (xml.get("static") == "yes")
        self.is_reimplementation = (xml.find("reimplementation") is not None)

class Constructor(Callable):

    def __init__(self, xml, index, name):
        Callable.__init__(self, xml, index, name)

    def _is_default(self):
        return len(self.params) == 0
    is_default = property(_is_default)

class Enum(Target):

    def __init__(self, xml, index, name):
        Target.__init__(self, xml, index, name)
        self.values = []
        for value_xml in xml.findall("enumvalue"):
            value_node = index.by_refid[value_xml.get("id")]
            value_node.target = EnumValue(value_xml, index, value_node.name)
            self.values.append(value_node.target)
        
class EnumValue(Target):

    def __init__(self, xml, index, name):
        Target.__init__(self, xml, index, name)
        self.value = xml.findtext("initializer")

class Parameter(object):

    def __init__(self, name, cxxtype, default=None, brief="", direction=None):
        self.name = name
        self.cxxtype = cxxtype
        self.default = default
        self.brief = brief
        self.direction = direction

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
                self.dictionary[refid] = index.by_refid[refid]
                terms.append("".join(("{", refid, "}")))
            else:
                add_text(child_xml.text)
            add_text(child_xml.tail)
        self.template = " ".join(terms)
