from ._bpdox import BlockMacro, SimpleMacro, OptionType

class DefaultCustom(BlockMacro):
    """Generate a default "custom" function for use with the Class and TemplateClass macros.

    This should generally be used outside any namespace scope, as the macro will expand
    to define its own anonymous namespace:

    @DefaultCustom { /* body */ }

    will expand to:

    namespace {
    template <typename Wrapper> void custom(Wrapper & wrapper) { /* body */ }
    } // <anonymous>
    """

    def __init__(self, name="DefaultCustom"):
        BlockMacro.__init__(self, name)

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

    def __init__(self, name="Namespace"):
        BlockMacro.__init__(self, name)
        self.addOption(name="target", type=OptionType.REF, default=None,
                       doc="fully-qualified C++ namespace")
        self.addOption(name="anonymous", type=OptionType.BOOL, default=True,
                       doc="if True (the default), add an anonymous namespace inside the target namespace")

    def apply(self, options, state):
        raise NotImplementedError()

    def finish(self, options, state):
        raise NotImplementedError()

class Class(BlockMacro):
    """Generate the skeleton of a C++ class wrapper.

    This macro expands to a struct definition containing a single static member function
    ("declare()") that builds a Boost.Python wrapper for the class.  For instance, the macro:
    
    @Class(Foo, const_aware=False) {
    /* body */
    };

    will expand to (roughly):

    struct PyFoo {
        static void declare() {
            bp::class_< Foo > wrapper("Foo", bp::no_init);
            wrapper.enable_shared_ptr();
            /* additional member wrapper declarations */
            custom(wrapper);
        }
        /* body */
    };

    The unqualified call to "custom(wrapper)" at the end allows the user to add their own
    custom Boost.Python code or more finely-grained bpdox macros to the class wrapper
    by adding a template static member function with of the form

    template <typename Wrapper> static void custom(Wrapper & wrapper);

    This call will generate an error if no custom member function is defined, but the user
    can use the @DefaultCustom macro to define a non-member custom function at the top of each
    bpdox source file that will be invoked when no custom member function exists.

    @Class and @TemplateClass calls may be nested, and generally should be to wrap nested classes.
    """

    def __init__(self, name="Class"):
        BlockMacro.__init__(self, name)
        self.addOption(name="target", type=OptionType.REF, default=None,
                       doc="target C++ class to wrap")
        self.addOption(name="rename", type=OptionType.STRING, default=None)
        self.addOption(name="init", type=OptionType.CODE, default="bp::no_init")
        self.addOption(name="doc", type=OptionType.CODE, default=None)
        self.addOption(name="const_aware", type=OptionType.BOOL, default=True)
        self.addOption(name="noncopyable", type=OptionType.BOOL, default=False)
        self.addOption(name="shared_ptr", type=OptionType.BOOL, default=True)
        self.addOption(name="include_regex", type=OptionType.STRING, default=".*")
        self.addOption(name="ignore_regex", type=OptionType.STRING, default=None)
        self.addOption(name="include_list", type=OptionType.LIST, default=None)
        self.addOption(name="ignore_list", type=OptionType.LIST, default=())

    def apply(self, options, state):
        raise NotImplementedError()

    def finish(self, options, state):
        raise NotImplementedError()

class TemplateClass(Class):
    """Generate the skeleton of a C++ template class wrapper.

    This macro expands to a template struct definition containing a single static member function
    ("declare()") that builds a Boost.Python wrapper for the class.  For instance, the macro:
    
    template <typename T, int N>
    @TemplateClass(Foo, {<T,N>}, const_aware=False) {
    /* body */
    };

    will expand to (roughly):

    template <typename T, int N>
    struct PyFoo<T,N> {
        static void declare(std::string const & name) {
            bp::class_< Foo<T,N> > wrapper(name, bp::no_init);
            wrapper.enable_shared_ptr();
            /* additional member wrapper declarations */
            custom(wrapper);
        }
        /* body */
    };

    Note that the macro does not generate the template parameter list; this should be included
    explicitly by the user before the macro call for correct results.  Also, unlike @Class,
    the declare() static member function takes a string argument that sets the Python name
    of the class; this should be different for different template instantiations.

    The unqualified call to "custom(wrapper)" at the end allows the user to add their own
    custom Boost.Python code or more finely-grained bpdox macros to the class wrapper
    by adding a template static member function with of the form

    template <typename Wrapper> static void custom(Wrapper & wrapper);

    This call will generate an error if no custom member function is defined, but the user
    can use the @DefaultCustom macro to define a non-member custom function at the top of each
    bpdox source file that will be invoked when no custom member function exists.

    @Class and @TemplateClass calls may be nested, and generally should be to wrap nested classes.
    """

    def __init__(self, name="TemplateClass"):
        Class.__init__(self, name)
        self.addOption(name="parameters", type=OptionType.CODE, default=None)
        self.reorderOptions(["target", "parameters"])

    def apply(self, options, state):
        raise NotImplementedError()

    def finish(self, options, state):
        raise NotImplementedError()

class Function(SimpleMacro):

    def __init__(self, name="Function"):
        SimpleMacro.__init__(self, name)
        self.addOption(name="target", type=OptionType.REF, default=None)
        self.addOption(name="rename", type=OptionType.STRING, default=None)
        self.addOption(name="policies", type=OptionType.CODE, default=None)
        self.addOption(name="pointer", type=OptionType.CODE, default=None)
        self.addOption(name="args", type=OptionType.CODE, default=None)
        self.addOption(name="doc", type=OptionType.CODE, default=None)

    def apply(self, options, state):
        raise NotImplementedError()

class Method(Function):

    def __init__(self, name="Method"):
        Function.__init__(self, name)

    def apply(self, options, state):
        raise NotImplementedError()
