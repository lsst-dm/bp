import logging
from collections import OrderedDict
from . import settings
import re

labeled_regex = re.compile(r"(?P<name>\w+(::\w+)*)(\[\s*(?P<labels>\w+(\s*,\s*\w+)*)\s*\])?")

class OverloadSet(object):

    __slots__ = "all", "visible", "lscope"

    auto_labels = {
        "const": lambda node: node.is_const,
        "nonconst": lambda node: not node.is_const,
        }

    def __init__(self, *nodes):
        self.all = OrderedDict()
        self.visible = OrderedDict()
        self.lscope = ()
        for node in nodes:
            self.add(node)

    def __str__(self):
        return "::".join(self.lscope)

    __repr__ = __str__
    
    def add(self, node):
        self.all[node.refid] = node
        if not node.is_hidden:
            self.visible[node.refid] = node
        node.overloads = self

    def remove(self, node):
        del self.all[node.refid]
        if not node.is_hidden:
            del self.visible[node.refid]
        node.overloads = None

    def get(self, label=None):
        if label is None:
            if len(self.visible) != 1:
                raise LookupError("Unresolved overloads in lookup of {0}".format("::".join(self.lscope)))
            return self.visible.itervalues().next()
        if label in self.auto_labels:
            check = self.auto_labels[label]
        else:
            check = lambda x: False
        for node in self.all.itervalues():
            if node.label == label or check(node):
                return node
        raise LookupError("Name {0} with label {1} not found".format("::".join(self.lscope), label))

    def filtered(self, labels=None):
        if labels is None:
            return self
        result = OverloadSet()
        result.lscope = self.lscope
        for node in self.iterate(labels):
            result.add(node)
        return result

    def iterate(self, labels=None):
        if isinstance(labels, basestring):
            labels = [label.strip() for label in m.group("labeled").split(",")]
        if labels is None:
            for node in self.visible.itervalues():
                yield node
        else:
            labels = set(labels)
            for node in self.all.itervalues():
                if node.label in labels:
                    labels.remove(node.label)
                    yield node
            for label in labels:
                check = self.auto_labels.get(label, None)
                if check:
                    for node in self.all.itervalues():
                        if check(node):
                            yield node

    def load(self, index):
        for node in self.all.itervalues():
            node.load(index)

class Ref(object):

    __slots__ = "refid", "has_tparams", "has_nested", "target", "text"

    def __init__(self, xml, index):
        self.refid = xml.get("refid")
        self.has_tparams = xml.tail and xml.tail.strip().startswith("<")
        self.has_nested = xml.tail and "::" in xml.tail
        self.text = xml.text
        try:
            self.target = index.by_refid[self.refid]
        except KeyError:
            self.target = None

class Parameter(object):

    __slots__ = "name", "cxxtype", "detault", "brief", "direction", "default"

    def __init__(self, name, cxxtype, default=None, brief="", direction=None):
        self.name = name
        self.cxxtype = cxxtype
        self.default = default
        self.brief = brief
        self.direction = direction
        if self.default:
            if self.default.template.strip() == "0" and self.cxxtype.is_pointer:
                self.default.template = "{bp}::object()".format(bp=settings.bp)

class CxxType(object):

    __slots__ = "refs", "template", "is_pointer"

    def __init__(self, xml, index):
        if xml is None: return None
        text = []
        self.refs = []
        def add_text(s):
            if s is not None:
                u = s.strip()
                if u:
                    text.append(u)
        add_text(xml.text)
        for child_xml in xml:
            if child_xml.tag == "ref":
                ref = Ref(child_xml, index)
                text.append("{%d}" % len(self.refs))
                self.refs.append(ref)
            else:
                add_text(child_xml.text)
            add_text(child_xml.tail)
        self.template = " ".join(text)
        if "*" in self.template:
            self.is_pointer = True
        else:
            self.is_pointer = False
