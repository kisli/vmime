#
# SConstruct
# libvmime build script
#
# Process this file with 'scons' to build the project.
# For more information, please visit: http://www.scons.org/ .
#
# Usage:
#
#   . scons               build the library
#   . scons -h            see available configuration options
#   . scons opt=value     set a configuration option
#   . scons install       install library and include files (as root)
#   . scons dist          build a source package (.tar.bz2)
#   . scons doc           build documentation for the project (Doxygen)
#

import commands
import os
import sys
import re
import string


##################
#  Source files  #
##################

libvmime_sources = [
	'address.cpp', 'address.hpp',
	'addressList.cpp', 'addressList.hpp',
	'attachment.hpp',
	'base.cpp', 'base.hpp',
	'body.cpp', 'body.hpp',
	'bodyPart.cpp', 'bodyPart.hpp',
	'charset.cpp', 'charset.hpp',
	'component.cpp', 'component.hpp',
	'constants.cpp', 'constants.hpp',
	'contentDispositionField.cpp', 'contentDispositionField.hpp',
	'contentHandler.cpp', 'contentHandler.hpp',
	'contentTypeField.cpp', 'contentTypeField.hpp',
	'dateTime.cpp', 'dateTime.hpp',
	'defaultAttachment.cpp', 'defaultAttachment.hpp',
	'disposition.cpp', 'disposition.hpp',
	'encoder.cpp', 'encoder.hpp',
	'encoder7bit.cpp', 'encoder7bit.hpp',
	'encoder8bit.cpp', 'encoder8bit.hpp',
	'encoderB64.cpp', 'encoderB64.hpp',
	'encoderBinary.cpp', 'encoderBinary.hpp',
	'encoderDefault.cpp', 'encoderDefault.hpp',
	'encoderFactory.cpp', 'encoderFactory.hpp',
	'encoderQP.cpp', 'encoderQP.hpp',
	'encoderUUE.cpp', 'encoderUUE.hpp',
	'encoding.cpp', 'encoding.hpp',
	'exception.cpp', 'exception.hpp',
	'fileAttachment.cpp', 'fileAttachment.hpp',
	'genericField.hpp',
	'genericParameter.hpp',
	'header.cpp', 'header.hpp',
	'headerFieldFactory.cpp', 'headerFieldFactory.hpp',
	'headerField.cpp', 'headerField.hpp',
	'htmlTextPart.cpp', 'htmlTextPart.hpp',
	'mailbox.cpp', 'mailbox.hpp',
	'mailboxField.cpp', 'mailboxField.hpp',
	'mailboxGroup.cpp', 'mailboxGroup.hpp',
	'mailboxList.cpp', 'mailboxList.hpp',
	'mediaType.cpp', 'mediaType.hpp',
	'messageBuilder.cpp', 'messageBuilder.hpp',
	'message.cpp', 'message.hpp',
	'messageId.cpp', 'messageId.hpp',
	'messageParser.cpp', 'messageParser.hpp',
	'options.cpp', 'options.hpp',
	'parameter.cpp', 'parameter.hpp',
	'parameterFactory.cpp', 'parameterFactory.hpp',
	'parameterizedHeaderField.cpp', 'parameterizedHeaderField.hpp',
	'parserHelpers.hpp',
	'plainTextPart.cpp', 'plainTextPart.hpp',
	'platformDependant.cpp', 'platformDependant.hpp',
	'propertySet.cpp', 'propertySet.hpp',
	'relay.cpp', 'relay.hpp',
	'standardFields.hpp',
	'standardParams.hpp',
	'text.cpp', 'text.hpp',
	'textPartFactory.cpp', 'textPartFactory.hpp',
	'textPart.hpp',
	'typeAdapter.cpp', 'typeAdapter.hpp',
	'types.hpp',
	'word.cpp', 'word.hpp',
	'vmime',
	'utility/file.hpp',
	'utility/md5.cpp', 'utility/md5.hpp',
	'utility/path.cpp', 'utility/path.hpp',
	'utility/random.cpp', 'utility/random.hpp',
	'utility/singleton.cpp', 'utility/singleton.hpp',
	'utility/smartPtr.hpp',
	'utility/stream.cpp', 'utility/stream.hpp',
	'utility/stringProxy.cpp', 'utility/stringProxy.hpp',
	'utility/stringUtils.cpp', 'utility/stringUtils.hpp'
]

libvmime_examples_sources = [
	'examples/common.inc',
	'examples/README',
	'examples/example1.cpp',
	'examples/example2.cpp',
	'examples/example3.cpp',
	'examples/example4.cpp',
	'examples/example5.cpp',
	'examples/example6.cpp'
]

libvmime_messaging_sources = [
	'messaging/authenticator.cpp', 'messaging/authenticator.hpp',
	'messaging/authenticationInfos.cpp', 'messaging/authenticationInfos.hpp',
	'messaging/authHelper.cpp', 'messaging/authHelper.hpp',
	'messaging/builtinServices.inl',
	'messaging/defaultAuthenticator.cpp', 'messaging/defaultAuthenticator.hpp',
	'messaging/events.cpp', 'messaging/events.hpp',
	'messaging/folder.cpp', 'messaging/folder.hpp',
	'messaging/message.cpp', 'messaging/message.hpp',
	'messaging/progressionListener.hpp',
	'messaging/service.cpp', 'messaging/service.hpp',
	'messaging/serviceFactory.cpp', 'messaging/serviceFactory.hpp',
	'messaging/serviceInfos.hpp',
	'messaging/session.cpp', 'messaging/session.hpp',
	'messaging/simpleAuthenticator.cpp', 'messaging/simpleAuthenticator.hpp',
	'messaging/socket.hpp',
	'messaging/store.hpp',
	'messaging/timeoutHandler.hpp',
	'messaging/transport.hpp',
	'messaging/url.cpp', 'messaging/url.hpp',
	'messaging/urlUtils.cpp', 'messaging/urlUtils.hpp'
]

libvmime_messaging_proto_sources = [
	[
		'pop3',
		[
			'messaging/POP3Store.cpp',       'messaging/POP3Store.hpp',
			'messaging/POP3Folder.cpp',      'messaging/POP3Folder.hpp',
			'messaging/POP3Message.cpp',     'messaging/POP3Message.hpp'
		]
	],
	[
		'smtp',
		[
			'messaging/SMTPTransport.cpp',   'messaging/SMTPTransport.hpp'
		]
	],
	[
		'imap',
		[
			'messaging/IMAPConnection.cpp',  'messaging/IMAPConnection.hpp',
			'messaging/IMAPStore.cpp',       'messaging/IMAPStore.hpp',
			'messaging/IMAPFolder.cpp',      'messaging/IMAPFolder.hpp',
			'messaging/IMAPMessage.cpp',     'messaging/IMAPMessage.hpp',
			'messaging/IMAPTag.cpp',         'messaging/IMAPTag.hpp',
			'messaging/IMAPUtils.cpp',       'messaging/IMAPUtils.hpp',
			'messaging/IMAPParser.hpp'
		]
	],
	[
		'maildir',
		[
			'messaging/maildirStore.cpp',    'messaging/maildirStore.hpp',
			'messaging/maildirFolder.cpp',   'messaging/maildirFolder.hpp',
			'messaging/maildirMessage.cpp',  'messaging/maildirMessage.hpp',
			'messaging/maildirUtils.cpp',    'messaging/maildirUtils.hpp'
		]
	]
]

libvmime_platforms_sources = {
	'posix':
	[
		'platforms/posix/file.cpp', 'platforms/posix/file.hpp',
		'platforms/posix/handler.cpp', 'platforms/posix/handler.hpp',
		'platforms/posix/socket.cpp', 'platforms/posix/socket.hpp'
	]
}

libvmime_extra = [
	'AUTHORS',
	'ChangeLog',
	'COPYING',
	'INSTALL',
	'NEWS',
	'README',
	'SConstruct',
	'VERSION'
]

libvmime_tests = [
	# charset
	'tests/charset/Makefile',
	'tests/charset/main.cpp',
	'tests/charset/run-test.sh',
	'tests/charset/test-suites/gnu.in.utf-8',
	'tests/charset/test-suites/gnu.out.iso-8859-1',

	# main
	'tests/Makefile',
	'tests/run-tests.sh'
]

libunitpp_common = [
	'tests/lib/unit++/aclocal.m4',
	'tests/lib/unit++/COPYING',
	'tests/lib/unit++/guitester.cc',
	'tests/lib/unit++/main.h',
	'tests/lib/unit++/optmap-compat.h',
	'tests/lib/unit++/tester.h',
	'tests/lib/unit++/Test_unit++.cc',
	'tests/lib/unit++/unit++-compat.h',
	'tests/lib/unit++/Changelog',
	'tests/lib/unit++/guitester.h',
	'tests/lib/unit++/optmap.h',
	'tests/lib/unit++/unit++.1',
	'tests/lib/unit++/unit++.h',
	'tests/lib/unit++/gui.cc',
	'tests/lib/unit++/INSTALL',
	'tests/lib/unit++/Makefile.in',
	'tests/lib/unit++/Test_gui.cc',
	'tests/lib/unit++/unit++.3',
	'tests/lib/unit++/configure.ac',
	'tests/lib/unit++/gui.h',
#	'tests/lib/unit++/main.cc',
#	'tests/lib/unit++/optmap.cc',
#	'tests/lib/unit++/tester.cc',
	'tests/lib/unit++/Test_optmap.cc',
#	'tests/lib/unit++/unit++.cc',
	'tests/lib/unit++/unitpp.m4'
]

libunitpp_sources = [
	'tests/lib/unit++/unit++.cc',
	'tests/lib/unit++/main.cc',
	'tests/lib/unit++/optmap.cc',
	'tests/lib/unit++/tester.cc'
]

libvmimetest_common = [
	'tests/parser/testUtils.hpp'
]

libvmimetest_sources = [
	[ 'tests/parser/encoderTest', [ 'tests/parser/encoderTest.cpp' ] ],
	[ 'tests/parser/headerTest', [ 'tests/parser/headerTest.cpp' ] ],
	[ 'tests/parser/mailboxTest', [ 'tests/parser/mailboxTest.cpp' ] ],
	[ 'tests/parser/mediaTypeTest', [ 'tests/parser/mediaTypeTest.cpp' ] ],
	[ 'tests/parser/textTest', [ 'tests/parser/textTest.cpp' ] ]
]

libvmime_dist_files = libvmime_sources + libvmime_messaging_sources

for i in range(len(libvmime_dist_files)):
	libvmime_dist_files[i] = 'src/' + libvmime_dist_files[i]

for p in libvmime_messaging_proto_sources:
	for f in p[1]:
		libvmime_dist_files.append('src/' + f)

libvmime_dist_files = libvmime_dist_files + libvmime_extra + libvmime_examples_sources
libvmime_dist_files_with_tests = libvmime_dist_files + libvmime_tests

libvmime_dist_files = libvmime_dist_files + libunitpp_common
libvmime_dist_files = libvmime_dist_files + libunitpp_sources
libvmime_dist_files = libvmime_dist_files + libvmimetest_common
libvmime_dist_files = libvmime_dist_files + libvmimetest_sources


#################
#  Set options  #
#################

EnsureSConsVersion(0, 94)

SetOption('implicit_cache', 1)

#SourceSignatures('timestamp')
SourceSignatures('MD5')
TargetSignatures('build')


#############
#  Version  #
#############

def GetPackageVersion():
	import re
	contents = open('VERSION', 'r').read()
	major = re.compile('(\d+)\.(\d+)\.(\d+)', re.DOTALL).sub(r'\1', contents)
	minor = re.compile('(\d+)\.(\d+)\.(\d+)', re.DOTALL).sub(r'\2', contents)
	patch = re.compile('(\d+)\.(\d+)\.(\d+)', re.DOTALL).sub(r'\3', contents)
	return '%d.%d.%d' % (int(major), int(minor), int(patch))

packageName = 'libvmime'
packageVersion = GetPackageVersion()


#############
#  Options  #
#############

# Try to guess some default values
# TODO

# Command line options
opts = Options('options.cache')

opts.AddOptions(
	(
		'prefix',
		'Installation prefix directory',
		'/usr'
	),
	EnumOption(
		'debug',
		'Debug version (useful for developers only)',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'static',
		'Build a static library (.a)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'shared',
		'Build a shared library (.so)',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'with_messaging',
		'Messaging support (connection to mail store/transport servers)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'with_filesystem',
		'Enable file-system support (this is needed for "maildir" messaging support)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	(
		'with_messaging_protocols',
		'Specifies which protocols to build into the library.\n'
		    + 'This option has no effect if "with_messaging" is not activated.\n'
		    + 'Separate protocols with spaces; string must be quoted with ".\n'
		    + 'Available protocols: pop3, smtp, imap, maildir.',
		'"pop3 smtp imap"'
	),
	(
		'with_platforms',
		'Specifies which default platform handlers libraries to build.\n'
		    + 'This builds a library for each platform selected (eg: "libvmime-posix.a").\n'
		    + 'If no default handler is available for your platform, you will have\n'
		    + 'to write your own...\n'
		    + 'Separate platforms with spaces; string must be quoted with ".\n'
		    + 'Available platform handlers: posix.',
		'"posix"'
	),
	EnumOption(
		'with_wide_char_support',
		'Support for wide characters (rarely used, should be set to "no")',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'byte_order',
		'Byte order',
		sys.byteorder,
		allowed_values = ('big', 'little'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'pf_8bit_type',
		'The C-language 8-bit type for your platform',
		'char',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'pf_16bit_type',
		'The C-language 16-bit type for your platform',
		'short',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'pf_32bit_type',
		'The C-language 32-bit type for your platform',
		'int',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumOption(
		'build_tests',
		'Build unit tests (in "tests" directory)',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	)
)


###############################
#  Configuration Environment  #
###############################

env = Environment(options = opts)

env.Append(ENV = {'PATH' : os.environ['PATH']})

env.Append(CPPPATH = [ '.', 'src' ])

env.Append(CPPDEFINES = { '_REENTRANT' : 1 })

env.Append(CXXFLAGS = ['-pipe'])
env.Append(CXXFLAGS = ['-W'])
env.Append(CXXFLAGS = ['-Wall'])
env.Append(CXXFLAGS = ['-ansi'])
env.Append(CXXFLAGS = ['-pedantic'])

env.Append(TARFLAGS = ['-c'])
env.Append(TARFLAGS = ['--bzip2'])

if env['debug'] == 'yes':
	env.Append(CXXFLAGS = ['-g'])
	env.Append(CXXFLAGS = ['-O0'])
else:
	env.Append(CXXFLAGS = ['-O2'])
	#-fomit-frame-pointer -fmove-all-movables -fstrict-aliasing ')

#env.Append(LIBS = ['additional-lib-here'])

# Generate help text for command line options
Help(opts.GenerateHelpText(env))

# Cache current options
opts.Save('options.cache', env)

# Documentation generation system
doxygenBuilder = Builder(action = 'doxygen $SOURCE')
env.Append(BUILDERS = { 'DoxygenDoc' : doxygenBuilder })



##########################
#  Some initializations  #
##########################

# Messaging protocols
messaging_protocols = [ ]

for proto in re.split('\W+', env['with_messaging_protocols']):
	proto = string.strip(proto)
	if len(proto) >= 1:
		messaging_protocols.append(proto)

# Platforms
platforms = [ ]

for platform in re.split('\W+', env['with_platforms']):
	platform = string.strip(platform)
	if len(platform) >= 1:
		platforms.append(platform)

# Show configuration summary
print ""
print "+=================+"
print "|  CONFIGURATION  |"
print "+=================+"
print ""
print "Installation prefix      : " + env['prefix']
print "Static library (.a)      : " + env['static']
print "Shared library (.so)     : " + env['shared']
print "Debugging mode           : " + env['debug']
print "Messaging support        : " + env['with_messaging']
if env['with_messaging'] == 'yes':
	print "  * protocols            : " + env['with_messaging_protocols']
print "File-system support      : " + env['with_filesystem']
print "Default handlers         : " + env['with_platforms']
print ""


########################
#  Some sanity checks  #
########################

def IsProtocolSupported(protoList, proto):
	for supportedProto in protoList:
		if string.upper(supportedProto) == string.upper(proto):
			return 1
	return 0

# File-system support must be activated when 'maildir' protocol is selected
if env['with_messaging'] == 'yes':
	if IsProtocolSupported(messaging_protocols, 'maildir'):
		if env['with_filesystem'] != 'yes':
			print "ERROR: 'maildir' protocol requires file-system support!\n"
			Exit(1)


#########################
#  Generate config.hpp  #
#########################

config_hpp = open('src/config.hpp', 'w')

config_hpp.write("""
//
// This file was automatically generated by configuration script.
//

#ifndef VMIME_CONFIG_HPP_INCLUDED
#define VMIME_CONFIG_HPP_INCLUDED


""")

config_hpp.write('// Name of package\n')
config_hpp.write('#define VMIME_PACKAGE "' + packageName + '"\n')
config_hpp.write('\n')
config_hpp.write('// Version number of package\n')
config_hpp.write('#define VMIME_VERSION "' + packageVersion + '"\n')
config_hpp.write('\n')
config_hpp.write('// Target OS and architecture\n')

if os.name == 'posix':
	config_hpp.write('#define VMIME_TARGET_ARCH "' + commands.getoutput('uname -m') + '"\n')
	config_hpp.write('#define VMIME_TARGET_OS "' + commands.getoutput('uname -s') + '"\n')
else:
	config_hpp.write('#define VMIME_TARGET_ARCH ""  // Unknown\n')
	config_hpp.write('#define VMIME_TARGET_OS "' + sys.platform + '/' + os.name + '"\n')

config_hpp.write('\n')

if os.name == 'posix':
	config_hpp.write('#define VMIME_TARGET_OS_IS_POSIX  1  // POSIX compatible\n')
	config_hpp.write('#define VMIME_TARGET_OS_IS_WIN32  0  // Win32\n')
elif os.name == 'win32' or os.name == 'nt':
	config_hpp.write('#define VMIME_TARGET_OS_IS_POSIX  0  // POSIX compatible\n')
	config_hpp.write('#define VMIME_TARGET_OS_IS_WIN32  1  // Win32\n')
else:
	print "ERROR: unsupported system: '" + os.name + "'\n"
	Exit(1)

config_hpp.write('\n')
config_hpp.write('// Set to 1 if debugging should be activated\n')

if env['debug'] == 'yes':
	config_hpp.write('#define VMIME_DEBUG 1\n')
else:
	config_hpp.write('#define VMIME_DEBUG 0\n')

config_hpp.write('\n')
config_hpp.write('// Byte order (set one or the other, but not both!)\n')

if env['byte_order'] == 'big':
	config_hpp.write('#define IMEL_BYTE_ORDER_BIG_ENDIAN    1\n')
	config_hpp.write('#define IMEL_BYTE_ORDER_LITTLE_ENDIAN 0\n')
else:
	config_hpp.write('#define IMEL_BYTE_ORDER_BIG_ENDIAN    0\n')
	config_hpp.write('#define IMEL_BYTE_ORDER_LITTLE_ENDIAN 1\n')

config_hpp.write('\n')
config_hpp.write('// Generic types\n')
config_hpp.write('// -- 8-bit\n')
config_hpp.write('typedef signed ' + env['pf_8bit_type'] + ' vmime_int8;\n')
config_hpp.write('typedef unsigned ' + env['pf_8bit_type'] + ' vmime_uint8;\n')
config_hpp.write('// -- 16-bit\n')
config_hpp.write('typedef signed ' + env['pf_16bit_type'] + ' vmime_int16;\n')
config_hpp.write('typedef unsigned ' + env['pf_16bit_type'] + ' vmime_uint16;\n')
config_hpp.write('// -- 32-bit\n')
config_hpp.write('typedef signed ' + env['pf_32bit_type'] + ' vmime_int32;\n')
config_hpp.write('typedef unsigned ' + env['pf_32bit_type'] + ' vmime_uint32;\n')
config_hpp.write('\n')

config_hpp.write('// Options\n')

config_hpp.write('// -- Wide characters support\n')
if env['with_wide_char_support'] == 'yes':
	config_hpp.write('#define VMIME_WIDE_CHAR_SUPPORT 1\n')
else:
	config_hpp.write('#define VMIME_WIDE_CHAR_SUPPORT 0\n')

config_hpp.write('// -- File-system support\n');
if env['with_filesystem'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 0\n')

config_hpp.write('// -- Messaging support\n');
if env['with_messaging'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 1\n')

	config_hpp.write('// -- Built-in messaging protocols\n')
	config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTOS ' + env['with_messaging_protocols'] + '\n')

	for proto in messaging_protocols:
		config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(proto) + ' 1\n');
else:
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 0\n')

config_hpp.write("""

#endif // VMIME_CONFIG_HPP_INCLUDED
""")

config_hpp.close()


#################
#  Build rules  #
#################

# Build directory
if env['debug'] == 'yes':
	BuildDir("#build/debug", 'src', duplicate = 0)
	buildDirectory = 'build/debug/'
else:
	BuildDir("#build/release", 'src', duplicate = 0)
	buildDirectory = 'build/release/'

# Create source files list
libvmime_full_sources = libvmime_sources

if env['with_messaging'] == 'yes':
	# -- Add common files for messaging support
	for file in libvmime_messaging_sources:
		libvmime_full_sources.append(file)

	# -- Add protocol specific source files
	for proto in messaging_protocols:
		for protosrc in libvmime_messaging_proto_sources:
			if protosrc[0] == proto:
				for file in protosrc[1]:
					libvmime_full_sources.append(file)

# Split source files list into two lists: .CPP and .HPP
libvmime_sources_CPP = [ ]
libvmime_sources_HPP = [ ]

libvmime_install_includes = [ ]

for file in libvmime_full_sources:
	slash = string.rfind(file, '/')
	dir = ''

	if slash != -1:
		dir = file[0:slash] + '/'

	if file[-4:] == '.cpp':
		libvmime_sources_CPP.append(buildDirectory + file)
	else:
		libvmime_sources_HPP.append(buildDirectory + file)
		libvmime_install_includes.append([dir, buildDirectory + file])


# Main program build
if env['debug'] == 'yes':
	if env['static'] == 'yes':
		libVmime = env.StaticLibrary(
			target = 'vmime-debug',
			source = libvmime_sources_CPP
		)

	if env['shared'] == 'yes':
		libVmimeSh = env.SharedLibrary(
			target = 'vmime-debug',
			source = libvmime_sources_CPP
		)
else:
	if env['static'] == 'yes':
		libVmime = env.StaticLibrary(
			target = 'vmime',
			source = libvmime_sources_CPP
		)

	if env['shared'] == 'yes':
		libVmimeSh = env.SharedLibrary(
			target = 'vmime',
			source = libvmime_sources_CPP
		)

if env['static'] == 'yes': Default(libVmime)
if env['shared'] == 'yes': Default(libVmimeSh)


# Platform header files
for platform in libvmime_platforms_sources:
	files = libvmime_platforms_sources[platform]

	for file in files:
		slash = string.rfind(file, '/')
		dir = ''

		if slash != -1:
			dir = file[0:slash] + '/'

		if file[-4:] == '.hpp':
			libvmime_install_includes.append([dir, buildDirectory + file])

# Platform libraries
platformLibraries = [ ]

for platform in platforms:
	files = libvmime_platforms_sources[platform]

	sources_CPP = [ ]

	for file in files:
		if file[-4:] == '.cpp':
			sources_CPP.append(buildDirectory + file)

	platformLib = env.StaticLibrary(
		target = 'vmime-' + platform,
		source = sources_CPP
	)

	Default(platformLib)

	platformLibraries.append(platformLib)


# Tests
if env['build_tests'] == 'yes':
	libUnitpp = env.StaticLibrary(
		target = 'tests/unit++',
		source = libunitpp_sources
	)

	Default(libUnitpp)

	for test in libvmimetest_sources:
		Default(
			env.Program(
				target = test[0],
				source = test[1],
				LIBS=['unit++', 'vmime-posix', 'vmime-debug'],
				LIBPATH=['.', './tests/']
			)
		)


########################
#  Installation rules  #
########################

libDir = "%s/lib" % env['prefix']
includeDir = "%s/include/vmime" % env['prefix']

installPaths = [libDir, includeDir]

# Library
if env['static'] == 'yes': env.Install(libDir, libVmime)
if env['shared'] == 'yes': env.Install(libDir, libVmimeSh)

# Platform libraries
for platformLib in platformLibraries:
	env.Install(libDir, platformLib)

# Header files
for i in range(len(libvmime_install_includes)):
	env.Install(includeDir + '/' + libvmime_install_includes[i][0], libvmime_install_includes[i][1])

# Configuration header file
env.Install(includeDir, 'src/config.hpp')

# Provide "install" target (ie. 'scons install')
env.Alias('install', installPaths)


#####################
#  Packaging rules  #
#####################

# 'tar' is not available under Windows...
if not (os.name == 'win32' or os.name == 'nt'):
	packageFile = 'libvmime-' + packageVersion + '.tar.bz2'
	packageFileWithTests = 'libvmime-' + packageVersion + '-with-tests.tar.bz2'

	#env.Tar(packageFile, libvmime_dist_files)
	#env.Tar(packageFileWithTests, libvmime_dist_files_with_tests)
	env.Tar(packageFile, libvmime_dist_files_with_tests)

	#env.Alias('dist', [ packageFile, packageFileWithTests ])
	env.Alias('dist', packageFile)


###################
#  Documentation  #
###################

doxygenDocPath = '(doxygen-generated-files)'

env.DoxygenDoc(doxygenDocPath, 'vmime.doxygen')
env.Alias('doc', doxygenDocPath)

