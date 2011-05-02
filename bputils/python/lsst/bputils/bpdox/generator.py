import re
import sys
from . import formatter
from . import lookup

class Generator(object):

    macro_regex = re.compile(r"%%(?P<name>\w+)\s*(?P<body>[^%]*)\s*%%")
    scope_regex = re.compile(r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*\)\s*")
    in_class_regex = re.compile(r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*\)\s*(?P<tparams><.*?>)?\s*")
    doc_regex = re.compile(
        r"\s*\(\s*(?P<name>(\w+::)*\w+)\s*(\[(?P<label>\w+)\])?\s*\)\s*"
        )
    auto_class_regex = re.compile(
        r"\s*<\s*(?P<name>(\w+::)*\w+)\s*(?P<tparams><.*?>)?\s*(,\s*(?P<nc>(boost::)?noncopyable))?\s*>"
        r"(\s+(?P<variable>\w+))?\s*(\((?P<init>.*)\))?"
        )
    auto_method_regex = re.compile(
        r"\s*\((?P<name>(\w+::)*\w+)"
        r"(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?"
        r"(\s*,\s*(?P<policies>.*))?\s*\)\s*"
        )
    auto_function_regex = auto_method_regex
    auto_init_regex = re.compile(r"\s*(\[(?P<labels>\s*\w+\s*(,\s*\w+\s*)*)\])?\s*")
    auto_enum_regex = re.compile(r"\s*\((?P<name>(\w+::)*\w+)(\s*,\s*(?P<tscope>\S.+\S))?\s*\)")        

    def __init__(self, formatter, index):
        self.formatter = formatter
        self.index = index
        self._scope = ()
        self._class = None
        self._class_tparams = None

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

    def m_scope(self, body, indent):
        match = self.scope_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing scope argument.")
        self._scope = tuple(match.group("name").split("::"))
        return ""

    def m_doc(self, body, indent):
        match = self.doc_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing %%doc%% argument.")
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
            raise SyntaxError("Error parsing scope argument.")
        node = self.index.lookup(match.group("name").split("::"), scope=self._scope)
        self._class = node
        self._class_tparams = match.group("tparams")
        return ""

    def m_auto_class(self, body, indent):
        match = self.auto_class_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing class argument.")
        node = self.index.lookup(match.group("name").split("::"), scope=self._scope)
        noncopyable = match.group("nc") != None
        variable = match.group("variable")
        tparams = match.group("tparams")
        init = match.group("init")
        if not init: init = None
        self._class = node
        self._class_tparams = tparams
        return self.formatter.getClassDeclaration(
            node.target, scope=self._scope, indent=indent, noncopyable=noncopyable,
            variable=variable, tparams=tparams, init=init
            )

    def m_auto_method(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at method invokation point.")
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
            sequence = overloads
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
        if is_static:
            results.append(
                'staticmethod("{name}")'.format(
                    name=formatter.formatName(method_name[-1], scope=self._class.name)
                    )
                )
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_init(self, body, indent):
        if self._class is None:
            raise RuntimeError("No class active at auto_init invokation point.")
        match = self.auto_init_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_method argument.")
        overloads = self.index.lookup(self._class.name + self._class.name[-1:], scope=self._class.name)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads
        results = []
        for node in sequence:
            results.append(
                self.formatter.getInitDeclaration(node.target, scope=self._scope, indent=indent)
                )
        if len(results) == 0:
            raise LookupError("Empty macro result.")
        return "\n{indent}.".format(indent=indent).join(results)

    def m_auto_function(self, body, indent):
        match = self.auto_function_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_function argument.".format(n=n))
        function_name = match.group("name").split("::")
        overloads = self.index.lookup(function_name, scope=self._scope)
        if match.group("labels"):
            labels = [label.strip() for label in match.group("labels").split(",")]
            sequence = overloads.findmany(labels)
        else:
            sequence = overloads
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

    def m_auto_enum(self, body, indent):
        match = self.auto_enum_regex.match(body)
        if not match:
            raise SyntaxError("Error parsing auto_enum argument.")
        enum_name = match.group("name").split("::")
        node = self.index.lookup(enum_name, scope=self._scope)
        if isinstance(node, lookup.OverloadSet):
            node = node.get()
        return self.formatter.getEnum(
            node.target, scope=self._scope, tscope=match.group("tscope"), indent=indent
            )
