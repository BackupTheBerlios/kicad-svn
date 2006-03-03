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
		# Check if wx-config exists
		if not context.TryAction(wx_config_command + ' --version')[0]:
			context.Result(False)
			return False
		else:
			context.Result(True)
		# Check the wx-config version
		context.Message('Checking for %s version ... ' % wx_config_command)
		ret = os.popen(wx_config_command + ' --version').read().split('.')
		if ((int(ret[0]) >= version[0]) and (int(ret[1]) >= version[1]) and (int(ret[2]) >= version[2])):
			env.AppendUnique( WX_CCFLAGS = 
				SCons.Util.CLVar( 
					os.popen(wx_config_command+" --cflags 2>/dev/null").read().strip() ));
			# To test unicode add --unicode=yes
			env.AppendUnique( WX_LDFLAGS = 
				SCons.Util.CLVar( 
					os.popen(wx_config_command+" --libs std,gl 2>/dev/null").read().strip() ));
			context.Result(True)
			return True
		else:
			context.Result(False)
			return False
	
	def CheckGL(context):
		context.Message( 'Checking for wxGL ...' )
		context.env.AppendUnique(CCFLAGS = env['WX_CCFLAGS'] )
		context.env.AppendUnique(LINKFLAGS = env['WX_LDFLAGS'] )
		ret = context.TryLink("""
#include <wx/glcanvas.h>
int main(int argc, char **argv) { 
	wxGLCanvas *c = new wxGLCanvas(new wxWindow());
	return 0;
}
""", '.cpp')
		context.Result( ret )
		return ret
	
	#def CheckUnicode(context):
		#context.Message( 'Checking for unicode support ...' )
		#context.env.AppendUnique(CCFLAGS = env['WX_CCFLAGS'] )
		#context.env.AppendUnique(LINKFLAGS = env['WX_LDFLAGS'] )
		#ret = context.TryLink("""
##define wxUSE_UNICODE 1
##include <wx/string.h>
#int main(int argc, char **argv) { 
##ifdef __UNICODE__
	#const char* ascii_str = "Some text";
	#wxString str(ascii_str, wxConvUTF8);
##else
	##error "Unicode support not enabled"
##end
	#return 0;
#}
#""", '.cpp')
		#context.Result( ret )
		#return ret
	
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
		conf = env.Configure(custom_tests = { 'Check_wx_config' : Check_wx_config,
											  'CheckGL' : CheckGL,
											  #'CheckUnicode' : CheckUnicode
											   })

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

		if not conf.CheckGL():
			print 'wxWidgets was not compiled with OpenGL support'
			env.Exit(1)
		# TODO : Add a good unicode detection. Currently, if the --unicode=yes is added and
		# wxWidgets was not compiled with unicode, even the OpenGL test failed because the 
		# unicode lib cannot be found
		#if not conf.CheckUnicode():
		#	print 'wxWidgets was not compiled with unicode support'

		env = conf.Finish()

		env['CACHED_WX'] = 1
		opts.Save(cachefile, env)

	if env.has_key('WX_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['WX_CCFLAGS'] )
		#env.AppendUnique(CXXFLAGS = env['WX_CCFLAGS'] )
	if env.has_key('WX_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['WX_LDFLAGS'] )
