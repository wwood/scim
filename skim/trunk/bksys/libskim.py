# Copyright LiuCougar 2005
# BSD license (see COPYING)

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

"""
This tool is used to find and load the libskim
neceessary compilation and link flags
"""
def exists(env):
	return true

def generate(env):
	import SCons.Util, os
	
	REQUIRED_LIBSKIM_VERSION = env['REQUIRED_LIBSKIM_VERSION']

	def SKIMinstall(env, restype, subdir, files):
		if not env['_INSTALL']:
				return
		basedir=env['DESTDIR']
		if len(restype)>0:
				if not env.has_key(restype):
						print RED+"unknown resource type "+restype+NORMAL
				else:
						basedir += env[restype]+'/'
		else:
			print "You must give a restype for SKIMinstall"
			env.exit(1)
		#print file # <- useful to trace stuff :)
		skim_install_list =  env.Install(basedir+subdir+'/', files)
		env.Alias('install', skim_install_list)
		return skim_install_list

	def SKIMshlib(env, restype, target, source):
		""" Makes a shared library for SKIM (and corresponding .la file)
		The library is installed except if one sets env['NOAUTOINSTALL']. 
		this function will automatically add necessary compile and link 
		parameters of libskim"""
		env.AppendUnique(CCFLAGS = env['SCIM_CFLAGS'] )
		env.AppendUnique(LINKFLAGS = env['SCIM_LIBS'] )

		if not env.has_key('BUILDING_SKIM')
			env.AppendUnique(CCFLAGS = env['SKIM_CFLAGS'] )
			env.AppendUnique(LINKFLAGS = env['SKIM_LIBS'] )
		else
			env.KDEaddpaths_includes( "#/src #/utils" )
		
		library_list = env.bksys_shlib(target, source, env[restype])
		return library_list


	# these are our options
	from SCons.Options import Options, PathOption
	cachefile = env['CACHEDIR']+'/skim.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'SKIM_ISCONFIGURED', 'whether it is necessary to run configure or not' ),
		( 'SKIM_VERSION', 'The version of installed skim' ),
		( 'SKIM_LIBDIR', 'The libdir of installed skim' ),
		( 'SKIM_CFLAGS', 'additional compilation flags from libskim' ),
		( 'SKIM_LIBS', 'additional link flags from libskim' ),
		( 'SKIM_KDEUTILS_CFLAGS', 'additional compilation flags from libskim-x11utils' ),
		( 'SKIM_KDEUTILS_LIBS', 'additional link flags from libskim-x11utils' ),
		( 'HAS_XCOMPOSITE', 'Whether XComposite extension is available'),
		)
	opts.Update(env)

	# detect the skim packages when needed
	if not env.has_key('BUILDING_SKIM') and not env.has_key('SKIM_ISCONFIGURED'):
		if not env.has_key('PKGCONFIG'):
			print 'you should add pkg module to the Environment constructor'
			env.Exit(1) 

		conf = env.Configure() 

		# delete the cached variables
		for var in "SKIM_VERSION SKIM_LIBDIR SKIM_CFLAGS SKIM_LIBS SKIM_KDEUTILS_CFLAGS SKIM_KDEUTILS_LIBS HAS_XCOMPOSITE".split():
			if env.has_key(var): env.__delitem__(var)

		have_skim = conf.PKGcheckmodules('SKIM', 'skim >= ' + REQUIRED_LIBSKIM_VERSION)
		have_skim_x11utils = conf.PKGcheckmodules('SKIM_X11', 'skim-x11utils')
		
		env = conf.Finish()
		# if the config worked, read the necessary variables and cache them
		if not have_skim:
			print RED+'skim >= ' + REQUIRED_LIBSKIM_VERSION + ' was not found (mandatory).'+NORMAL
			env.Exit(1)
		# mark the config as done - libxml2 and libxslt are found
		env['SKIM_ISCONFIGURED'] = 1

		env['SKIM_VERSION'] = os.popen(env['PKGCONFIG'] + " --modversion skim").read().strip()
		env['SKIM_BINARY_VERSION'] = os.popen(env['PKGCONFIG'] + " --variable=skim_binary_version skim").read().strip()
		env['SKIM_LIBDIR'] = os.popen(env['PKGCONFIG'] + " --variable=libdir skim").read().strip()
		
		env['SKIMCONFIGLIB_VERSIONED'] = env['SKIM_LIBDIR'] + "/skim-1.0/" + \
			 env['SKIM_BINARY_VERSION'] + "/Config"

	# Attach the functions to the environment so that sconscripts or other modules can use them
	from SCons.Script.SConscript import SConsEnvironment
	SConsEnvironment.SKIMinstall = SKIMinstall
	SConsEnvironment.SKIMshlib = SKIMshlib
