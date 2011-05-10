# -*- python -*-
#
# Setup our environment
#
import glob, os.path, re, os
import lsst.scons.SConsUtils as scons

env = scons.makeEnv(
    "pex_exceptions",
    r"$HeadURL$",
    scons.ConfigureDependentProducts("pex_exceptions"))

#
# Build/install things
#
for d in Split("examples lib python/lsst/pex/exceptions tests doc"):
    SConscript(os.path.join(d, "SConscript"))

env['IgnoreFiles'] = r"(~$|\.pyc$|^\.svn$|\.o$)"

env.InstallLSST(env['prefix'], ["python", "etc", "include", "lib", "doc", "ups"])

scons.CleanTree(r"*~ core *.so *.os *.o")

#
# Build TAGS files
#
files = scons.filesToTag()
if files:
    env.Command("TAGS", files, "etags -o $TARGET $SOURCES")

env.Declare()
env.Help("""
LSST Pipeline Execution Exceptions package
""")

