#! /usr/bin/env python

import SCons.Util, os

def exists(env):
	return true

def generate(env):

	if env['HELP']:
		p=env.pprint
		p('BOLD','*** wxWidgets options ***')
		p('BOLD','--------------------')
		p('BOLD','* with-wx-config    ','wx-config program to use (ie: wx-config-2.6)')
		p('BOLD','ie: scons configure with-wx-config=wx-config-2.6 ')
		return

	def Check_wx_config(context, version, wx_config_command = "wx-config"):
		context.Message('Checking for %s ... ' % wx_config_command)
		if os.environ.has_key("WX_CONFIG_PATH"):
			wx_config_command = "WX_CONFIG_PATH=" + os.environ["WX_CONFIG_PATH"] + " " + wx_config_command
		ret = os.popen(wx_config_command + ' --version').read().split('.')
		if ((int(ret[0]) >= version[0]) and (int(ret[1]) >= version[1]) and (int(ret[2]) >= version[2])):
			#env.ParseConfig(wx_config_command + ' --cflags --libs');
			
			
			
			#--gl-libs
			#--libs std,gl
			
			env.AppendUnique( WX_CCFLAGS = 
				SCons.Util.CLVar( 
					os.popen(wx_config_command+" --cflags 2>/dev/null").read().strip() ));
			env.AppendUnique( WX_LDFLAGS = 
				SCons.Util.CLVar( 
					os.popen(wx_config_command+" --libs std,gl 2>/dev/null").read().strip() ));
			context.Result(True)
			return True
		else:
			context.Result(False)
			return False

	# load the options
	from SCons.Options import Options, PathOption
	cachefile = env['CACHEDIR']+'/wx.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'CACHED_WX', 'debug level for the project : full or just anything' ),
		( 'WX_CCFLAGS', 'additional compilation flags' ),
		( 'WX_LDFLAGS', 'additional link flags' )
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_WX')):
		## Configure stuff
		conf = env.Configure(custom_tests = { 'Check_wx_config' : Check_wx_config })

		if env.has_key('WX_CCFLAGS'):   env.__delitem__('WX_CCFLAGS')
		if env.has_key('WX_LDFLAGS'):   env.__delitem__('WX_LDFLAGS')
		if env.has_key('CACHED_WX'): env.__delitem__('CACHED_WX')

		if 'with-wx-config' in env['ARGS']:
			if not conf.Check_wx_config([2,6,1], env['ARGS']['with-wx-config']):
				print 'wxWidgets >= 2.6.1 is required.'
				env.Exit(1)
		else:
			if not conf.Check_wx_config([2,6,1]):
				print 'wxWidgets >= 2.6.1 is required.'
				env.Exit(1)

		env = conf.Finish()

		env['CACHED_WX'] = 1
		opts.Save(cachefile, env)

	if env.has_key('WX_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['WX_CCFLAGS'] )
		#env.AppendUnique(CXXFLAGS = env['WX_CCFLAGS'] )
	if env.has_key('WX_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['WX_LDFLAGS'] )
