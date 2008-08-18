# Copyright LiuCougar 2005
# BSD license (see COPYING)

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

"""
This tool is used to find and load the libscim and libscim_x11
neceessary compilation and link flags
"""
def exists(env):
	return true

def generate(env):
	import SCons.Util, os
	
	if env['HELP']:
		return
	
	REQUIRED_LIBSCIM_VERSION = env['REQUIRED_LIBSCIM_VERSION']

	def SCIMinstallpath(env, restype, versioned):
		if restype == 'SCIMCONFIGLIB':
			if versioned :
				return env['SCIM_LIBDIR'] + "/scim-1.0/" + \
				env['SCIM_BINARY_VERSION'] + "/Config"
			else :
				return env['SCIM_LIBDIR'] + "/scim-1.0/Config"
		return ''

	def SCIMinstall(env, restype, subdir, files):
		if not env['_INSTALL']:
				return
		basedir=env['DESTDIR']
		if len(restype)>0:
				if not env.has_key(restype):
						print RED+"unknown resource type "+restype+NORMAL
				else:
						basedir += env[restype]+'/'
		else:
			print "You must give a restype for SCIMinstall"
			env.exit(1)
		#print file # <- useful to trace stuff :)
		scim_install_list =  env.Install(basedir+subdir+'/', files)
		env.Alias('install', scim_install_list)
		return scim_install_list

	def SCIMshlib(env, restype, target, source, versioned = 0, localshlibs = ""):
		""" Makes a shared library for SCIM (and corresponding .la file)
		The library is installed except if one sets env['NOAUTOINSTALL']. 
		this function will automatically add necessary compile and link 
		parameters of libscim"""
		env.AppendUnique(CCFLAGS = env['SCIM_CFLAGS'] )
		env.AppendUnique(LINKFLAGS = env['SCIM_LIBS'] )
                # we link the program against a shared library done locally, add the dependency
                lst=[]
                if len(localshlibs)>0:
                        lst=env.make_list(localshlibs)
                        env.link_local_shlib(lst)
		
		library_list = env.bksys_shlib(target, source, SCIMinstallpath(env,restype,versioned), '')
                if len(lst)>0: env.Depends( library_list, lst )
		return library_list

	def Add_define(env, name):
		if env.has_key(name):
				env.AppendUnique(CCFLAGS = '-D' + name + '=\\"' + env[name] + '\\"' )
		return

	# these are our options
	from SCons.Options import Options, PathOption
	cachefile = env['CACHEDIR']+'/scim.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'SCIM_ISCONFIGURED', 'whether it is necessary to run configure or not' ),
		( 'SCIM_VERSION', 'The version of installed scim' ),
		( 'SCIM_BINARY_VERSION', 'The binary version of installed scim' ),
		( 'SCIM_LIBDIR', 'The libdir of installed scim' ),
		( 'SCIM_CFLAGS', 'additional compilation flags from libscim' ),
		( 'SCIM_LIBS', 'additional link flags from libscim' ),
		( 'SCIM_X11_CFLAGS', 'additional compilation flags from libscim-x11utils' ),
		( 'SCIM_X11_LIBS', 'additional link flags from libscim-x11utils' ),
		( 'SCIM_ICONDIR', 'install path for scim icons' ),
		( 'ENABLE_DEBUG', ''),
		)
	opts.Update(env)

	# detect the scim packages when needed
	if not env.has_key('SCIM_ISCONFIGURED'):
		if not env.has_key('PKGCONFIG'):
			print 'you should add pkg module to the Environment constructor'
			env.Exit(1) 

		conf = env.Configure(custom_tests = { 'PKGcheckmodules' : env.PKGcheckmodules }) 

		# delete the cached variables
		if env.has_key('SCIM_VERSION'):
			env.__delitem__('SCIM_VERSION')
		if env.has_key('SCIM_BINARY_VERSION'):
			env.__delitem__('SCIM_BINARY_VERSION')
		if env.has_key('SCIM_LIBDIR'):
			env.__delitem__('SCIM_LIBDIR')
		if env.has_key('SCIM_CFLAGS'):
			env.__delitem__('SCIM_CFLAGS')
		if env.has_key('SCIM_LIBS'):
			env.__delitem__('SCIM_LIBS')
		if env.has_key('SCIM_X11_CFLAGS'):
			env.__delitem__('SCIM_X11_CFLAGS')
		if env.has_key('SCIM_X11_LIBS'):
			env.__delitem__('SCIM_X11_LIBS')
		if env.has_key('ENABLE_DEBUG'):
			env.__delitem__('ENABLE_DEBUG')

		have_scim = conf.PKGcheckmodules('SCIM', 'scim >= ' + REQUIRED_LIBSCIM_VERSION)
		have_scim_x11utils = conf.PKGcheckmodules('SCIM_X11', 'scim-x11utils')

		env = conf.Finish()
		# if the config worked, read the necessary variables and cache them
		if not have_scim:
			print RED+'scim >= ' + REQUIRED_LIBSCIM_VERSION + ' was not found (mandatory).'+NORMAL
			print 'Perhaps you should add the directory containing "scim.pc"\nto the PKG_CONFIG_PATH environment variable'
			env.Exit(1)
		
		if env['ARGS'].get('debug', None):
			env['ENABLE_DEBUG'] = 1

		env['SCIM_ISCONFIGURED'] = 1

		env['SCIM_VERSION'] = os.popen(env['PKGCONFIG'] + " --modversion scim").read().strip()
		env['SCIM_BINARY_VERSION'] = os.popen(env['PKGCONFIG'] + " --variable=scim_binary_version scim").read().strip()
		env['SCIM_LIBDIR'] = os.popen(env['PKGCONFIG'] + " --variable=libdir scim").read().strip()
		env['SCIM_ICONDIR'] = env.join(os.popen(env['PKGCONFIG'] + " --variable=datadir scim").read().strip(), '/scim/icons')

		# store the config
		opts.Save(cachefile, env)

	if env.has_key('ENABLE_DEBUG'):
		env.AppendUnique(CXXFLAGS=['-DENABLE_DEBUG=1'])
	else:
		env.AppendUnique(CXXFLAGS=['-DENABLE_DEBUG=0'])

	# Attach the functions to the environment so that sconscripts or other modules can use them
	from SCons.Script.SConscript import SConsEnvironment
	SConsEnvironment.Add_define = Add_define
	SConsEnvironment.SCIMinstall = SCIMinstall
	SConsEnvironment.SCIMshlib = SCIMshlib
