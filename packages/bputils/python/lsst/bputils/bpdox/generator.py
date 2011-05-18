import re
import sys
import logging
from . import formatter
from . import lookup
from . import targets

def discard(d, k):
    try:
        del d[k]
    except KeyError:
        pass

class Generator(object):

    macro_regex = re.compile(r"%%(?P<name>\w+)\s*(?P<body>[^%]*)\s*%%")
    scope_regex = re.compile(r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*\)\s*")
    in_class_regex = re.compile(r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*(?P<tparams><.*>)?\s*\)\s*")
    doc_regex = re.compile(r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*(\[(?P<label>\w+)\])?\s*\)\s*")
    auto_class_regex = re.compile(
        r"\s*<\s*(?P<name>(\w+::)*\w+)\s*(?P<tparams><.*>)?\s*(,\s*(?P<nc>(boost::)?noncopyable))?\s*>"
        r"(\s+(?P<variable>\w+))?\s*(\((?P<init>.*)\))?"
        )
    auto_method_regex = re.compile(
        r"\s*\((?P<name>\w+)"
        r"(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?"
        r"(\s*,\s*(?P<policies>.*))?\s*\)\s*"
        )
    auto_method_rename_regex = re.compile(
        r"\s*\((?P<name>\w+)"
        r"(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?"
        r"\s*,\s*(?P<pyname>\w+)"
        r"(\s*,\s*(?P<policies>.*))?\s*\)\s*"
        )
    auto_function_regex = re.compile(
        r"\s*\((?P<name>(\w+::)*\w+)"
        r"(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?"
        r"(\s*,\s*(?P<policies>.*))?\s*\)\s*"
        )
    auto_init_regex = re.compile(r"\s*(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?\s*")
    auto_enum_regex = re.compile(r"\s*\((?P<name>(\w+::)*\w+)(\s*,\s*(?P<tscope>\S.+\S))?\s*\)")            
    members_list_regex = re.compile(r"\s*\(\s*(?P<names>\w+\s*(,\s*\w+)*)\s*\)\s*")
    members_xpr_regex = re.compile(r"\s*\((?P<xpr>.*)\)\s*")
    
    def __init__(self, formatter, index):
        self.formatter = formatter
        self.index = index
        self._scope = ()
        self._class = None
        self._class_tparams = None
        self._class_members_todo = None

    def __call__(self, input, output):
        for n, line in enumerate(input):
            s = line
            while s:
                start = s.find("%%")
                if start < 0:
                    output.write(s)
                    break
                stop = s.find("%%", start + 2)
                if stop < 0:
                    raise SyntaxError("Unterminated macro on line {n}.".format(n=n+1))
                stop += 2
                head = s[:start]
                tail = s[stop:]
                macro_match = self.macro_regex.match(s, start, stop)
                if not macro_match:
                    raise SyntaxError("Syntax error on line {n}: '{s}'.".format(n=n+1, s=s[start:stop]))
                try:
                    method = getattr(self, "m_{0}".format(macro_match.group("name")))
                except AttributeError:
                    raise SyntaxError(
                        "Undefined macro '{macro}' on line {n}.".format(
                            macro=macro_match.group("name"), n=n+1
                            )
                        )
                try:
                    macro_output = method(macro_match.group("body"), indent=(" " * start))
                except Exception, err:
                    import traceback
                    newErr = type(err)(("{0} (line {1})".format(err.args[0], n+1), traceback.format_exc()))
                    raise newErr
                if head:
                    output.write(head)
                if macro_output:
                    output.write(macro_output)
                s = tail

    def auto_members(self, names, indent):
        class_type = formatter.formatName(self._class, scope=self._scope)
        if self._class_tparams:
            class_type += self._class_tparams
        sequence = []
        names_done = set()
        for name in names:
            if name in names_done: continue
            sequence.extend(self._class.members[name].visible.itervalues())
            names_done.add(name)
        results = []
        static_methods = set()
        for node in sequence:
            if node.target is None:
                logging.warning("Skipping member '{0}' ({1}).".format("::".join(node.name), node.refid))
                continue
            wrappers = node.target.format(self.formatter, scope=self._scope, 
                                          class_type=class_type, indent=indent)
            results.extend(wrappers)
            if wrappers:
                logging.debug("Generating auto for '{0}' ({1})".format("::".join(node.name), node.refid))
            if isinstance(node.target, targets.Method) and node.target.is_static:
                static_methods.add(node.target.name[-1])
            discard(self._class_members_todo, node.refid)
        for name in static_methods:
            results.append('staticmethod("{0}")'.format(name))
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)    

    def ignore_members(self, names, indent):
        for name in names:
            for refid in self._class.members[name].all:
                discard(self._class_members_todo, refid)

    def m_scope(self, body, indent):
        match = self.scope_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing scope argument.")
        self._scope = tuple(match.group("name").split("::"))
        return ""

    def m_doc(self, body, indent):
        match = self.doc_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing doc argument.")
        node = self.index.lookup(match.group("name").split("::"), scope=self._scope)
        if isinstance(node, lookup.OverloadSet):
            label = match.group("label")
            if label is None:
                node = node.get()
            else:
                node = node.findone(label)
        return self.formatter.getDocumentation(node.target, scope=self._scope, indent=indent)

    def m_in_class(self, body, indent):
        match = self.in_class_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing in_class argument.")
        node = self.index.lookup(match.group("name").split("::"), scope=self._scope)
        self._class = node
        self._class_tparams = match.group("tparams")
        self._class_members_todo = self._class.visible
        return ""

    def m_auto_class(self, body, indent):
        match = self.auto_class_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_class argument.")
        node = self.index.lookup(match.group("name").split("::"), scope=self._scope)
        noncopyable = match.group("nc") != None
        variable = match.group("variable")
        tparams = match.group("tparams")
        init = match.group("init")
        if not init: init = None
        self._class = node
        self._class_tparams = tparams
        self._class_members_todo = self._class.visible
        return self.formatter.getClassDeclaration(
            node.target, scope=self._scope, indent=indent, noncopyable=noncopyable,
            variable=variable, tparams=tparams, init=init
            )

    def m_auto_method(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_method invocation point.")
        match = self.auto_method_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_method argument.")
        class_type = formatter.formatName(self._class, scope=self._scope)
        if self._class_tparams:
            class_type += self._class_tparams
        method_name = match.group("name").split("::")
        overloads = self.index.lookup(method_name, scope=self._class.name)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads.visible.values()
        results = []
        is_static = False
        for node in sequence:
            if node.target.is_static:
                is_static = True
            results.append(
                self.formatter.getMethodDeclaration(
                    node.target, scope=self._scope, indent=indent,
                    call_policies=match.group("policies"), class_type=class_type
                    )
                )
            discard(self._class_members_todo, node.refid)
        if is_static:
            results.append('staticmethod("{0}")'.format(method_name[-1]))
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_method_rename(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_method_rename invocation point.")
        match = self.auto_method_rename_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_method_regex argument.")
        class_type = formatter.formatName(self._class, scope=self._scope)
        if self._class_tparams:
            class_type += self._class_tparams
        method_name = match.group("name").split("::")
        overloads = self.index.lookup(method_name, scope=self._class.name)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads.visible.values()
        results = []
        is_static = False
        for node in sequence:
            if node.target.is_static:
                is_static = True
            results.append(
                self.formatter.getMethodDeclaration(
                    node.target, scope=self._scope, indent=indent, name=match.group("pyname"),
                    call_policies=match.group("policies"), class_type=class_type
                    )
                )
            discard(self._class_members_todo, node.refid)
        if is_static:
            results.append('staticmethod("{0}")'.format(method_name[-1]))
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_init(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_init invocation point.")
        match = self.auto_init_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_method argument.")
        overloads = self.index.lookup(self._class.name + self._class.name[-1:], scope=self._class.name)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads.visible.values()
        results = []
        for node in sequence:
            results.append(
                self.formatter.getInitDeclaration(node.target, scope=self._scope, indent=indent)
                )
            discard(self._class_members_todo, node.refid)
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_function(self, body, indent):
        match = self.auto_function_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_function argument.")
        function_name = match.group("name").split("::")
        overloads = self.index.lookup(function_name, scope=self._scope)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads.visible.values()
        results = []
        for node in sequence:
            results.append(
                self.formatter.getFunctionDeclaration(
                    node.target, scope=self._scope, indent=indent,
                    call_policies=match.group("policies")
                    )
                )
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_members(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_members invocation point.")
        match = self.members_list_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_members argument.")
        names = [name.strip() for name in match.group("names").split(",")]
        return self.auto_members(names, indent)

    def m_ignore_members(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at ignore_members invocation point.")
        match = self.members_list_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing ignore_members argument.")
        names = [name.strip() for name in match.group("names").split(",")]
        self.ignore_members(names, indent)
        return ""

    def m_auto_members_regex(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_members_regex invocation point.")
        match = self.members_xpr_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_members_regex argument.")
        regex = re.compile(match.group("xpr"))
        names = []
        for overload in self._class_members_todo.itervalues():
            if regex.match(overload.name[-1]):
                names.append(overload.name[-1])
        return self.auto_members(names, indent)

    def m_ignore_members_regex(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at ignore_members_regex invocation point.")
        match = self.members_xpr_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing ignore_members_regex argument.")
        regex = re.compile(match.group("xpr"))
        names = []
        for overload in self._class_members_todo.itervalues():
            if regex.match(overload.name[-1]):
                names.append(overload.name[-1])
        self.ignore_members(names, indent)
        return ""

    def m_auto_enum(self, body, indent):
        match = self.auto_enum_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_enum argument.")
        enum_name = match.group("name").split("::")
        node = self.index.lookup(enum_name, scope=self._scope)
        if isinstance(node, lookup.OverloadSet):
            node = node.get()
        if self._class is not None:
            discard(self._class_members_todo, node.refid)
        return self.formatter.getEnum(
            node.target, scope=self._scope, tscope=match.group("tscope"), indent=indent
            )

    def m_finish_class(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at finish_class invocation point.")
        for node in self._class_members_todo.itervalues():
            logging.warning("{0} not wrapped.".format("::".join(node.name)))
        self._class = None
        self._class_tparams = None
        self._class_members_todo = None
        return ""
