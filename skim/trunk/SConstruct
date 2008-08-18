#! /usr/bin/env python

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## Load the builders in config

##helper function
import types
VERSION = "1.4.5"
PACKAGE = "skim"
REQUIRED_SCIM_VERSION='1.4.4'
REQUIRED_KDE_VERSION='3.2.0'

# increment if the interface has additions, changes, removals.
SKIM_CURRENT=1

# increment any time the source changes; set to 0 if you increment CURRENT
SKIM_REVISION=0

# increment if any interfaces have been added; set to 0
# if any interfaces have been removed. removal has
# precedence over adding, so set to 0 if both happened.
SKIM_AGE=1

#def Add_define(env, name):
#	if env.has_key(name):
#		env.AppendUnique(CCFLAGS = '-D' + name + '=\\"' + env[name] + '\\"' )
#	return

def libskim_version(env):
	return str(env['SKIM_CURRENT']-env['SKIM_AGE']) + '.' + str(env['SKIM_AGE']) + '.' + str(env['SKIM_REVISION'])
	
def generate_configed_file(env1, target, source, env):
	path_defines = {
		# this is where you put all of your custom configuration values
		"prefix"        : env['PREFIX'],
		"bindir"        : env['KDEBIN'],
		"exec_prefix"   : env['KDEBIN'],
		"libdir"        : env['KDELIB'],
		"includedir"    : env['KDEINCLUDE'],
		"localedir"     : env['KDELOCALE'],
		"datadir"       : env['KDEDATA'],
		"sysconfdir"    : env['PREFIX'] + "/etc",
		"VERSION"       : VERSION,
		"PACKAGE"       : PACKAGE,
		"REQUIRED_SCIM_VERSION" : REQUIRED_SCIM_VERSION,
		"REQUIRED_KDE_VERSION"  : REQUIRED_KDE_VERSION,
		"SCIM_LIBDIR"   : env['SCIM_LIBDIR'],
		"SCIM_BINARY_VERSION"   : env['SCIM_BINARY_VERSION'],
	}

	import re, string
	MakeToBksysVariableReg = re.compile('\@([a-zA-Z_]+)\@')
	for a_target, a_source in zip(target, source):
		pcfile = file(str(a_target), "w")
		pcfile_in = file(str(a_source), "r")
		#first protect any existing % symbols
		pcfile_in_string = string.replace(pcfile_in.read(), '%', '%%')
		pcfile_in_string = MakeToBksysVariableReg.sub(r'%(\1)s',pcfile_in_string)
		pcfile.write(pcfile_in_string % path_defines)
		pcfile_in.close()
		pcfile.close()

def SKIMPLUGIN(env, target_name, sources, desktop_file):
	lenv=env.Copy()
	lenv.AppendUnique(CCFLAGS = env['SCIM_CFLAGS'] )
	lenv.AppendUnique(LINKFLAGS = env['SCIM_LIBS'] )
	lenv.KDEaddpaths_includes( "#/src #/utils" )
	
	lenv.KDEshlib(target_name , sources, 0, '', '', 
		'#/src/libskim.so #/utils/libscim-kdeutils.so' )
	
	lenv.KDEaddlibs( "qt-mt kdecore kdeui" )
	lenv.KDEinstall( 'KDESERV', '', desktop_file)

def SKIMKCMPLUGIN(env, target_name, sources, desktop_file):
	lenv=env.Copy()
	lenv.AppendUnique(CCFLAGS = env['SCIM_CFLAGS'] )
	lenv.AppendUnique(LINKFLAGS = env['SCIM_LIBS'] )
	lenv.KDEaddpaths_includes( "#/src #/utils" )
	lenv.KDEshlib(target_name , sources, kdelib=0, libprefix='', includes='',
		localshlibs='#/src/libskim.la #/utils/libscim-kdeutils.la' )
	lenv.KDEaddlibs( "qt-mt kdecore kdeui kutils" )
	lenv.KDEinstall( 'KDESERV', 'skimconfiguredialog', desktop_file)

from SCons.Script.SConscript import SConsEnvironment
#SConsEnvironment.Add_define = Add_define
SConsEnvironment.libskim_version = libskim_version
SConsEnvironment.generate_configed_file = generate_configed_file
SConsEnvironment.SKIMPLUGIN = SKIMPLUGIN
SConsEnvironment.SKIMKCMPLUGIN = SKIMKCMPLUGIN

env = Environment( REQUIRED_LIBSCIM_VERSION=REQUIRED_SCIM_VERSION, tools=['default', 'generic', 'kde', 'pkg', 'libscim'], toolpath=['./', './bksys'])

env['BUILDING_SKIM']=1

env['VERSION']=VERSION
env['PACKAGE']=PACKAGE
env['APPNAME']=env['PACKAGE']

env['SKIM_CURRENT']=SKIM_CURRENT
env['SKIM_REVISION']=SKIM_REVISION
env['SKIM_AGE']=SKIM_AGE

env.AppendUnique(CCFLAGS = '-DNO_CONFIG_H')	

env.KDEuse("environ rpath thread")
		
#env.KDEuse("environ rpath lang_qt thread nohelp")

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################

env.Command('skim.pc', 'skim.pc.in', env.generate_configed_file)
env.Command('scim_kdeutils.pc', 'scim_kdeutils.pc.in', env.generate_configed_file)
env.Command('skim.spec', 'skim.spec.in', env.generate_configed_file)

env.bksys_install( env['PCFILE_PATH'],  "skim.pc scim_kdeutils.pc" )

## check X extension Xcomposite START
# from SCons.Options import Options, PathOption
# cachefile = env['CACHEDIR']+'/skim_.cache.py'
# opts = Options(cachefile)
# opts.AddOptions(
# 	( 'SKIMSELF_ISCONFIGURED', 'whether it is necessary to run configure or not'),
# 	( 'HAS_XCOMPOSITE', 'Whether XComposite extension is available'),
# 	)
# opts.Update(env)
# 
# if not env.has_key('SKIMSELF_ISCONFIGURED'):
# 	# delete the cached variables
# 	for var in "SKIMSELF_ISCONFIGURED HAS_XCOMPOSITE".split():
# 		if env.has_key(var): env.__delitem__(var)
# 	conf = env.Configure() 
# 	if conf.CheckCHeader('X11/extensions/Xcomposite.h'):
# 		HAS_XCOMPOSITE=1
# 	else:
# 		print 'Xcomposite.h and/or libXcomposite can not be found!'
# 		HAS_XCOMPOSITE=0
# 	env = conf.Finish()
# 	env['SKIMSELF_ISCONFIGURED']=HAS_XCOMPOSITE

## check X extension Xcomposite END

dirs="""
utils
scim/config
src
plugins/compmgrclient
plugins/inputwindow
plugins/kdetraystatus
plugins/mainwindow
plugins/mainwindow/applet
plugins/scimlauncher
plugins/setupwindow
icons
doc
po
"""
env.subdirs(dirs)

## As an example, i have left the SConscript here out of the subdirs above :)

## Here is a first example
#env.SConscript('test8-kate/SConscript')

## Icons are handled automatically by KDEicon - have a look
#env.SConscript('icons/SConscript')

## We are now using one top-level file for the documentation instead of several almost empty scripts
#env.SConscript('doc/SConscript')

## Process the translations in the po/ directory
#env.KDElang('po/', 'bksys')

## Demonstration of the error handling
#env.SConscript("error_handling/SConscript")

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

### To make a tarball of your masterpiece, use 'scons dist'
if 'dist' in COMMAND_LINE_TARGETS:

	## The target scons dist requires the python module shutil which is in 2.3
	env.EnsurePythonVersion(2, 3)

	import os
	APPNAME = env['APPNAME']
	VERSION = env['VERSION']
	FOLDER  = APPNAME+'-'+VERSION
	ARCHIVE = FOLDER+'.tar.bz2'
	TMPFOLD = ".tmp"+FOLDER

	import shutil
	import glob

        ## check if the temporary directory already exists
        for dir in [FOLDER, TMPFOLD, ARCHIVE]:
                if os.path.isdir(dir):
                        shutil.rmtree(dir)

        ## create a temporary directory
        startdir = os.getcwd()

        os.popen("mkdir -p "+TMPFOLD)
        os.popen("cp -R * "+TMPFOLD)
        os.popen("mv "+TMPFOLD+" "+FOLDER)

        ## remove scons-local if it is unpacked
        os.popen("rm -rf "+FOLDER+"/scons "+FOLDER+"/sconsign "+FOLDER+"/scons-local-0.96.1")

        ## remove our object files first
        os.popen("find "+FOLDER+" -name \"*cache*\" | xargs rm -rf")
        os.popen("find "+FOLDER+" -name \"*.pyc\" | xargs rm -f")
	## CVS cleanup
	os.popen("find "+FOLDER+" -name \"CVS\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \".cvsignore\" | xargs rm -rf")

	## Subversion cleanup
	os.popen("find %s -name .svn -type d | xargs rm -rf" % FOLDER)

	## GNU Arch cleanup
	os.popen("find "+FOLDER+" -name \"{arch}\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \".arch-i*\" | xargs rm -rf")

        ## Auto* tool and Makefile cleanup
	os.popen("find "+FOLDER+" -name \"Makefile.am\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \"configure.in.in\" | xargs rm -rf")

	cleanfile = "scim_panel_kde.kdevses scim_panel_kde.kdevelop.pcs qt_downgrade templates fixuifiles config.log admin bootstrap.make Makefile.cvs mkchlog bksys/libskim.py bksys/qt4.py"
	os.popen("cd "+FOLDER+"; rm -fr "+cleanfile+"; cd -")

	## Create the tarball (coloured output)
	print "\033[92m"+"Writing archive "+ARCHIVE+"\033[0m"
	os.popen("tar cjf "+ARCHIVE+" "+FOLDER)

	## Remove the temporary directory
	if os.path.isdir(FOLDER):
		shutil.rmtree(FOLDER)

	env.Default(None)
        env.Exit(0)

### Emulate "make distclean"
if 'distclean' in COMMAND_LINE_TARGETS:
	## Remove the cache directory
	import os, shutil
	if os.path.isdir(env['CACHEDIR']):
		shutil.rmtree(env['CACHEDIR'])
	os.popen("find . -name \"*.pyc\" | xargs rm -rf")

	env.Default(None)
	env.Exit(0)

