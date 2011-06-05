from ._bpdox import Macro, BlockMacro, SimpleMacro, OptionType
from . import settings

registry = []

def register(cls):
    """Decorator that adds a class to the macro registry."""
    registry.append(cls)
    return cls

def addSharedCompoundOptions(self):
    self.addOption(name="include_regex", type=OptionType.STRING, default=settings.default_include_regex,
                   doc="auto-generate wrappers for all members matching the given regular expression")
    self.addOption(name="exclude_regex", type=OptionType.STRING, default=settings.default_exclude_regex,
                   doc="do not generate wrappers for all members matching the given regular expression")
    self.addOption(name="include_list", type=OptionType.LIST, default=(),
                   doc="auto-generate wrappers for all members in the given list")
    self.addOption(name="exclude_list", type=OptionType.LIST, default=(),
                   doc="do not generate wrappers for all members in the given list")
    
def addSharedClassOptions(self):
    self.addOption(name="target", type=OptionType.REF, default=None, doc="target C++ class to wrap")
    self.addOption(name="doc", type=OptionType.CODE, default=None, doc="Python docstring")
    self.addOption(name="const_aware", type=OptionType.BOOL, default=True,
                   doc="use const_aware::exposer instead of class_")
    self.addOption(name="noncopyable", type=OptionType.BOOL, default=False,
                   doc="disable by-value returns (necessary for pure abstract classes")
    self.addOption(name="enable_shared_ptr", type=OptionType.BOOL, default=True,
                   doc="enable return by shared_ptr for the class")
    self.addOption(name="bases", type=OptionType.CODE, default=None,
                   doc="comma-separated list of base classes")
    addSharedCompoundOptions(self)

def addSharedMemberOptions(self):
    self.addOption(name="pyname", type=OptionType.STRING, default=None, 
                   doc="Python name for the wrapped object")
    self.addOption(name="doc", type=OptionType.CODE, default=None, doc="Python docstring")

def addSharedCallableOptions(self):
    addSharedMemberOptions(self)
    self.addOption(name="tparams", type=OptionType.CODE, default=None, 
                   doc="template parameters; only valid when target is a template")
    self.addOption(name="policies", type=OptionType.CODE, default=None, doc="Boost.Python call policies")
    self.addOption(name="pointer", type=OptionType.CODE, default=None,
                   doc="C++ function, member, or member function pointer")
    self.addOption(name="args", type=OptionType.CODE, default=None, 
                   doc="sequence of bp::arg calls to specify keyword arguments")

@register
class Customize(BlockMacro):
    """Generate a "customize" function for use with the Class and TemplateClass macros.
    """

    def __init__(self):
        BlockMacro.__init__(self, type(self).__name__)

    def apply(self, indent, options, processor):
        if not processor.getActiveClass():
            return "namespace {\ntemplate <typename Wrapper> void customize(Wrapper & wrapper) {"
        else:
            return "template <typename Wrapper> static void customize(Wrapper & wrapper) {"

    def finish(self, indent, options, processor):
        if not processor.getActiveClass():
            return "}\n} // <anonymous"
        else:
            return "}"

@register
class Namespace(BlockMacro):
    """Generate a C++ namespace block and set the scope for Doxygen lookups.
    
    Nesting @Namespace calls is not permitted, but a single namespace call
    can include multiple nested namespaces, delimited by '::'.  This will
    expand to a sequence of C++ namespace calls, and will be correctly closed
    by multiple braces.  Unless the option 'anonymous' is set to False,
    and innermost anonymous namespace will be added.  For example:

    @Namespace(foo::bar) {
    /* body */
    }

    will expand to:

    namespace foo { namespace bar { namespace {
    /* body */
    }}} // namespace foo::bar::<anonymous>

    """

    def __init__(self):
        BlockMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None,
                       doc="fully-qualified C++ namespace")
        self.addOption(name="anonymous", type=OptionType.BOOL, default=True,
                       doc="if True (the default), add an anonymous namespace inside the target namespace")

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Namespace macro requires a target argument.")
        if processor.getActiveNamespace():
            raise RuntimeError("Nested @Namespace macros not allowed.")
        target = processor.lookup(options['target'], no_overloads=True)
        terms = []
        for name in target.lscope:
            terms.append("namespace %s {" % name)
        if options["anonymous"]:
            terms.append("namespace {")
        processor.setNamespace(target)
        return indent + " ".join(terms)

    def finish(self, indent, options, processor):
        target = processor.getActiveNamespace()
        assert(target)
        terms = list(target.lscope)
        if options["anonymous"]:
            terms.append("<anonymous>")
        processor.setNamespace(None)
        return "%s%s // %s" % (indent, "}" * len(terms), "::".join(terms))

@register
class Class(BlockMacro):
    """Generate a Boost.Python wrapper for a C++ class.

    This macro expands to a struct definition containing a single static member function
    ("declare()") that builds a Boost.Python wrapper for the class.  For instance, the macro:
    
    @Class(Foo, const_aware=False) {
    /* body */
    };

    will expand to (roughly):

    struct PyFoo {
        /* body */
        static void declare() {
            bp::class_< Foo > wrapper("Foo", bp::no_init);
            bp::scope scope(wrapper);
            wrapper.enable_shared_ptr();
            /* automatic member wrapper declarations */
            customize(wrapper);
            /* staticmethod calls */
        }
    };

    The unqualified call to "customize(wrapper)" at the end allows the user to add their own
    custom Boost.Python code or more finely-grained bpdox macros to the class wrapper
    by adding a template static member function with of the form

    template <typename Wrapper> static void customize(Wrapper & wrapper);

    This call will generate an error if no customize member function is defined, but the user
    can use the @DefaultCustomize macro to define a non-member customize function at the top of each
    bpdox source file that will be invoked when no customize member function exists.  Note that
    a bp::scope object for the class is active when customize is called, so any non-member
    Boost.Python wrappers (such as enum_ or nested classes) will automatically appear within
    the class.  Also note that automatically-generated staticmethod() calls will happen
    after the custom wrappers, so some overloads of a static member function can be customized
    while others are automatic.

    @Class and @TemplateClass calls may be nested, and generally should be to wrap nested classes.

    The exact list of members that will have automatic wrappers generated for them is given by
    (((matches include_regex) and not (matches exclude_regex) and not (in exclude list)) 
    or (in include_list)).
    """

    def __init__(self):
        BlockMacro.__init__(self, type(self).__name__)
        addSharedClassOptions(self)
        self.addOption(name="pyname", type=OptionType.STRING, default=None,
                       doc="Python name for the class (default is the same as the C++ class)")

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Class macro requires a target argument.")
        target = processor.lookup(options['target'], no_overloads=True)
        processor.pushClass(target)
        return "struct Py{name} {{".format(name=target.name)

    def finish(self, indent, options, processor):
        target = processor.getActiveClass()
        declaration = target.formatDeclare(processor, indent, accept_pyname=False, **options)
        processor.popClass()
        return "{0}\n{indent}}}".format(declaration, indent=indent)

@register
class Exception(BlockMacro):
    """Generate a Boost.Python wrapper for a C++ exception.

    For more information, see the documentation for @Class.
    """

    def __init__(self):
        BlockMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None, doc="target C++ class to wrap")
        addSharedCompoundOptions(self)
        self.addOption(name="pyname", type=OptionType.STRING, default=None,
                       doc="Python name for the class (default is the same as the C++ class)")

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Exception macro requires a target argument.")
        target = processor.lookup(options['target'], no_overloads=True)
        processor.pushClass(target)
        return "struct Py{name} {{".format(name=target.name)

    def finish(self, indent, options, processor):
        target = processor.getActiveClass()
        options["const_aware"] = False
        options["enable_shared_ptr"] = False
        options["noncopyable"] = False
        builder = ' = lsst::bputils::declareException< {name}, {bases[0]} >(\n'\
            '{indent3}"{pyname}",\n{indent3}{doc}\n{indent2})'
        declaration = target.formatDeclare(processor, indent, builder=builder, **options)
        processor.popClass()
        return "{0}\n{indent}}}".format(declaration, indent=indent)

@register
class TemplateClass(BlockMacro):
    """Generate a Boost.Python wrapper for a C++ template class.

    This macro expands to a template struct definition containing a single static member function
    ("declare()") that builds a Boost.Python wrapper for the class.  For instance, the macro:
    
    template <typename T, int N>
    @TemplateClass(Foo, {<T,N>}, const_aware=False) {
    /* body */
    };

    will expand to (roughly):

    template <typename T, int N>
    struct PyFoo<T,N> {
        typedef Foo<T,N> Wrapped;
        /* body */
        static void declare(std::string const & name) {
            bp::class_< Foo<T,N> > wrapper(name, bp::no_init);
            bp::scope scope(wrapper);
            wrapper.enable_shared_ptr();
            /* automatic member wrapper declarations */
            customize(wrapper);
            /* staticmethod calls */
        }
    };

    Note that the macro does not generate the template parameter list; this should be included
    explicitly by the user before the macro call for correct results.  Also, unlike @Class,
    the declare() static member function takes a string argument that sets the Python name
    of the class; this should be different for different template instantiations.

    For more information, see the documentation for @Class.
    """

    def __init__(self):
        BlockMacro.__init__(self, type(self).__name__)
        addSharedClassOptions(self)
        self.addOption(name="tparams", type=OptionType.CODE, default=None,
                       doc="template parameters; only valid when target is a template")
        self.reorderOptions(["target", "tparams"])

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@TemplateClass macro requires a target argument.")
        if not options['tparams']:
            raise ValueError("@TemplateClass macro requires a tparams argument.")
        target = processor.lookup(options['target'], no_overloads=True)
        processor.pushClass(target, options['tparams'])
        return "struct Py{name} {{".format(name=target.name)

    def finish(self, indent, options, processor):
        target = processor.getActiveClass()
        declaration = target.formatDeclare(processor, indent, accept_pyname=True, **options)
        processor.popClass()
        return "{0}\n{indent}}}".format(declaration, indent=indent)

@register
class Function(SimpleMacro):
    """Generate a Boost.Python wrapper for a (possibly overloaded) C++ free-function."""

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None, doc="C++ object to be wrapped")
        addSharedCallableOptions(self)

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Function macro requires a target argument.")
        terms = []
        for target in processor.lookup(options['target'], iterate=True):
            body = target.formatFunction(processor, indent, **options)
            terms.append("{bp}::{body}".format(bp=settings.bp, body=body))
        return ";\n{0}".format(indent).join(terms)

@register
class Member(SimpleMacro):

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None, doc="C++ object to be wrapped")
        addSharedCallableOptions(self)

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Member macro requires a target argument.")
        terms = []
        for target in processor.lookup(options['target'], iterate=True):
            processor.getActiveCustomizedSet().add(target.refid)
            terms.append(target.formatMember(processor, indent, **options))
        return "\n{0}.".format(indent[:-1]).join(terms)

@register
class MemberList(SimpleMacro):

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.LIST, default=None, doc="C++ objects to be wrapped")
        addSharedCallableOptions(self)

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@MemberList macro requires a target argument.")
        terms = []
        for target in processor.getActiveClass().members(include_list=options["target"]):
            processor.getActiveCustomizedSet().add(target.refid)
            terms.append(target.formatMember(processor, indent, **options))
        return "\n{0}.".format(indent[:-1]).join(terms)

@register
class MemberRegex(SimpleMacro):

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.STRING, default=None,
                       doc="Regular expression for C++ objects to be wrapped")
        addSharedCallableOptions(self)

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Member macro requires a target argument.")
        terms = []
        for target in processor.getActiveClass().members(include_regex=options["target"]):
            processor.getActiveCustomizedSet().add(target.refid)
            terms.append(target.formatMember(processor, indent, **options))
        return "\n{0}.".format(indent[:-1]).join(terms)

@register
class Enum(SimpleMacro):

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None, doc="C++ object to be wrapped")
        addSharedMemberOptions(self)
        self.addOption(name="export_values", type=OptionType.BOOL, default=True,
                       doc="if True (default) add enum values to the enum's enclosing scope")

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Enum macro requires a target argument.")
        target = processor.lookup(options['target'], no_overloads=True)
        return target.formatEnum(processor, indent, **options)

@register
class Doc(SimpleMacro):

    def __init__(self):
        SimpleMacro.__init__(self, type(self).__name__)
        self.addOption(name="target", type=OptionType.REF, default=None, 
                       doc="C++ object get documentation for")

    def apply(self, indent, options, processor):
        if not options['target']:
            raise ValueError("@Doc macro requires a target argument.")
        terms = []
        target = processor.lookup(options['target'], no_overloads=True)
        return processor.formatDocumentation(target, indent)
