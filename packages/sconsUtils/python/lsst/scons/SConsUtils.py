#
# Note that this file is called SConsUtils.py not SCons.py so as to allow us to import SCons
#
import glob
import os
import re
import shutil
from SCons.Script import *              # So that this file has the same namespace as SConstruct/SConscript
from SCons.Script.SConscript import SConsEnvironment
SCons.progress_display = SCons.Script.Main.progress_display
import stat
import sys
import imp
from types import *
from . import svn

try:
    import eups
except ImportError:
    pass    

def MakeEnv(eups_product, versionString=None, dependencies=[],
            eups_product_path=None, variables=None, traceback=False):
    """
    Setup a standard SCons environment, add our dependencies, and fix some
    os/x problems
    
    This function should be called early in a SConstruct file to initialize
    the build environment for the product being built.

    Identifying the Product

    The eups_product, versionString, and eups_product_path arguments are
    used to identify the product being built.  eups_product is the name that
    EUPS will know the product as; that is, this is the name one would provide
    to the EUPS setup command to load the product into the user's environment.
    Certain assumptions are made about the product based on this name and
    LSST conventions.  In particular,
    - if the product builds a linkable library, that library will be
      named after the product name.  For example, if eups_product='foo',
      the static library will be called libfoo.a.
    - Unless the eups_product_path is specified, it will be assumed that
      the default location for installing the product will be
      root/flavor/eups_product/version where root is the first directory 
      listed in $EUPS_PATH, flavor is the platform flavor name (e.g. Linux), 
      and version is the actual version of the product release (see below).

    This function will attempt to determine the version of the product being
    built from the value of the versionString.  This is a string that is
    generated automatically by the code repository/revisioning system; 
    currently supported systems are CVS and Subversion (SVN).  When the 
    SConstruct file is first created by the product developer, the 
    versionString argument is set to r'$HeadURL$' (for SVN) or r'$Name$' (for 
    CVS).  When the SConstruct file is subsequently checked out, the code
    repository system converts this into a value that encodes the release
    version of the product.  This function will automatically decode the
    versionString into a real version number.

    The eups_product_path argument is the path to the default directory for
    installing the product relative.  The value can be parameterized with
    with printf-like directives.  For example, when eups_product_path is
    not set, the install path is equivalent to:
    @verbatim
        "%P/%f/%p/%v"
    @endverbatim
    where the %-sequences are replaced as follows:
    @verbatim
        %P     the first directory in the EUPS_PATH environment variable
        %f     the platform flavor (e.g. Linux, Darwin, etc.)
        %p     the product name (e.g. fw, mwi)
        %v     the product version
        %c     the current working directory
    @endverbatim
    Of course, the path can be explicitly specified without using any
    %-sequences.  The most common use, however is to insert additional
    directories into the path; for example:
    @verbatim
        "%P/%f/deploy/%p/%v"
    @endverbatim
    
    The latter elements of a dependency description are optional.  If less
    information is provided, less is done in terms of verification.  Generally,
    a symbol will need to be provided to verify that a required library is
    usable.  Note, however, that for certain special products, specific checks
    are carried out to ensure that the product is in a useable state; thus,
    providing a library name without a symbol is often still useful. 
    
    @param eups_product   the name of this product (as it is to be known as
                             by EUPS) that is being built.  See "Identifying
                             the Product" above for a discussion of how this
                             is used.  
    @param versionString  a string provided by the code repository identifying
                             the version of the product being built.  When 
                             using Subversion (SVN), this is initially set to
                             r"$HeadURL$"; when the SConstruct file is checked
                             out of SVN, this value will be changed to a string
                             encoding the release version of the product.  If
                             not provided, the release version will be set to
                             unknown.  See "Identifying the Product" above for 
                             more details.
    @param eups_product_path  the relative path to the default installation
                             directory as format string.  Use this if the
                             product should be installed in a subdirectory
                             relative to $EUPS_PATH.  See "Identifying the
                             Product" above for more details.
    @param variables       a Variables object to use to define command-line
                             options custom to the current build script.
                             If provided, this should have been created with
                             LsstVariables()
    @param traceback      a boolean switch indicating whether any uncaught 
                             Python exceptions raised during the build process
                             should result in a standard Python traceback
                             message to be displayed.  The default, False,
                             causes tracebacks not to be printed; only the
                             error message will be printed.
    """
    #
    # We made the changes needed for scons 1.2; some (Option -> Variable) are not backwards compatible
    #
    if False:                           # apparently we're using trunk sconsUtils in the buildbot
        EnsureSConsVersion(2, 0, 1)
    #
    # We don't usually want a traceback at the interactive prompt
    # XXX This hook appears to be ignored by scons. Why?
    #
    if not traceback:
        def my_excepthook(type, value, tb):
            print >> sys.stderr, "Error:", value
            sys.exit(1)
        sys.excepthook = my_excepthook
    #
    # Argument handling
    #
    opts = variables
    if opts is None:
        opts = LsstVariables()

    opts.AddVariables(
        EnumVariable('cc', 'Choose the compiler to use', '', allowed_values=('', 'gcc', 'icc')),
        BoolVariable('debug', 'Set to enable debugging flags', True),
        ('eupsdb', 'Specify which element of EUPS_PATH should be used', None),
        BoolVariable('filterWarn', 'Filter out a class of warnings deemed irrelevant', True),
        ('flavor', 'Set the build flavor', None),
        BoolVariable('force', 'Set to force possibly dangerous behaviours', False),
        ('optfile', 'Specify a file to read default options from', None),
        ('prefix', 'Specify the install destination', None),
        EnumVariable('opt', 'Set the optimisation level', 0, allowed_values=('0', '1', '2', '3')),
        EnumVariable('profile', 'Compile/link for profiler', 0, allowed_values=('0', '1', 'pg', 'gcov')),
        BoolVariable('setenv', 'Treat arguments such as Foo=bar as defining construction variables', False),
        ('version', 'Specify the current version', None),
        ('baseversion', 'Specify the current base version', None),
        ('noOptFiles', "Specify a list of files that should NOT be optimized", None)
        )

    toolpath = []
    if os.path.exists("python/lsst/scons/SConsUtils.py"): # boostrapping sconsUtils
        toolpath += ["python/lsst/scons"]
    elif os.environ.has_key('SCONSUTILS_DIR'):
        toolpath += ["%s/python/lsst/scons" % os.environ['SCONSUTILS_DIR']]

    if os.environ.has_key('LD_LIBRARY_PATH'):
        LD_LIBRARY_PATH = os.environ['LD_LIBRARY_PATH']
    else:
        LD_LIBRARY_PATH = None
        
    if os.environ.has_key('DYLD_LIBRARY_PATH'):
        DYLD_LIBRARY_PATH = os.environ['DYLD_LIBRARY_PATH']
    else:
        DYLD_LIBRARY_PATH = None

    if os.environ.has_key('SHELL'):     # needed by eups
        SHELL = os.environ['SHELL']
    else:
        SHELL = None
        
    if os.environ.has_key('TMPDIR'):     # needed by eups
        TMPDIR = os.environ['TMPDIR']
    else:
        TMPDIR = None

    if os.environ.has_key('BPDOX_PATH'):     # needed by bpdox
        BPDOX_PATH = os.environ['BPDOX_PATH']
    else:
        BPDOX_PATH = None

    ourEnv = {'EUPS_DIR' : os.environ['EUPS_DIR'],
              'EUPS_PATH' : os.environ['EUPS_PATH'],
              'PATH' : os.environ['PATH'],
              'DYLD_LIBRARY_PATH' : DYLD_LIBRARY_PATH,
              'LD_LIBRARY_PATH' : LD_LIBRARY_PATH,
              'SHELL' : SHELL,
              'TMPDIR' : TMPDIR,
              'BPDOX_PATH' : BPDOX_PATH,
              }

    # Add all EUPS directories
    upsDirs = []
    for k in filter(lambda x: re.search(r"_DIR$", x), os.environ.keys()):
        p = re.search(r"^(.*)_DIR$", k).groups()[0]
        try:
            varname = eups.utils.setupEnvNameFor(p)
        except AttributeError:
            varname = "SETUP_" + p      # We're running an old (<= 1.2) version of eups
        if os.environ.has_key(varname):
            ourEnv[varname] = os.environ[varname]
            ourEnv[k] = os.environ[k]
            upsDirs.append(os.path.join(os.environ[k], "ups"))
    env = Environment(ENV = ourEnv, variables=opts,
		      tools = ["default", "doxygen"],
		      toolpath = toolpath
		      )
    env0 = env.Clone()

    env['eups_product'] = eups_product
    Help(opts.GenerateHelpText(env))

    # The first level of the libs dict is the "target": we have separate
    # lists for main libraries, Python modules, and C++-coded unit tests.
    env.libs = {}
    for target in ("main", "python", "test"):
        env.libs[target] = []

    #
    # We don't want "lib" inserted at the beginning of loadable module names;
    # we'll import them under their given names.
    #
    env['LDMODULEPREFIX'] = ""

    if env['PLATFORM'] == 'darwin':
        env['LDMODULESUFFIX'] = ".so"

        if not re.search(r"-install_name", str(env['SHLINKFLAGS'])):
            env.Append(SHLINKFLAGS = ["-Wl,-install_name", "-Wl,${TARGET.file}"])
        
    #
    # Remove valid options from the arguments
    #
    for opt in opts.keys():
        try:
            del ARGUMENTS[opt]
        except KeyError:
            pass
    #
    # Process those arguments
    #
    if env['debug']:
        env.Append(CCFLAGS = ['-g'])

    eups_path = None
    try:
        db = env['eupsdb']
        if not os.environ.has_key('EUPS_PATH'):
            msg = "You can't use eupsdb=XXX without an EUPS_PATH set"
            if traceback:
                raise RuntimeError, msg
            else:
                sys.excepthook(RuntimeError, msg, None)

        eups_path = None
        for d in os.environ['EUPS_PATH'].split(':'):
            if re.search(r"/%s$|^%s/|/%s/" % (db, db, db), d):
                eups_path = d
                break

        if not eups_path:
            msg = "I cannot find DB \"%s\" in $EUPS_PATH" % db
            if traceback:
                raise RuntimeError, msg
            else:
                sys.excepthook(RuntimeError, msg, None)
    except KeyError:
        if os.environ.has_key('EUPS_PATH'):
            eups_path = os.environ['EUPS_PATH'].split(':')[0]

    env['eups_path'] = eups_path

    try:
        env['PLATFORM'] = env['flavor']
        del env['flavor']
    except KeyError:
        pass

    #
    # Check arguments
    #
    errorStr = ""
    #
    # Process otherwise unknown arguments.  If setenv is true,
    # set construction variables; otherwise generate an error
    #
    if env['setenv']:
        for key in ARGUMENTS.keys():
            env[key] = Split(ARGUMENTS[key])
    else:
        for key in ARGUMENTS.keys():
            errorStr += " %s=%s" % (key, ARGUMENTS[key])
        if errorStr:
            sys.stderr.write("Unprocessed arguments:%s\n" % errorStr)

    #
    # We need a binary name, not just "Posix"
    #
    try:
        env['eups_flavor'] = eups.flavor()
    except:
        print >> sys.stderr, "Unable to import eups; guessing flavor"
        if env['PLATFORM'] == "posix":
            env['eups_flavor'] = os.uname()[0].title()
        else:
            env['eups_flavor'] = env['PLATFORM'].title()

    #
    # Where to install
    #
    env.installing = filter(lambda t: t == "install", BUILD_TARGETS)# are we installing?
    env.declaring = filter(lambda t: t == "declare" or t == "current", BUILD_TARGETS)# are we declaring?

    prefix = setPrefix(env, versionString, eups_product_path)
    env['prefix'] = prefix
    
    env["libDir"] = "%s/lib" % prefix
    env["pythonDir"] = "%s/python" % prefix

    if env.installing:
        SCons.progress_display("Installing into %s" % prefix)
    #
    # Is the C compiler really gcc/g++?
    #
    def ClassifyCc(context):
        """Return a string identifing the compiler in use"""
        versionStrings = {"Free Software Foundation" : "gcc",
                          "Intel Corporation" : "icc",
                          }

        context.Message("Checking who built the CC compiler...")
        if False:                           # fails with scons 1.2.d20100306.r4691 (1.3 release candidate)
            action = r"$CC --version | perl -ne 'chomp; " + r"print if(s/.*(%s).*/\1/)' > $TARGET" \
                     % "|".join(versionStrings.keys())
            result = context.TryAction(Action(action))
            context.Result(result[1])
            return versionStrings.get(result[1], "unknown")
        else:                           # workaround scons bug
            for string, key in versionStrings.items():
                action = r"$CC --version | grep '%s' > $TARGET" % string
                result = context.TryAction(Action(action))
                if result[0]:
                    context.Result(key)
                    return key

            return "unknown"

    if env.GetOption("clean") or env.GetOption("no_exec") or env.GetOption("help") :
        env.whichCc = "unknown"         # who cares? We're cleaning/not execing, not building
    else:
        if env['cc'] != '':
            CC = CXX = None

            if re.search(r"^gcc( |$)", env['cc']):
                CC = env['cc']
                CXX = re.sub(r"^gcc", "g++", CC)
            elif re.search(r"^icc( |$)", env['cc']):
                CC = env['cc']
                CXX = re.sub(r"^icc", "icpc", CC)
            else:
                errors += ["Unrecognised compiler:%s" % env['cc']]

            if CC and env['CC'] == env0['CC']:
                env['CC'] = CC
            if CC and env['CXX'] == env0['CXX']:
                env['CXX'] = CXX

        conf = Configure(env, custom_tests = {'ClassifyCc' : ClassifyCc})
        env.whichCc = conf.ClassifyCc()

        conf.Finish()
    #
    # Compiler flags; CCFLAGS => C and C++
    #
    if env.whichCc == "gcc":
        env.Append(CCFLAGS = ['-Wall'])
    if env.whichCc == "icc":
        env.Append(CCFLAGS = ['-Wall'])

        ignoreWarnings = {
            21 : 'type qualifiers are meaningless in this declaration',
            68 : 'integer conversion resulted in a change of sign',
            111 : 'statement is unreachable',
            191 : 'type qualifier is meaningless on cast type',
            193 : 'zero used for undefined preprocessing identifier "SYMB"',
            279 : 'controlling expression is constant',
            304 : 'access control not specified ("public" by default)', # comes from boost
            383 : 'value copied to temporary, reference to temporary used',
            #424 : 'Extra ";" ignored',
            444 : 'destructor for base class "CLASS" is not virtual',
            981 : 'operands are evaluated in unspecified order',
            1418 : 'external function definition with no prior declaration',
            1419 : 'external declaration in primary source file',
            1572 : 'floating-point equality and inequality comparisons are unreliable',
            1720 : 'function "FUNC" has no corresponding member operator delete (to be called if an exception is thrown during initialization of an allocated object)',
            2259 : 'non-pointer conversion from "int" to "float" may lose significant bits',
            }
        if env['filterWarn']:
            env.Append(CCFLAGS = ["-wd%s" % (",".join([str(k) for k in ignoreWarnings.keys()]))])
        # Workaround intel bug; cf. RHL's intel bug report 580167
        env.Append(LINKFLAGS = ["-Wl,-no_compact_unwind", "-wd,11015"])
        
    if env['opt']:
        env.Append(CCFLAGS = ['-O%d' % int(env['opt'])])
    if env['profile'] == '1' or env['profile'] == "pg":
        env.Append(CCFLAGS = ['-pg'])
        env.Append(LINKFLAGS = ['-pg'])
    elif env['profile'] == 'gcov':
        env.Append(CCFLAGS = '--coverage')
        env.Append(LINKFLAGS = '--coverage')

    env.GetOption("silent")
    #
    # Is C++'s TR1 available?  If not, use e.g. #include "lsst/tr1/foo.h"
    #
    if not (env.GetOption("clean") or env.GetOption("help")):
        if not env.GetOption("no_exec"):
            conf = env.Configure()
            env.Append(CCFLAGS = ['-DLSST_HAVE_TR1=%d' % int(conf.CheckHeader("tr1/unordered_map", language="C++"))])
            conf.Finish()
    #
    # Byte order
    #
    import socket
    if socket.htons(1) != 1:
        env.Append(CCFLAGS = ['-DLSST_LITTLE_ENDIAN=1'])

    #
    # If we're linking to libraries that themselves linked to
    # shareable libraries we need to do something special.
    if (re.search(r"^(Linux|Linux64)$", env["eups_flavor"]) and 
        os.environ.has_key("LD_LIBRARY_PATH")):
        env.Append(LINKFLAGS = ["-Wl,-rpath-link"])
        env.Append(LINKFLAGS = ["-Wl,%s" % os.environ["LD_LIBRARY_PATH"]])

    #
    # A list of doxygen tag files from dependent products with a doc subdirectory.
    #
    env["DOXYGEN_TAGS"] = []

    #
    # A list of doxygen include files from dependent products with a doc subdirectory.
    #    
    env["DOXYGEN_INCLUDES"] = []

    env['CPPPATH'] = []
    env['LIBPATH'] = []

    #
    # Recursively configure dependencies.
    #
    if not env.GetOption("clean") and not env.GetOption("help"):
        env = configureProducts(env, upsDirs)

    Export('env')
    return env

makeEnv = MakeEnv                       # backwards compatibility

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def getLibs(env, targets="main"):
    """Get the libraries the package should be linked with.

    Arguments:
       targets --- A string containing whitespace-delimited targets.  Standard
                   targets are "main", "python", and "test".  Default is "main".
                   A special virtual target "self" can be provided, returning
                   the results of targets="main" with the eups_target library
                   removed.

    Typically, main libraries will be linked with LIBS=getLibs("self"),
    Python modules will be linked with LIBS=getLibs("main python") and
    C++-coded test programs will be linked with LIBS=getLibs("main test")
    """
    libs = []
    removeSelf = False
    for target in targets.split():
        if target == "self":
            target = "main"
            removeSelf = True
        for lib in env.libs[target]:
            if lib not in libs:
                libs.append(lib)
    if removeSelf:
        try:
            libs.remove(env["eups_product"])
        except ValueError:
            pass
    return libs

SConsEnvironment.getLibs = getLibs

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def LsstVariables(files=None):
    """@brief Create a Variables object using LSST conventions.

    The SCons Variables object is used in the LSST build system to define
    variables that can be used on the command line or loaded in from a file.
    These variables given as "name=value" arguments on the scons command line or
    listed, one per line, in a separate file.  An variables file can be specified
    on the scons command line with the "optfile" variable (e.g. "optfile=myoptions.py";
    "variables" used to be called "options").  If optfile is not specified, scons will look for
    a file called "buildOpts.py" by default.  (You can specify additional
    option files to load via the "files" argument to this constructor.)  If the
    user provides any command-line variable options that has not been defined
    via an Variables instance, scons will exit with an error, complaining about
    an unused argument.  

    To define your custom variable options, you should create an Variables object
    with this constructor function \e prior to the use of scons.makeEnv.
    Then you can use the standard Variables member functions (Add() or 
    AddVariables()) to define your variable options (see
    @link http://www.scons.org/doc/HTML/scons-man.html the SCons Man
    page for details).  For example,
    @code
       opts = scons.LsstVariables()
       opts.Add('pkgsurl', 'the base url for the software server',
                'http://dev.lsstcorp.org/pkgs')
    @endcode
    In this example, we defined a new options called "pkgsurl" with a default
    value of "http://dev.lsstcorp.org/pkgs".  The second argument is a help
    string.

    To actually use these options, you must load them into the environment
    by passing it to the scons.makeEnv() function via its variables argument:
    @code
       env = scons.makeEnv("mypackage", "$HeadURL$", variables=opts)
    @endcode
    scons.makeEnv() will automatically look for these options on the command
    line as well as any option files.  The values found their will be loaded
    into the environment's dictionary (i.e. accessible via env[optionname]).

    Note that makeEnv() will internally add additional options to the Variables
    object you pass it, overriding your definitions where you have used the
    same name.  These standard options include:
    @verbatim
        debug     Set to > 1 to enable debugging flag (default: 0)
        eupsdb    Specify which element of EUPS_PATH should be used (default:
                    the value of $EUPS_PATH)
        flavor    the desired build flavor (default is auto-detected)
        optfile   a file to read default options from (default:
                    buildOpts.py)
        prefix    the install destination directory (default is auto-detected
                    from the EUPS environment).
        opt       the optimization level, an integer between 0 and 3, inclusive
                    (default: 0)
        version   Specify the current version (default is auto-detected)
    @endverbatim    
    
    This constructor should be preferred over the standard SCons Variables
    constructor because it defines various LSST conventions.  In particular,
    it defines the default name an options file to look for.  It will also
    print a warning message if any specified options file (other than the
    default) cannot be found.

    @param files    one or more names of option files to look for.  Multiple
                       names must be given as a python list.
    """
    if files is None:
        files = []
    elif type(files) is not ListType:
        files = [files]

    if ARGUMENTS.has_key("optfile"):
        configfile = ARGUMENTS["optfile"]
        if configfile not in files:
            files.append(configfile)

    for file in files:
        if not os.path.isfile(file):
            print >> sys.stderr, \
                     "Warning: Will ignore non-existent options file, %s" \
                     % file

    if not ARGUMENTS.has_key("optfile"):
        files.append("buildOpts.py")

    return Variables(files)

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

class ParseBoostLibrary(object):
    def __init__(self, shlibprefix, library, shlibsuffix, blib):
        """Parse a boost library name, given the prefix (often "lib"),
        the library name (e.g. "boost_regexp"), the suffix (e.g. ".so")
        and the actual name of the library

        Taken from libboost-doc/HTML/more/getting_started.html
        """

        self.toolset, threading, runtime = None, None, None # parts of boost library name
        self.libversion = None

        mat = re.search(r"^%s%s-?(.+)%s" % (shlibprefix, library, shlibsuffix), blib)
        self.libname = library
        if mat:
            self.libname += "-" + mat.group(1)

            opts = mat.group(1).split("-")

            if opts:
                self.libversion = opts.pop()

            if opts:
                self.toolset = opts.pop(0)

            if opts:
                if opts[0] == "mt":
                    threading = opts.pop(0)

            if opts:
                runtime = opts.pop(0)

        self.threaded = threading and threading == "mt"

        self.static_runtime =     runtime and re.search("s", runtime)
        self.debug_runtime =      runtime and re.search("g", runtime)
        self.debug_python =       runtime and re.search("y", runtime)
        self.debug_code =         runtime and re.search("d", runtime)
        self.stlport_runtime =    runtime and re.search("p", runtime)
        self.stlport_io_runtime = runtime and re.search("n", runtime)

        return

def mangleLibraryName(env, libdir, lib):
    """If lib's a boost library, choose the right one; there may be a number to choose from"""

    if not libdir:       # we don't know libdir, so we can't poke around
        return lib

    if not re.search(r"^boost", lib):
        return lib
    
    shlibprefix = env['SHLIBPREFIX']
    if re.search(r"^\$", shlibprefix) and env.has_key(shlibprefix[1:]):
        shlibprefix = env[shlibprefix[1:]]
    shlibsuffix = env['SHLIBSUFFIX']

    libs = glob.glob(os.path.join(libdir, shlibprefix + lib + "*" + shlibsuffix))

    blibs = {}
    for blib in libs:
        blibs[blib] = ParseBoostLibrary(shlibprefix, lib, shlibsuffix,
                                        os.path.basename(blib))

    if len(blibs) == 0: # nothing clever to do
        return lib
    elif len(blibs) == 1: # only one choice
        lib = blibs.values()[0].libname
    else:           # more than one choice
        if env['debug'] and filter(lambda key: blibs[key].debug_code, blibs.keys()):
            for blib in blibs:
                if not blibs[blib].debug_code:
                    del blibs[blib]
                    break

        if len(blibs) == 1:             # only one choice
            lib = blibs.values()[0].libname
        else:                           # How do we choose? Take the shortest
            lib = None
            for blib in blibs.values():
                if not lib or len(blib.libname) < lmin:
                    lib = blib.libname
                    lmin = len(lib)

    return lib            

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def SharedLibraryIncomplete(self, target, source, **keywords):
    """Like SharedLibrary, but don't insist that all symbols are resolved"""

    myenv = self.Clone()

    if myenv['PLATFORM'] == 'darwin':
        myenv['SHLINKFLAGS'] += ["-undefined", "suppress", "-flat_namespace"]

    return myenv.SharedLibrary(target, source, **keywords)

SConsEnvironment.SharedLibraryIncomplete = SharedLibraryIncomplete

def LoadableModuleIncomplete(self, target, source, **keywords):
    """Like LoadableModule, but don't insist that all symbols are resolved"""

    myenv = self.Clone()
    if myenv['PLATFORM'] == 'darwin':
        myenv.Append(LDMODULEFLAGS = ["-undefined", "suppress", "-flat_namespace",])
    #
    # Swig-generated .cc files cast pointers to long longs and back,
    # which is illegal.  This flag tells g++ about the sin
    #
    try:
        if myenv.isGcc:
            myenv.Append(CCFLAGS = ["-fno-strict-aliasing",])
    except AttributeError:
        pass

    return myenv.LoadableModule(target, source, **keywords)

SConsEnvironment.LoadableModuleIncomplete = LoadableModuleIncomplete

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# My reimplementation of installFunc that accepts a regexp of files to ignore
#
def copytree(src, dst, symlinks=False, ignore = None):
    """Recursively copy a directory tree using copy2().

    The destination directory must not already exist.
    If exception(s) occur, an Error is raised with a list of reasons.

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied.

    If the optional ignore option is present, treat it as a
    regular expression and do NOT copy files that match the pattern

    XXX Consider this example code rather than the ultimate tool.

    """

    last_component = os.path.split(src)[-1]
    if ignore and re.search(ignore, last_component):
        #print "Ignoring", last_component
        return

    names = os.listdir(src)
    os.mkdir(dst)
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)

        if ignore and re.search(ignore, srcname):
            #print "Ignoring", srcname
            continue

        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, symlinks, ignore)
            else:
                shutil.copy2(srcname, dstname)
            # XXX What about devices, sockets etc.?
        except (IOError, os.error), why:
            errors.append((srcname, dstname, why))
    if errors:
        raise Error, errors

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def ProductDir(product):
    """Return a product's PRODUCT_DIR, or None"""
    import eups

    return eups.productDir(product)

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def _ProductDir(self, product):
    return ProductDir(product)

SConsEnvironment.ProductDir = _ProductDir
    
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def makeProductPath(pathFormat, env):
    """return a path to use as the installation directory for a product
    @param pathFormat     the format string to process 
    @param env            the scons environment
    @param versionString  the versionString passed to MakeEnv
    """
    pathFormat = re.sub(r"%(\w)", r"%(\1)s", pathFormat)
    
    eups_path = os.environ['PWD']
    if env.has_key('eups_product') and env['eups_path']:
        eups_path = env['eups_path']

    return pathFormat % { "P": eups_path,
                          "f": env['eups_flavor'],
                          "p": env['eups_product'],
                          "v": env['version'],
                          "c": os.environ['PWD'] }
    
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def getVersion(env, versionString):
    """Set a version ID from env, or
    a cvs or svn ID string (dollar name dollar or dollar HeadURL dollar)"""

    version = "unknown"

    if env.has_key('version'):
        version = env['version']
        if env.has_key('baseversion') and \
                not version.startswith(env['baseversion']):
            print >> sys.stderr, \
                  "Warning: explicit version %s is incompatible with baseversion %s" % (version, env['baseversion'])
    elif not versionString:
        version = "unknown"
    elif re.search(r"^[$]Name:\s+", versionString):
        # CVS.  Extract the tagname
        version = re.search(r"^[$]Name:\s+([^ $]*)", versionString).group(1)
        if version == "":
            version = "cvs"
    elif re.search(r"^[$]HeadURL:\s+", versionString):
        # SVN.  Guess the tagname from the last part of the directory
        HeadURL = re.search(r"^[$]HeadURL:\s+(.*)", versionString).group(1)
        HeadURL = os.path.split(HeadURL)[0]
        if env.installing or env.declaring:
            try:
                version = svn.guessVersionName(HeadURL)
            except RuntimeError, e:
                if env['force']:
                    version = "unknown"
                else:
                    print >> sys.stderr, \
                          "%s\nFound problem with svn revision number; update or specify force=True to proceed" %e
                    sys.exit(1)
            if env.has_key('baseversion'):
                version = env['baseversion'] + "+" + version

    env["version"] = version
    return version

def setPrefix(env, versionString, eups_product_path=None):
    """Set a prefix based on the EUPS_PATH, the product name, and a versionString from cvs or svn"""

    if eups_product_path:
        getVersion(env, versionString)
        eups_prefix = makeProductPath(eups_product_path, env)
        
    elif env.has_key('eups_path') and env['eups_path']:
        eups_prefix = env['eups_path']
	flavor = env['eups_flavor']
	if not re.search("/" + flavor + "$", eups_prefix):
	    eups_prefix = os.path.join(eups_prefix, flavor)

        prodpath = env['eups_product']
        if env.has_key('eups_product_path') and env['eups_product_path']:
            prodpath = env['eups_product_path']

        eups_prefix = os.path.join(eups_prefix, prodpath,
				   getVersion(env, versionString))
    else:
        eups_prefix = None

    if env.has_key('prefix'):
        if eups_prefix:
            print >> sys.stderr, "Ignoring prefix %s from EUPS_PATH" % eups_prefix

        return makeProductPath(env['prefix'], env)
    elif env.has_key('eups_path') and env['eups_path']:
        prefix = eups_prefix
    else:
        prefix = "/usr/local"

    return prefix

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# Don't use these in new code --- they date from before RHL learnt about GetOption()
#
def CleanFlagIsSet(self):
    """Return True iff they're running "scons --clean" """

    return self.GetOption("clean")

SConsEnvironment.CleanFlagIsSet = CleanFlagIsSet

def HelpFlagIsSet(self):
    """Return True iff they're running "scons --help" """

    return self.GetOption("help")

SConsEnvironment.HelpFlagIsSet = HelpFlagIsSet

def NoexecFlagIsSet(self):
    """Return True iff they're running "scons -n" """

    return self.GetOption("no_exec")

SConsEnvironment.NoexecFlagIsSet = NoexecFlagIsSet

def QuietFlagIsSet(self):
    """Return True iff they're running "scons -Q" """

    return self.GetOption("silent")
    
SConsEnvironment.QuietFlagIsSet = QuietFlagIsSet

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def Declare(self, products=None):
    """Create current and declare targets for products.  products
    may be a list of (product, version) tuples.  If product is None
    it's taken to be self['eups_product']; if version is None it's
    taken to be self['version'].
    
    We'll add Declare to class Environment"""

    if "undeclare" in COMMAND_LINE_TARGETS and not self.GetOption("silent"):
        print >> sys.stderr, "'scons undeclare' is deprecated; please use 'scons declare -c' instead"

    if \
           "declare" in COMMAND_LINE_TARGETS or \
           "undeclare" in COMMAND_LINE_TARGETS or \
           ("install" in COMMAND_LINE_TARGETS and self.GetOption("clean")) or \
           "current" in COMMAND_LINE_TARGETS:
        current = []; declare = []; undeclare = []

        if not products:
            products = [None]

        for prod in products:
            if not prod or isinstance(prod, str):   # i.e. no version
                product = prod

                if self.has_key('version'):
                    version = self['version']
                else:
                    version = None
            else:
                product, version = prod

            if not product:
                product = self['eups_product']

            if "EUPS_DIR" in os.environ.keys():
                self['ENV']['PATH'] += os.pathsep + "%s/bin" % (os.environ["EUPS_DIR"])

                if "undeclare" in COMMAND_LINE_TARGETS or self.GetOption("clean"):
                    if version:
                        command = "eups undeclare --flavor %s %s %s" % \
                                  (self['eups_flavor'], product, version)
                        if "current" in COMMAND_LINE_TARGETS and not "declare" in COMMAND_LINE_TARGETS:
                            command += " --current"
                            
                        if self.GetOption("clean"):
                            self.Execute(command)
                        else:
                            undeclare += [command]
                    else:
                        print >> sys.stderr, "I don't know your version; not undeclaring to eups"
                else:
                    command = "eups declare --force --flavor %s --root %s" % \
                              (self['eups_flavor'], self['prefix'])

                    if self.has_key('eups_path'):
                        command += " -Z %s" % self['eups_path']
                        
                    if version:
                        command += " %s %s" % (product, version)

                    current += [command + " --current"]
                    declare += [command]

        if current:
            self.Command("current", "", action=current)
        if declare:
            if "current" in COMMAND_LINE_TARGETS:
                self.Command("declare", "", action="") # current will declare it for us
            else:
                self.Command("declare", "", action=declare)
        if undeclare:
            self.Command("undeclare", "", action=undeclare)
                
SConsEnvironment.Declare = Declare

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def CleanTree(files, dir=".", recurse=True, verbose=False):
    """Remove files matching the argument list starting at dir
    when scons is invoked with -c/--clean and no explicit targets are listed
    
    E.g. CleanTree(r"*~ core")

    If recurse is True, recursively descend the file system; if
    verbose is True, print each filename after deleting it
    """
    #
    # Generate command that we may want to execute
    #
    files_expr = ""
    for file in Split(files):
        if files_expr:
            files_expr += " -o "

        files_expr += "-name %s" % re.sub(r"(^|[^\\])([[*])", r"\1\\\2",file) # quote unquoted * and []
    #
    # don't use xargs --- who knows what needs quoting?
    #
    action = "find %s" % dir
    action += r" \( -name .svn -prune -o -name \* \) "
    if not recurse:
        action += " ! -name . -prune"

    file_action = "rm -f"

    action += r" \( %s \) -exec %s {} \;" % \
        (files_expr, file_action)

    if verbose:
        action += " -print"
    #
    # Clean up scons files --- users want to be able to say scons -c and get a clean copy
    #
    action += " ; rm -rf .sconf_temp .sconsign.dblite"
    #
    # Do we actually want to clean up?  We don't if the command is e.g. "scons -c install"
    #
    if "clean" in COMMAND_LINE_TARGETS:
        Command("clean", "", action=action)
    elif not COMMAND_LINE_TARGETS and GetOption("clean"):
        Execute(Action([action]))

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def InstallEups(env, dest, files=[], presetup=""):
    """Install a ups directory, setting absolute versions as appropriate
    (unless you're installing from the trunk, in which case no versions
    are expanded).  Any build/table files present in "./ups" are automatically
    added to files.
    
    If presetup is provided, it's expected to be a dictionary with keys
    product names and values the version that should be installed into
    the table files, overriding eups expandtable's usual behaviour. E.g.
env.InstallEups(os.path.join(env['prefix'], "ups"), presetup={"sconsUtils" : env['version']})
    """

    if not env.installing:
        return

    if env.GetOption("clean"):
        print >> sys.stderr, "Removing", dest
        shutil.rmtree(dest, ignore_errors=True)
    else:
        presetupStr = []
        for p in presetup:
            presetupStr += ["--product %s=%s" % (p, presetup[p])]
        presetup = " ".join(presetupStr)

        env = env.Clone(ENV = os.environ)
        #
        # Add any build/table files to the desired files
        #
        files = [str(f) for f in files] # in case the user used Glob not glob.glob
        files += glob.glob(os.path.join("ups", "*.build")) + glob.glob(os.path.join("ups","*.table"))
        files = list(set(files))        # remove duplicates

        buildFiles = filter(lambda f: re.search(r"\.build$", f), files)
        build_obj = env.Install(dest, buildFiles)
        
        tableFiles = filter(lambda f: re.search(r"\.table$", f), files)
        table_obj = env.Install(dest, tableFiles)

        miscFiles = filter(lambda f: not re.search(r"\.(build|table)$", f), files)
        misc_obj = env.Install(dest, miscFiles)

        for i in build_obj:
            env.AlwaysBuild(i)

            cmd = "eups expandbuild -i --version %s %s" % (env['version'], str(i))
            env.AddPostAction(i, Action("%s" %(cmd), cmd, ENV = os.environ))

        for i in table_obj:
            env.AlwaysBuild(i)

            cmd = "eups expandtable -i -W '^(?!LOCAL:)' " # version doesn't start "LOCAL:"
            if presetup:
                cmd += presetup + " "
            cmd += str(i)

            env.AddPostAction(i, Action("%s" %(cmd), cmd, ENV = os.environ))

    return dest

SConsEnvironment.InstallEups = InstallEups

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

def PkgConfigEUPS(self, product, function=None, unique=1):
    """Load pkg-config options into the environment. Look for packages in
    PRODUCT_DIR, if they're not in the path, and suppress error messages
    about failing to find config files"""
    
    try:
        self.ParseConfig('%s-config --cflags --libs 2> /dev/null' % product)
        #print "pkg %s succeeded" % product
    except OSError:
        try:
            self.ParseConfig('env PKG_CONFIG_PATH=%s/etc pkg-config %s --cflags --libs 2> /dev/null' % \
                             (ProductDir(product), product))
            #print "pkg %s succeeded from EUPS" % product
        except OSError:
            #print "pkg %s failed" % product
            raise OSError, "Failed to find config file for %s" % product
    #
    # Strip flags that we don't want added
    #
    for k in ['CCFLAGS', 'LINKFLAGS']:
        new = []
        for flag in self[k]:
            if isinstance(flag, tuple):
                if flag[0] == "-arch":
                    continue
            new += [flag]    
        self[k] = new

SConsEnvironment.PkgConfigEUPS = PkgConfigEUPS

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def SourcesForSharedLibrary(self, files):
    """Prepare the list of files to be passed to a SharedLibrary constructor

In particular, ensure that any files listed in env.NoOptFiles (set by the command line option
noOptFile="file1 file2") are built without optimisation.

The usage pattern in an SConscript file is:
   ccFiles = env.SourcesForSharedLibrary(glob.glob("../src/*/*.cc"))
   env.SharedLibrary('afw', ccFiles, LIBS=filter(lambda x: x != "afw", env.getlibs("afw")))
"""

    if not self.get("noOptFiles"):
        return files

    noOptFiles = self["noOptFiles"].replace(".", r"\.") # it'll be used in an RE
    noOptFiles = Split(noOptFiles.replace(",", " "))

    noOptFilesRe = "/(%s)$" % "|".join(noOptFiles)

    CCFLAGS_NOOPT = re.sub(r"-O\d\s*", "", str(self["CCFLAGS"])) # remove -O flags from CCFLAGS

    sources = []
    for ccFile in files:
        if re.search(noOptFilesRe, ccFile):
            self.SharedObject(ccFile, CCFLAGS=CCFLAGS_NOOPT)
            ccFile = os.path.splitext(ccFile)[0] + self["SHOBJSUFFIX"]

        sources.append(ccFile)

    return sources

SConsEnvironment.SourcesForSharedLibrary = SourcesForSharedLibrary

#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

if False:
    # Here's a way to cache other information if we want to e.g. save
    # the configured include files -- the TryAction could save the
    # paths to a file [I think]
    #
    # As the cached tests don't seem to take any time, we're not using this
    def CheckEups(self, product):
        self.Message("Checking %s ... " % product)
        self.TryAction("echo XX %s" % product)
        ret = True
        self.Result(ret)

        return ret

    conf = env.Configure( custom_tests = { 'CheckEups' : CheckEups } )
    conf.CheckEups("numpy")
    conf.Finish()

# See if a program supports a given flag
if False:
    def CheckVariable(context, prog, flag):
        context.Message('Checking for option %s to %s... ' % (flag, prog))
        result = context.TryAction(["%s %s" % (prog, flag)])[0]
        context.Result(result)
        return result

    env = Environment()
    conf = Configure(env, custom_tests = {'CheckOption' : CheckOption})
    if not conf.CheckVariable("gcc", "-Wall"):
        print "Can't find flag"
    env = conf.Finish()
    
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def filesToTag(root=".", file_regexp=r"^[a-zA-Z0-9_].*\.(cc|h(pp)?|py)$", ignoreDirs=["examples", "tests"]):
    """Return a list of files that need to be scanned for tags, starting at directory root

    Files are chosen if they match file_regexp; toplevel directories in list ignoreDirs are ignored

    Unless force is true, this routine won't do anything unless you specified a "TAGS" target
    """

    if len(filter(lambda t: t == "TAGS", COMMAND_LINE_TARGETS)) == 0:
        return []

    files = []
    for dirpath, dirnames, filenames in os.walk(root):
        if dirpath == ".":
            dirnames[:] = [d for d in dirnames if not re.search(r"^(%s)$" % "|".join(ignoreDirs), d)]

        dirnames[:] = [d for d in dirnames if not re.search(r"^(\.svn)$", d)] # ignore .svn tree
        #
        # List of possible files to tag, but there's some cleanup required for machine-generated files
        #
        candidates = [f for f in filenames if re.search(file_regexp, f)]
        #
        # Remove files generated by swig
        #
        for swigFile in [f for f in filenames if re.search(r"\.i$", f)]:
            name = os.path.splitext(swigFile)[0]
            candidates = [f for f in candidates if not re.search(r"%s(_wrap\.cc?|\.py)$" % name, f)]

        files += [os.path.join(dirpath, f) for f in candidates]

    return files

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def InstallDir(self, prefix, dir, ignoreRegexp = r"(~$|\.pyc$|\.os?$)", recursive=True):
    """
    Install the directory dir into prefix, (along with all its descendents if recursive is True).
    Omit files and directories that match ignoreRegexp

    Unless force is true, this routine won't do anything unless you specified an "install" target
    """

    if not self.installing:
        return []

    targets = []
    for dirpath, dirnames, filenames in os.walk(dir):
        if not recursive:
            dirnames[:] = []
        else:
            dirnames[:] = [d for d in dirnames if d != ".svn"] # ignore .svn tree
        #
        # List of possible files to install
        #
        for f in filenames:
            if re.search(ignoreRegexp, f):
                continue

            targets += self.Install(os.path.join(prefix, dirpath), os.path.join(dirpath, f))

    return targets

SConsEnvironment.InstallDir = InstallDir

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def InstallLSST(self, prefix, dirs):
    """Install directories in the usual LSST way, handling "doc" and "ups" specially"""
    
    for d in dirs:
        if d == "ups":
            t = self.InstallEups(os.path.join(prefix, "ups"))
        else:
            t = self.InstallDir(prefix, d)

        self.Alias("install", t)
            
    self.Clean("install", prefix)

SConsEnvironment.InstallLSST = InstallLSST

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def PythonDependencies():
    """Return the dependencies needed to build Python modules.

    We can't add these to dependencies files, because we don't want the libraries 
    them added to env.libs.
    """
    return [["python", "Python.h"],
            ["numpy"],
            ["boost", "boost/python.hpp", "boost_python:Python"]]

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def TestDependencies():
    """Return the dependencies needed to build C++ unit tests.

    We can't add these to dependencies files, because we don't want the libraries 
    them added to env.libs.
    """
    return [["boost", "boost/test/unit_test.hpp", "boost_unit_test_framework:C++"]]

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def BuildDoxygenConfig(target, source, env):
    f = open(target[0].abspath, 'w')
    for tagPath in env['DOXYGEN_TAGS']:
        docDir, tagFile = os.path.split(tagPath)
        htmlDir = os.path.join(docDir, "html")
        f.write('TAGFILES += "{tagPath}={htmlDir}"\n'.format(tagPath=tagPath, htmlDir=htmlDir))
    docPaths = []
    incFiles = []
    for incPath in env['DOXYGEN_INCLUDES']:
        docDir, incFile = os.path.split(incPath)
        docPaths.append('"{0}"'.format(docDir))
        incFiles.append('"{0}"'.format(incFile))
    if docPaths:
        f.write('@INCLUDE_PATH = {0}\n'.format(" ".join(docPaths)))
    for incFile in incFiles:
        f.write('@INCLUDE = {0}\n'.format(incFile))
    f.write("PROJECT_NAME = {0}\n".format("/".join(["lsst"] + env["eups_product"].split("_"))))
    f.write("PROJECT_NUMBER = {0}\n".format(env["version"]))
    f.write("GENERATE_HTML = YES\n")
    f.write("HTML_OUTPUT = html\n")
    f.write("GENERATE_XML = YES\n")
    f.write("GENERATE_LATEX = NO\n") # scons' doxygen.py doesn't recognize these in @INCLUDE files
    f.write("GENERATE_MAN = NO\n")
    f.write("XML_OUTPUT = {0}\n".format(os.path.join("xml", env["eups_product"])))
    f.write("GENERATE_TAGFILE = {0}.tag\n".format(env["eups_product"]))
    i = open(source[0].abspath, 'r')
    f.write(i.read())
    i.close()
    f.close()

def DoxygenConfig(env, src):
    """Build a doxygen.conf file from a doxygen.conf.in, adding a TAGFILES option containing
    all of the doxygen tag files from dependent products."""
    dest, garbage = os.path.splitext(src)
    return env.Command(dest, src, action=BuildDoxygenConfig)

SConsEnvironment.DoxygenConfig = DoxygenConfig

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def BPDox(env, src, *dependencies):
    """Generate a Boost.Python C++ source using bpdox (part of the bputils package).
    """
    packages = [env["eups_product"]]
    for item in dependencies:
        packages.extend(Split(item))
    src = File(src)
    return env.Command(src.target_from_source("", ""), src, 
                       action="bpdox $SOURCE {0}".format(" ".join(packages)))

SConsEnvironment.BPDox = BPDox

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def _tryImport(upsDirs, conf, products, name, required=True):
    try:
        for path in upsDirs:
            filename = os.path.join(path, name + ".cfg")
            if os.path.exists(filename):
                module = imp.load_source(name + "_cfg", filename)
                break
        else:
            raise ImportError()
        return module
    except ImportError:
        sys.stderr.write("Failed to import configuration module for '%s'.\n" % name)
        products[name] = None
        if required:
            Exit(1)
        else:
            return

def _trySetup(upsDirs, module, conf, products, name, required, build):
    try:
        sys.stdout.write("Setting up package %s... " % name)
        products[name] = module.setup(conf, products, build)
        sys.stdout.write("yes\n")
    except Exception as err:
        sys.stdout.write("no\n")
        sys.stderr.write("Exception encountered processing configuration module for '%s':\n" % name)
        sys.stderr.write(str(err) + "\n")
        products[name] = None
        if required:
            Exit(1)

def _recursiveConfigure(upsDirs, conf, products, name, required):
    """Setup the given product as a dependency."""
    if name in products:
        if products[name] is None and required:
            Exit(1)
        else:
            return
    module = _tryImport(upsDirs, conf, products, name, required)
    if module is None:
        return
    try:
        for dependency in module.dependencies["required"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required)
        for dependency in module.dependencies["optional"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required=False)
    except AttributeError:
        sys.stderr.write("No dependencies found while configuring '%s':\n" % name)
        products[name] = None
        if required:
            Exit(1)
        return
    _trySetup(upsDirs, module, conf, products, name, required, build=False)

def configureProducts(env, upsDirs):
    """Setup env['eups_product'] to be built, including recursively configuring dependencies."""
    conf = env.Configure()
    products = {}
    module = _tryImport(upsDirs, conf, products, env["eups_product"], required=True)
    try:
        for dependency in module.dependencies["required"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required=True)
        for dependency in module.dependencies["buildRequired"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required=True)
        for dependency in module.dependencies["optional"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required=False)
        for dependency in module.dependencies["buildOptional"]:
            _recursiveConfigure(upsDirs, conf, products, dependency, required=False)
    except AttributeError:
        sys.stderr.write("No dependencies found while configuring '%s':\n" % name)
        products[name] = None
        if required:
            Exit(1)
        return
    _trySetup(upsDirs, module, conf, products, env["eups_product"], required=True, build=True)
    env = conf.Finish()
    env['products'] = products
    return env
