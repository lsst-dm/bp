try:
    import xml.etree.cElementTree as ElementTree
except ImportError:
    import xml.etree.ElementTree as ElementTree
import targets
import os
import logging
from collections import OrderedDict

class OverloadSet(object):

    def __init__(self, name, parent):
        self.name = name
        self.parent = parent
        self.all = OrderedDict()
        self.visible = OrderedDict()

    def ignore(self, refid):
        self.all[refid].hidden = True
        try:
            del self.visible[refid]
        except KeyError:
            pass

    def add(self, member):
        self.all[member.refid] = member
        if not member.hidden:
            self.visible[member.refid] = member

    def get(self):
        if len(self.visible) != 1:
            raise LookupError("Multiple unresolved overloads of '{0}'.".format("::".join(self.name)))
        for member in self.visible.itervalues():
            return member

    def findone(self, label):
        result = None
        for member in self.all.itervalues():
            if member.label == label:
                if result is None:
                    result = member
                else:
                    raise LookupError(
                        "Identical labels '{0}' for overloads of '{1}'.".format(
                            label, "::".join(self.name)
                            )
                        )
        if result is None:
            raise LookupError(
                "Label '{0}' not found for overload '{1}'.".format(name, "::".join(self.name))
                )
        return result

    def findmany(self, labels):
        result = []
        found = set()
        for member in self.all.itervalues():
            if member.label in labels:
                result.append(member)
                found.add(member.label)
        for label in labels:
            if label not in found:
                raise LookupError(
                    "Label '{0}' not found for overload '{1}'.".format(label, "::".join(self.name))
                )
        return result

    def load(self, index):
        self.parent.load(index)

class Node(object):

    def __init__(self, name, kind, refid):
        self.name = name
        self.kind = kind
        self.refid = refid
        self.target = None
        self.hidden = False
        self.label = None

class MemberNode(Node):

    def __init__(self, name, kind, refid, parent):
        Node.__init__(self, name, kind, refid)
        self.parent = parent
        
    def load(self, index):
        if self.target is None:
            self.parent.load(index)
        if self.target is None:
            raise RuntimeError("Failed to load target for object '{0}'.".format("::".join(self.name)))

    def hide(self):
        self.parent.members[self.name[-1]].ignore(self.refid)

class CompoundNode(Node):

    def __init__(self, name, kind, refid, path):
        Node.__init__(self, name, kind, refid)
        self.xmlfile = os.path.join(path, "{0}.xml".format(self.refid))
        self.members = OrderedDict()

    def _get_visible(self):
        result = OrderedDict()
        for overloads in self.members.itervalues():
            for overload in overloads.visible.itervalues():
                result[overload.refid] = overload
        return result
    visible = property(_get_visible)

    def load(self, index):
        if self.target is not None:
            return
        if __debug__:
            logging.debug("Parsing xml target file '{0}'".format(self.xmlfile))
        xml_root = ElementTree.parse(self.xmlfile).getroot()
        for compound_xml in xml_root.findall("compounddef"):
            refid = compound_xml.get("id")
            compound_node = index.by_refid[refid]
            if compound_node.kind in ("class", "struct"):
                compound_node.target = targets.Class(compound_xml, index, compound_node)
                if __debug__:
                    logging.debug(
                        "  added {type} target '{name}'".format(
                            type=type(compound_node.target),
                            name="::".join(compound_node.name)
                            )
                        )
            for section_xml in compound_xml.findall("sectiondef"):
                for member_xml in section_xml.findall("memberdef"):
                    try:
                        member_node = index.by_refid[member_xml.get("id")]
                    except KeyError:
                        if __debug__:
                            logging.debug(
                                "    skipping member '{0}' ({1})".format(
                                    member_xml.findtext("name"),
                                    member_xml.get("id")
                                    )
                                )
                        continue
                    if member_node.kind == "function":
                        if isinstance(compound_node.target, targets.Class):
                            if member_node.name[-1] == member_node.name[-2]:
                                member_node.target = targets.Constructor(
                                    member_xml, index, member_node
                                    )
                            else:
                                member_node.target = targets.Method(
                                    member_xml, index, member_node
                                    )
                        else:
                            member_node.target = targets.Function(member_xml, index, member_node)
                    elif member_node.kind == "variable":
                        member_node.target = targets.Variable(member_xml, index, member_node)
                    elif member_node.kind == "enum":
                        member_node.target = targets.Enum(member_xml, index, member_node)
                    else:
                        member_node.hide()
                    if __debug__:
                        if member_node.target:
                            logging.debug(
                                "    added {type} member: '{name}'".format(
                                    type=type(member_node.target).__name__,
                                    name="::".join(member_node.name)
                                    )
                                )
                        else:
                            logging.debug(
                                "    skipping C++-only member: '{name}'.".format(
                                    name="::".join(member_node.name)
                                    )
                                )

class Index(object):

    def __init__(self, paths=()):
        self.by_name = {}
        self.by_refid = {}
        groups = []
        for path in paths:
            xmlfile = os.path.join(path, "index.xml")
            logging.debug("Parsing xml index file '{0}'.".format(xmlfile))
            root_xml = ElementTree.parse(xmlfile).getroot()
            for compound_xml in root_xml.findall("compound"):
                compound_node = CompoundNode(
                    name=tuple(compound_xml.findtext("name").split("::")),
                    kind=compound_xml.get("kind"),
                    refid=compound_xml.get("refid"),
                    path=path
                    )
                if compound_node.kind == "group":
                    groups.append((compound_node, compound_xml))
                    if __debug__:
                        logging.debug(
                            "  added group '{0}' ({1}).".format("::".join(compound_node.name), 
                                                                compound_node.refid)
                            )
                    continue
                if __debug__:
                    logging.debug(
                        "  added compound '{0}' ({1}).".format("::".join(compound_node.name),
                                                               compound_node.refid)
                        )
                self.by_name[compound_node.name] = compound_node
                self.by_refid[compound_node.refid] = compound_node
                for member_xml in compound_xml.findall("member"):
                    if compound_node.kind == "file":
                        member_name = (member_xml.findtext("name"),)
                    else:
                        member_name = compound_node.name + (member_xml.findtext("name"),)
                    member_node = MemberNode(
                        name=member_name,
                        kind=member_xml.get("kind"),
                        refid=member_xml.get("refid"),
                        parent=compound_node
                        )
                    if __debug__:
                        logging.debug(
                            "    added member '{0}' ({1}).".format(
                                "::".join(member_node.name),
                                member_node.refid
                                )
                            )
                    if member_node.name[-1] in compound_node.members:
                        compound_node.members[member_node.name[-1]].add(member_node)
                    else:
                        overloads = OverloadSet(member_node.name, parent=compound_node)
                        overloads.add(member_node)
                        compound_node.members[member_node.name[-1]] = overloads
                        self.by_name[overloads.name] = overloads
                    self.by_refid[member_node.refid] = member_node
        for compound_node, compound_xml in groups:
            for member_xml in compound_xml.findall("member"):
                member_refid = member_xml.get("refid")
                member_node = self.by_refid[member_refid]
                member_node.parent = compound_node
                if __debug__:
                    logging.debug(
                        "    reset parent of member '{0}' ({1}) to group {2}.".format(
                            "::".join(member_node.name), member_node.refid, compound_node.name[-1]
                            )
                        )

    def lookup(self, name, scope=()):
        end = tuple(name)
        start = tuple(scope)
        node = None
        n = len(start)
        while n >= 0:
            node = self.by_name.get(start[:n] + end, None)
            if node is not None:
                node.load(self)
                return node
            n -= 1
        raise KeyError(
            "Name '{name}' not found in scope '{scope}'.".format(name="::".join(name), scope="::".join(scope))
            )
