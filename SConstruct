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


#############
#  Version  #
#############

# How to increment version number when building a public release?
# ===============================================================
#
# Changing package version number:
#
# * Increment major number if major changes have been made to the library,
#   that is, program depending on this library will need to be changed to
#   work with the new major release.
#
# * Increment minor number when the changes are upward-compatible: some
#   interfaces have been added, but compatibility is maintained for
#   existing interfaces.
#
# * Increment micro number when the changes do not add any new interface.
#   The changes only apply to the implementation (bug or security fix,
#   performance improvement, etc.).
#

# Package version number
packageVersionMajor = 0
packageVersionMinor = 6
packageVersionMicro = 4

# Shared library version number (computed from package version number)
packageAPICurrent   = packageVersionMajor + packageVersionMinor
packageAPIRevision  = packageVersionMicro
packageAPIAge       = packageVersionMinor

# Package information
packageName = 'libvmime'
packageGenericName = 'vmime'
packageRealName = 'VMime Library'
packageDescription = 'VMime C++ Mail Library (http://vmime.sourceforge.net)'
packageMaintainer = 'vincent@vincent-richard.net'

packageVersion = '%d.%d.%d' % (packageVersionMajor, packageVersionMinor, packageVersionMicro)
packageAPI = '%d:%d:%d' % (packageAPICurrent, packageAPIRevision, packageAPIAge)

#if packageVersionMajor >= 2:
#	packageVersionedGenericName = packageGenericName + ('%d' % packageVersionMajor)
#	packageVersionedName = packageName + ('%d' % packageVersionMajor)
#else:
#	packageVersionedGenericName = packageGenericName
#	packageVersionedName = packageName
packageVersionedGenericName = packageGenericName
packageVersionedName = packageName


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
	'contentDisposition.cpp', 'contentDisposition.hpp',
	'contentDispositionField.cpp', 'contentDispositionField.hpp',
	'contentHandler.cpp', 'contentHandler.hpp',
	'contentTypeField.cpp', 'contentTypeField.hpp',
	'dateTime.cpp', 'dateTime.hpp',
	'defaultAttachment.cpp', 'defaultAttachment.hpp',
	'disposition.cpp', 'disposition.hpp',
	'emptyContentHandler.cpp', 'emptyContentHandler.hpp',
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
	'path.cpp', 'path.hpp',
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
	'stringContentHandler.cpp', 'stringContentHandler.hpp',
	'streamContentHandler.cpp', 'streamContentHandler.hpp',
	'text.cpp', 'text.hpp',
	'textPartFactory.cpp', 'textPartFactory.hpp',
	'textPart.hpp',
	'typeAdapter.cpp', 'typeAdapter.hpp',
	'types.hpp',
	'word.cpp', 'word.hpp',
	'vmime.hpp',
	'utility/file.hpp',
	'utility/datetimeUtils.cpp', 'utility/datetimeUtils.hpp',
	'utility/md5.cpp', 'utility/md5.hpp',
	'utility/path.cpp', 'utility/path.hpp',
	'utility/random.cpp', 'utility/random.hpp',
	'utility/smartPtr.hpp',
	'utility/stream.cpp', 'utility/stream.hpp',
	'utility/stringProxy.cpp', 'utility/stringProxy.hpp',
	'utility/stringUtils.cpp', 'utility/stringUtils.hpp',
	'mdn/MDNHelper.cpp', 'mdn/MDNHelper.hpp',
	'mdn/MDNInfos.cpp', 'mdn/MDNInfos.hpp',
	'mdn/receivedMDNInfos.cpp', 'mdn/receivedMDNInfos.hpp',
	'mdn/sendableMDNInfos.cpp', 'mdn/sendableMDNInfos.hpp'
]

libvmime_examples_sources = [
	'examples/README',
	'examples/Makefile.am',    # not generated
	'examples/Makefile.in',
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
		'platforms/posix/posixFile.cpp', 'platforms/posix/posixFile.hpp',
		'platforms/posix/posixHandler.cpp', 'platforms/posix/posixHandler.hpp',
		'platforms/posix/posixSocket.cpp', 'platforms/posix/posixSocket.hpp'
	]
}

libvmime_extra = [
	'AUTHORS',
	'ChangeLog',
	'COPYING',
	'HACKING',
	'INSTALL',
	'NEWS',
	'README',
	'SConstruct',
	'vmime.doxygen'
]

libvmime_tests = [
#	'tests/Makefile.am',    # not generated
#	'tests/Makefile.in',

	# parser
#	'tests/parser/Makefile.am',    # not generated
#	'tests/parser/Makefile.in',

	# charset
#	'tests/charset/Makefile.am',   # not generated
#	'tests/charset/Makefile.in',
	'tests/charset/main.cpp',
	'tests/charset/run-test.sh',
	'tests/charset/test-suites/gnu.in.utf-8',
	'tests/charset/test-suites/gnu.out.iso-8859-1',
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
#	'tests/lib/unit++/Makefile.in',
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
	[ 'tests/parser/bodyPartTest', [ 'tests/parser/bodyPartTest.cpp' ] ],
	[ 'tests/parser/dispositionTest', [ 'tests/parser/dispositionTest.cpp' ] ],
	[ 'tests/parser/encoderTest', [ 'tests/parser/encoderTest.cpp' ] ],
	[ 'tests/parser/headerTest', [ 'tests/parser/headerTest.cpp' ] ],
	[ 'tests/parser/mailboxTest', [ 'tests/parser/mailboxTest.cpp' ] ],
	[ 'tests/parser/mediaTypeTest', [ 'tests/parser/mediaTypeTest.cpp' ] ],
	[ 'tests/parser/pathTest', [ 'tests/parser/pathTest.cpp' ] ],
	[ 'tests/parser/textTest', [ 'tests/parser/textTest.cpp' ] ],
	[ 'tests/utility/md5Test', [ 'tests/utility/md5Test.cpp' ] ],
	[ 'tests/utility/stringProxyTest', [ 'tests/utility/stringProxyTest.cpp' ] ],
	[ 'tests/utility/stringUtilsTest', [ 'tests/utility/stringUtilsTest.cpp' ] ],
	[ 'tests/utility/pathTest', [ 'tests/utility/pathTest.cpp' ] ]
]

libvmime_autotools = [
	'autotools/install-sh',
#	'autotools/mkinstalldirs',
	'autotools/missing',
	'autotools/config.guess',
	'autotools/config.sub',
	'autotools/ltmain.sh',
	'autotools/depcomp',
	'bootstrap',                   # not generated
	'configure',
	'configure.in',
	'config.h.in',
	'Makefile.am',
	'Makefile.in',
	'src/Makefile.am',
	'src/Makefile.in',
	'vmime/Makefile.am',
	'vmime/Makefile.in'
]

libvmime_all_sources = [] + libvmime_sources + libvmime_messaging_sources

for i in range(len(libvmime_all_sources)):
	f = libvmime_all_sources[i]
	if f[-4:] == '.hpp':
		libvmime_all_sources[i] = 'vmime/' + f
	else:
		libvmime_all_sources[i] = 'src/' + f

for p in libvmime_messaging_proto_sources:
	for f in p[1]:
		if f[-4:] == '.hpp':
			libvmime_all_sources.append('vmime/' + f)
		else:
			libvmime_all_sources.append('src/' + f)

for p in libvmime_platforms_sources:
	for f in libvmime_platforms_sources[p]:
		if f[-4:] == '.hpp':
			libvmime_all_sources.append('vmime/' + f)
		else:
			libvmime_all_sources.append('src/' + f)

libvmime_dist_files = libvmime_all_sources + libvmime_extra + libvmime_examples_sources

libvmime_dist_files += libvmime_tests
libvmime_dist_files += libunitpp_common
libvmime_dist_files += libunitpp_sources
libvmime_dist_files += libvmimetest_common
libvmime_dist_files += libvmime_autotools

for t in libvmimetest_sources:
	for f in t[1]:
		libvmime_dist_files.append(f)


#################
#  Set options  #
#################

EnsureSConsVersion(0, 94)

SetOption('implicit_cache', 1)

#SourceSignatures('timestamp')
SourceSignatures('MD5')
TargetSignatures('build')


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
		    + 'Currently available protocols: pop3, smtp, imap, maildir.',
		'"pop3 smtp imap maildir"'
	),
	(
		'with_platforms',
		'Specifies which default platform handlers to build.\n'
		    + 'This provides support for each platform selected.\n'
		    + 'If no platform handler is available for your platform, you will have\n'
		    + 'to write your own...\n'
		    + 'Separate platforms with spaces; string must be quoted with ".\n'
		    + 'Currently available platform handlers: posix.',
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
		'Byte order (Big Endian or Little Endian)',
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

env.Append(ENV = os.environ)
env.Append(ENV = {'PATH' : os.environ['PATH']})

env.Append(CPPPATH = [ '.' ])

env.Append(CPPDEFINES = { '_REENTRANT' : 1 })

env.Append(CXXFLAGS = ['-pipe'])
env.Append(CXXFLAGS = ['-W'])
env.Append(CXXFLAGS = ['-Wall'])
env.Append(CXXFLAGS = ['-ansi'])
env.Append(CXXFLAGS = ['-pedantic'])
env.Append(CXXFLAGS = ['-Wpointer-arith'])
env.Append(CXXFLAGS = ['-Wold-style-cast'])
env.Append(CXXFLAGS = ['-Wconversion'])

env.Append(TARFLAGS = ['-c'])
env.Append(TARFLAGS = ['--bzip2'])

if env['debug'] == 'yes':
	env.Append(CXXFLAGS = ['-g'])
	env.Append(CXXFLAGS = ['-O0'])
else:
	env.Append(CXXFLAGS = ['-O2'])
	#-fomit-frame-pointer -fmove-all-movables -fstrict-aliasing

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

for proto in re.split('\W+', string.replace(env['with_messaging_protocols'], '"', '')):
	proto = string.strip(proto)
	if len(proto) >= 1:
		messaging_protocols.append(proto)

# Platforms
platforms = [ ]

for platform in re.split('\W+', string.replace(env['with_platforms'], '"', '')):
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
print "Debugging mode           : " + env['debug']
print "Messaging support        : " + env['with_messaging']
if env['with_messaging'] == 'yes':
	print "  * protocols            : " + env['with_messaging_protocols']
print "File-system support      : " + env['with_filesystem']
print "Platform handlers        : " + env['with_platforms']
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

config_hpp = open('vmime/config.hpp', 'w')

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
config_hpp.write('#define VMIME_API "' + packageAPI + '"\n')
config_hpp.write('\n')
config_hpp.write('// Target OS and architecture\n')

if os.name == 'posix':
	config_hpp.write('#define VMIME_TARGET_ARCH "' + commands.getoutput('uname -m') + '"\n')
	config_hpp.write('#define VMIME_TARGET_OS "' + commands.getoutput('uname -s') + '"\n')
else:
	config_hpp.write('#define VMIME_TARGET_ARCH ""  // Unknown\n')
	config_hpp.write('#define VMIME_TARGET_OS "' + sys.platform + '/' + os.name + '"\n')

config_hpp.write('\n')
config_hpp.write('// Set to 1 if debugging should be activated\n')

if env['debug'] == 'yes':
	config_hpp.write('#define VMIME_DEBUG 1\n')
else:
	config_hpp.write('#define VMIME_DEBUG 0\n')

config_hpp.write('\n')
config_hpp.write('// Byte order (set one or the other, but not both!)\n')

if env['byte_order'] == 'big':
	config_hpp.write('#define VMIME_BYTE_ORDER_BIG_ENDIAN    1\n')
	config_hpp.write('#define VMIME_BYTE_ORDER_LITTLE_ENDIAN 0\n')
else:
	config_hpp.write('#define VMIME_BYTE_ORDER_BIG_ENDIAN    0\n')
	config_hpp.write('#define VMIME_BYTE_ORDER_LITTLE_ENDIAN 1\n')

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

config_hpp.write('// -- File-system support\n')
if env['with_filesystem'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 0\n')

config_hpp.write('// -- Messaging support\n')
if env['with_messaging'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 1\n')

	config_hpp.write('// -- Built-in messaging protocols\n')
	config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTOS "' +
		string.replace(env['with_messaging_protocols'], '"', '') + '"\n')

	for proto in messaging_protocols:
		config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(proto) + ' 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 0\n')

config_hpp.write('// -- Built-in platform handlers\n')
config_hpp.write('#define VMIME_BUILTIN_PLATFORMS "' +
	string.replace(env['with_platforms'], '"', '') + '"\n')

for platform in platforms:
	config_hpp.write('#define VMIME_BUILTIN_PLATFORM_' + string.upper(platform) + ' 1\n')

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

# Create effective source files list

# -- main library
libvmime_sel_sources = [] + libvmime_sources

# -- messaging module
if env['with_messaging'] == 'yes':
	# -- Add common files for messaging support
	for file in libvmime_messaging_sources:
		libvmime_sel_sources.append(file)

	# -- Add protocol specific source files
	for proto in messaging_protocols:
		for protosrc in libvmime_messaging_proto_sources:
			if protosrc[0] == proto:
				for file in protosrc[1]:
					libvmime_sel_sources.append(file)

# -- platform handlers
for platform in platforms:
	files = libvmime_platforms_sources[platform]

	for file in files:
		libvmime_sel_sources.append(file)

# Split source files list into two lists: .CPP and .HPP
libvmime_sources_CPP = [ ]
libvmime_sources_HPP = [ ]

libvmime_install_includes = [ ]

for file in libvmime_sel_sources:
	slash = string.rfind(file, '/')
	dir = ''

	if slash != -1:
		dir = file[0:slash] + '/'

	if file[-4:] == '.hpp':
		libvmime_sources_HPP.append(buildDirectory + file)
		libvmime_install_includes.append([dir, 'vmime/' + file, file])
	else:
		if file[-4:] == '.cpp':
			libvmime_sources_CPP.append(buildDirectory + file)

# HACK: SCons does not allow '.' in target name, so we have to
# detect the suffix for library name and add it ourself
#libFoo = env.StaticLibrary(target = 'FOO', source = [])
#libNameSuffix = '';
#
#if str(libFoo[0]).rfind('.') != -1:
#	libNameSuffix = str(libFoo[0])[str(libFoo[0]).rfind('.'):]

# Static library build
if env['debug'] == 'yes':
	libVmime = env.StaticLibrary(
		target = packageVersionedGenericName + '-debug',
		source = libvmime_sources_CPP
	)
else:
	libVmime = env.StaticLibrary(
		target = packageVersionedGenericName,
		source = libvmime_sources_CPP
	)

Default(libVmime)

# Tests
if env['build_tests'] == 'yes':
	if env['debug'] == 'yes':
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
					LIBS=['unit++', packageVersionedGenericName + '-debug'],
					LIBPATH=['.', './tests/']
				)
			)
	else:
		print 'Debug mode must be enabled to build tests!'
		Exit(1)


########################
#  Installation rules  #
########################

libDir = "%s/lib" % env['prefix']
#includeDir = "%s/include/%s/vmime" % (env['prefix'], packageVersionedGenericName)
includeDir = "%s/include/vmime" % env['prefix']

installPaths = [libDir, includeDir]

# Library
env.Install(libDir, libVmime)

# Header files
for i in range(len(libvmime_install_includes)):
	env.Install(includeDir + '/' + libvmime_install_includes[i][0], libvmime_install_includes[i][1])

# Configuration header file
env.Install(includeDir, 'vmime/config.hpp')

# Pkg-config support
vmime_pc = open(packageVersionedGenericName + ".pc", 'w')

vmime_pc.write("prefix=" + env['prefix'] + "\n")
vmime_pc.write("exec_prefix=" + env['prefix'] + "\n")
vmime_pc.write("libdir=" + env['prefix'] + "/lib\n")
vmime_pc.write("includedir=" + env['prefix'] + "/include\n")
vmime_pc.write("\n")
vmime_pc.write("Name: " + packageRealName + "\n")
vmime_pc.write("Description: " + packageDescription + "\n")
vmime_pc.write("Version: " + packageVersion + "\n")
vmime_pc.write("Requires:\n")
vmime_pc.write("Libs: -L${libdir} -l" + packageVersionedGenericName + "\n")
#vmime_pc.write("Cflags: -I${includedir}/" + packageVersionedGenericName + "\n")
vmime_pc.write("Cflags: -I${includedir}/" + "\n")

vmime_pc.close()

env.Install(libDir + "/pkgconfig", packageVersionedGenericName + ".pc")

# Provide "install" target (ie. 'scons install')
env.Alias('install', installPaths)


################################
#  Generate autotools scripts  #
################################

# Return the path of the specified filename.
# Example: getPath("a/b/c/myfile") will return "a/b/c"
def getPath(s):
	x = s.rfind('/')
	if x != -1:
		return s[0:x]
	else:
		return ""

def getParentPath(s):
	return getPath(s)

def getLastDir(s):
	x = s.rfind('/')
	if x != -1:
		return s[x + 1:len(s)]
	else:
		return s

def buildMakefileFileList(l, replaceSlash):
	s = ''

	for i in range(len(l)):
		if i != 0:
			s += ' \\\n\t'
		f = l[i]
		if replaceSlash:
			f = string.replace(f, "/", "_")
		s += f

	return s

def selectFilesFromSuffixNot(l, ext):
	r = []
	n = len(ext)

	for f in l:
		if f[-n:] != ext:
			r.append(f)

	return r

def generateAutotools(target, source, env):
	# Generate pkg-config file for shared and static library
	vmime_pc_in = open(packageVersionedGenericName + ".pc.in", 'w')
	vmime_pc_in.write("# File automatically generated by SConstruct ('scons autotools')\n")
	vmime_pc_in.write("# DOT NOT EDIT!\n")
	vmime_pc_in.write("\n")
	vmime_pc_in.write("prefix=@prefix@\n")
	vmime_pc_in.write("exec_prefix=@exec_prefix@\n")
	vmime_pc_in.write("libdir=@libdir@\n")
	vmime_pc_in.write("includedir=@includedir@\n")
	vmime_pc_in.write("\n")
	vmime_pc_in.write("Name: @GENERIC_LIBRARY_NAME@\n")
	vmime_pc_in.write("Description: " + packageDescription + "\n")
	vmime_pc_in.write("Version: @VERSION@\n")
	vmime_pc_in.write("Requires:\n")
	vmime_pc_in.write("Libs: -L${libdir} -l@GENERIC_VERSIONED_LIBRARY_NAME@\n")
	#vmime_pc_in.write("Cflags: -I${includedir}/@GENERIC_VERSIONED_LIBRARY_NAME@\n")
	vmime_pc_in.write("Cflags: -I${includedir}/\n")
	vmime_pc_in.close()

	# Generate 'Makefile.am'
	Makefile_am = open("Makefile.am", 'w')
	Makefile_am.write("""
# File automatically generated by SConstruct ('scons autotools')
# DOT NOT EDIT!

BINDING =
INCLUDE = vmime
#examples tests

SUBDIRS = src $(INCLUDE) $(BINDING)

DIST_SUBDIRS = $(SUBDIRS) autotools

#AUTOMAKE_OPTIONS = dist-bzip2
AUTOMAKE_OPTIONS = no-dist

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(GENERIC_VERSIONED_LIBRARY_NAME).pc

EXTRA_DIST=SConstruct bootstrap

dist:
	@ echo ""
	@ echo "Please use 'scons dist' to generate distribution tarball."
	@ echo ""

doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README
docdir = $(datadir)/doc/$(GENERIC_LIBRARY_NAME)
""")
	Makefile_am.close()

	# Generate Makefile for header files
	Makefile_am = open("vmime/Makefile.am", 'w')
	Makefile_am.write("""
# File automatically generated by SConstruct ('scons autotools')
# DOT NOT EDIT!
""")

	#Makefile_am.write(packageVersionedName + "includedir = $(prefix)/include/@GENERIC_VERSIONED_LIBRARY_NAME@/@GENERIC_LIBRARY_NAME@\n")
	Makefile_am.write(packageVersionedName + "includedir = $(prefix)/include/@GENERIC_LIBRARY_NAME@\n")
	Makefile_am.write("nobase_" + packageVersionedName + "include_HEADERS = ")

	x = []

	for file in libvmime_all_sources:
		if file[-4:] == '.hpp':
			x.append(file[len("vmime/"):])   # remove 'vmime/' prefix

	x.append("config.hpp")

	Makefile_am.write(buildMakefileFileList(x, 0))
	Makefile_am.close()

	# Generate 'src/Makefile.am' (Makefile for source files)
	Makefile_am = open("src/Makefile.am", 'w')
	Makefile_am.write("""
# File automatically generated by SConstruct ('scons autotools')
# DOT NOT EDIT!

AUTOMAKE_OPTIONS = no-dependencies foreign
INTERNALS =
INCLUDES = -I$(top_srcdir) -I$(srcdir) @PKGCONFIG_CFLAGS@ @EXTRA_CFLAGS@
""")

	Makefile_am.write('lib_LTLIBRARIES = ' + packageVersionedName + '.la\n')
	Makefile_am.write(packageVersionedName + '_la_LDFLAGS = -export-dynamic -version-info '
#		+ '@LIBRARY_VERSION@ -release @LIBRARY_RELEASE@ @PKGCONFIG_LIBS@ @EXTRA_LIBS@\n')
		+ '@LIBRARY_VERSION@ @PKGCONFIG_LIBS@ @EXTRA_LIBS@\n')

	sourceFiles = []  # for conversion:   subpath/file.cpp --> subpath_file.cpp
	                  # used to avoid collision when two files have the same name if different dirs

	# -- base library
	x = selectFilesFromSuffixNot(libvmime_sources, '.hpp')
	sourceFiles += x

	Makefile_am.write(packageVersionedName + "_la_SOURCES = " + buildMakefileFileList(x, 1) + "\n")

	# -- messaging module
	x = selectFilesFromSuffixNot(libvmime_messaging_sources, '.hpp')
	sourceFiles += x

	Makefile_am.write("\n")
	Makefile_am.write("if VMIME_HAVE_MESSAGING_FEATURES\n")
	Makefile_am.write(packageVersionedName + "_la_SOURCES += " + buildMakefileFileList(x, 1) + "\n")
	Makefile_am.write("endif\n")

	# -- messaging protocols
	for proto in libvmime_messaging_proto_sources:
		Makefile_am.write("\n")
		Makefile_am.write("if VMIME_BUILTIN_MESSAGING_PROTO_" + string.upper(proto[0]) + "\n")

		x = selectFilesFromSuffixNot(proto[1], '.hpp')
		sourceFiles += x

		Makefile_am.write(packageVersionedName + "_la_SOURCES += " + buildMakefileFileList(x, 1) + "\n")
		Makefile_am.write("endif\n")

	# -- platform handlers
	for platform in libvmime_platforms_sources:
		Makefile_am.write("\n")
		Makefile_am.write("if VMIME_BUILTIN_PLATFORM_" + string.upper(platform) + "\n")

		x = selectFilesFromSuffixNot(libvmime_platforms_sources[platform], '.hpp')
		sourceFiles += x

		Makefile_am.write(packageVersionedName + "_la_SOURCES += " + buildMakefileFileList(x, 1) + "\n")
		Makefile_am.write("endif\n")

	Makefile_am.write("""

noinst_HEADERS = $(INTERNALS)

""")

	for f in sourceFiles:    # symbolic links to avoid filename collision
		a = f
		b = string.replace(f, "/", "_")
		if a != b:
			Makefile_am.write(b + ": " + a + "\n\tln -sf $< $@\n\n")

	Makefile_am.close()

	# Generate 'configure.in'
	configure_in = open("configure.in", 'w')
	configure_in.write("""
# configure.in

# File automatically generated by SConstruct ('scons autotools')
# DOT NOT EDIT!

# Init
""")

	configure_in.write("AC_INIT([" + packageRealName
		+ "], [" + packageVersion + "], [" + packageMaintainer
		+ "], [" + packageGenericName + "])")
	configure_in.write("""

AC_PREREQ([2.53])

AC_CONFIG_AUX_DIR(autotools)

# Library name
GENERIC_LIBRARY_NAME=""" + '"' + packageGenericName + '"' + """
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSIONED_LIBRARY_NAME=""" + '"' + packageVersionedGenericName + '"' + """
AC_SUBST(GENERIC_VERSIONED_LIBRARY_NAME)

LIBRARY_NAME=""" + '"' + packageName + '"' + """
AC_SUBST(LIBRARY_NAME)

""")

	configure_in.write('# Library version\n')
	configure_in.write('LIBRARY_VERSION="' + str(packageAPICurrent)
		+ ':' + str(packageAPIRevision) + ':' + str(packageAPIAge) + '"\n')
	configure_in.write('AC_SUBST(LIBRARY_VERSION)\n')
	configure_in.write('\n')
	configure_in.write('LIBRARY_RELEASE="' + str(packageVersionMajor) + '"\n')
	configure_in.write('AC_SUBST(LIBRARY_RELEASE)\n')

	configure_in.write("""

#
# Miscellaneous init stuff
#

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(""" + packageGenericName + """, """ + packageVersion + """)
AC_CONFIG_SRCDIR([src/base.cpp])
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE


#
# Check compilers, processors, etc
#

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_C_CONST

AC_LANG(C++)

AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL

AM_SANITY_CHECK
AM_PROG_LIBTOOL
AM_PROG_CC_C_O


#
# Some checks
#

# -- global constructors (stolen from 'configure.in' in libsigc++)
AC_MSG_CHECKING([if linker supports global constructors])
cat > mylib.$ac_ext <<EOF
#include <stdio.h>

struct A
{
  A() { printf(\"PASS\\n\"); }
};

A a;

int foo()
  { return 1; }

EOF
cat > mytest.$ac_ext <<EOF
#include <stdio.h>

extern int foo();

int main(int, char**)
{
  int i = foo();
  if(i != 1) printf(\"FAIL\\n\");
  return 0;
}

EOF
sh libtool --mode=compile $CXX -c mylib.$ac_ext >&5
sh libtool --mode=link $CXX -o libtest.la -rpath / -version-info 0 mylib.lo  >&5
$CXX -c $CFLAGS $CPPFLAGS mytest.$ac_ext >&5
sh libtool --mode=link $CXX -o mytest mytest.o libtest.la >&5 2>/dev/null

if test -x mytest; then
	myresult=`./mytest`
	if test "X$myresult" = "XPASS"; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AC_ERROR([
===================================================================
ERROR: This platform lacks support of construction of global
objects in shared librarys.

See ftp://rtfm.mit.edu/pub/usenet/news.answers/g++-FAQ/plain
for details about this problem.  Also for possible solutions
http://www.informatik.uni-frankfurt.de/~fp/Tcl/tcl-c++/tcl-c++.html
===================================================================])
	fi
else
	AC_MSG_RESULT(unknown)
fi
rm -f mylib.* mytest.* libtest.la .libs/libtest* mytest .libs/mytest .libs/lt-mytest .libs/mylib.* >&5
rmdir .libs >&5

# -- const_cast
AC_MSG_CHECKING([if C++ compiler supports const_cast<> (required)])
AC_TRY_COMPILE(
[
   class foo;
],[
   const foo *c=0;
   foo *c1=const_cast<foo*>(c);
],[
  AC_MSG_RESULT(yes)
],[
  AC_MSG_RESULT(no)
  AC_ERROR(C++ compiler const_cast<> does not work)
])

# -- dynamic_cast
AC_MSG_CHECKING([if C++ compiler supports dynamic_cast<> (required)])
AC_TRY_COMPILE(
[
   class foo { virtual ~foo() { } };
   class bar : public foo { };
],[
   foo *c=0;
   bar *c1=dynamic_cast<bar*>(c);
],[
  AC_MSG_RESULT(yes)
],[
  AC_MSG_RESULT(no)
  AC_ERROR(C++ compiler dynamic_cast<> does not work)
])

# -- mutable
AC_MSG_CHECKING(if C++ compiler supports mutable (required))
AC_TRY_COMPILE(
[
class k {
        mutable char *c;
public:
   void foo() const { c=0; }
};
],[
],[
  AC_MSG_RESULT(yes)
],[
  AC_MSG_RESULT(no)
  AC_ERROR(C++ compiler does not support 'mutable')
])

# -- namespace
AC_MSG_CHECKING(if C++ compiler supports name spaces (required))
AC_TRY_COMPILE(
[
namespace Check
  {
   int i;
  }
],[
  Check::i=1;
],[
  AC_MSG_RESULT(yes)
],[
  AC_MSG_RESULT(no)
  AC_ERROR(C++ compiler does not support 'namespace')
])


#
# Target OS and architecture
#

VMIME_TARGET_ARCH=${target_cpu}
VMIME_TARGET_OS=${target_os}

#
# Byte Order
#

AC_C_BIGENDIAN

if test "x$ac_cv_c_bigendian" = "xyes"; then
  VMIME_BYTE_ORDER_BIG_ENDIAN=1
  VMIME_BYTE_ORDER_LITTLE_ENDIAN=0
else
  VMIME_BYTE_ORDER_BIG_ENDIAN=0
  VMIME_BYTE_ORDER_LITTLE_ENDIAN=1
fi


#
# Generic Type Size
#

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)

case 1 in
$ac_cv_sizeof_char)
  VMIME_TYPE_INT8=char
  ;;
*)
  AC_MSG_ERROR([no 8-bit type available])
esac

case 2 in
$ac_cv_sizeof_short)
  VMIME_TYPE_INT16=short
  ;;
$ac_cv_sizeof_int)
  VMIME_TYPE_INT16=int
  ;;
*)
  AC_MSG_ERROR([no 16-bit type available])
esac

case 4 in
$ac_cv_sizeof_int)
  VMIME_TYPE_INT32=int
  ;;
$ac_cv_sizeof_int)
  VMIME_TYPE_INT32=long
  ;;
*)
  AC_MSG_ERROR([no 32-bit type available])
esac


#
# Options
#

# ** debug

AC_ARG_ENABLE(debug,
     [  --enable-debug        Turn on debugging (default: disabled)],
     [case "${enableval}" in
       yes) conf_debug=yes ;;
       no)  conf_debug=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
      esac],
     [conf_debug=no])

if test "x$conf_debug" = "xyes"; then
	AM_CONDITIONAL(VMIME_DEBUG, true)
	VMIME_DEBUG=1
else
	AM_CONDITIONAL(VMIME_DEBUG, false)
	VMIME_DEBUG=0
fi

# ** messaging

AC_ARG_ENABLE(messaging,
     [  --enable-messaging    Enable messaging support\
 (connection to mail servers, default: enabled)],
     [case "${enableval}" in
       yes) conf_messaging=yes ;;
       no)  conf_messaging=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-messaging) ;;
      esac],
     [conf_messaging=yes])

if test "x$conf_messaging" = "xyes"; then
	AM_CONDITIONAL(VMIME_HAVE_MESSAGING_FEATURES, true)
	VMIME_HAVE_MESSAGING_FEATURES=1
else
	AM_CONDITIONAL(VMIME_HAVE_MESSAGING_FEATURES, false)
	VMIME_HAVE_MESSAGING_FEATURES=0
fi

# ** messaging protocols

VMIME_BUILTIN_MESSAGING_PROTOS=''

""")

	for proto in libvmime_messaging_proto_sources:
		p = proto[0]

		configure_in.write("AC_ARG_ENABLE(messaging-proto-" + p + ",\n")
		configure_in.write("     [  --enable-messaging-proto-" + p
			+ "   Enable built-in support for protocol '" + p + "' "
			+ " (default: enabled)],\n")
		configure_in.write('     [case "${enableval}" in\n')
		configure_in.write('       yes) conf_messaging_proto_' + p + '=yes ;;\n')
		configure_in.write('       no)  conf_messaging_proto_' + p + '=no ;;\n')
		configure_in.write('       *) AC_MSG_ERROR(bad value ${enableval} for '
			+ '--enable-messaging-proto-' + p + ') ;;\n')
		configure_in.write('      esac],\n')
		configure_in.write('     [conf_messaging_proto_' + p + '=yes])\n')

		configure_in.write('if test "x$conf_messaging_proto_' + p + '" = "xyes"; then\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + ', true)\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + '=1\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTOS="$VMIME_BUILTIN_MESSAGING_PROTOS ' + p + '"\n')
		configure_in.write('else\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + ', false)\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + '=0\n')
		configure_in.write('fi\n')

	configure_in.write("""
# ** platform handlers

VMIME_BUILTIN_PLATFORMS=''

""")

	for p in libvmime_platforms_sources:
		configure_in.write("AC_ARG_ENABLE(platform-" + p + ",\n")
		configure_in.write("     [  --enable-platform-" + p
			+ "   Compile built-in platform handler for '" + p + "' "
			+ " (default: enabled)],\n")
		configure_in.write('     [case "${enableval}" in\n')
		configure_in.write('       yes) conf_platform_' + p + '=yes ;;\n')
		configure_in.write('       no)  conf_platform_' + p + '=no ;;\n')
		configure_in.write('       *) AC_MSG_ERROR(bad value ${enableval} for '
			+ '--enable-platform-' + p + ') ;;\n')
		configure_in.write('      esac],\n')
		configure_in.write('     [conf_platform_' + p + '=yes])\n')

		configure_in.write('if test "x$conf_platform_' + p + '" = "xyes"; then\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_PLATFORM_' + string.upper(p) + ', true)\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORM_' + string.upper(p) + '=1\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORMS="$VMIME_BUILTIN_PLATFORMS ' + p + '"\n')
		configure_in.write('else\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_PLATFORM_' + string.upper(p) + ', false)\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORM_' + string.upper(p) + '=0\n')
		configure_in.write('fi\n')

	configure_in.write("""

#
# Flags
#

LIBRARY_LD_FLAGS="\$(top_builddir)/src/\$(LIBRARY_NAME).la"
AC_SUBST(LIBRARY_LD_FLAGS)

PKGCONFIG_CFLAGS=""
PKGCONFIG_LIBS=""

AC_SUBST(PKGCONFIG_CFLAGS)
AC_SUBST(PKGCONFIG_LIBS)

EXTRA_CFLAGS="$EXTRA_CFLAGS -D_REENTRANT=1"
EXTRA_LIBS=""

CFLAGS=""
CXXFLAGS=""

# -- Debug
if test x$VMIME_DEBUG = x1 ; then
	# -g
	OLD_CXXFLAGS="$CXXFLAGS"
	CXX_FLAGS="$CXXFLAGS -g"
	AC_MSG_CHECKING(whether cc accepts -g)
	AC_TRY_COMPILE(,,echo yes,echo no; CXXFLAGS="$OLD_CXXFLAGS")
else
	# -O2
	OLD_CXXFLAGS="$CXXFLAGS"
	CXX_FLAGS="$CXXFLAGS -O2"
	AC_MSG_CHECKING(whether cc accepts -O2)
	AC_TRY_COMPILE(,,echo yes,echo no; CXXFLAGS="$OLD_CXXFLAGS")
fi

# -- HACK: add -fPIC or -fpic on static library object files
EXTRA_CFLAGS="$EXTRA_CFLAGS $lt_prog_compiler_pic"



#
# Check to see if the compiler can handle some flags
#

""")

	compilerFlags = [ '-pipe', '-ansi', '-pedantic', '-W', '-Wall', '-Wpointer-arith', '-Wold-style-cast', '-Wconversion' ]

	for f in compilerFlags:
		configure_in.write('# ' + f + '\n')
		configure_in.write('OLD_EXTRA_CFLAGS="$EXTRA_CFLAGS"\n')
		configure_in.write('EXTRA_CFLAGS="$EXTRA_CFLAGS ' + f + '"\n')
		configure_in.write('AC_MSG_CHECKING(whether cc accepts ' + f + ')\n')
		configure_in.write('AC_TRY_COMPILE(,,echo yes,echo no; EXTRA_CFLAGS="$OLD_EXTRA_CFLAGS")\n\n')

	configure_in.write("""
#EXTRA_CFLAGS=`echo $EXTRA_CFLAGS | sed -e 's| |\\n|g' | sort | uniq | tr '\\n' ' '`
EXTRA_CFLAGS=`echo $EXTRA_CFLAGS | tr '\\n' ' ' | sort | uniq | tr '\\n' ' '`
EXTRA_LIBS=`echo $EXTRA_LIBS | sed -e 's|^ ||g' | sed -e 's|  | |g'`

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LIBS)

LIBS=`echo $LIBS | sed -e 's|^ ||g' | sed -e 's|  | |g'`

AC_CONFIG_FILES([ """)

	libvmime_dist_files.append(packageVersionedGenericName + ".pc.in")

	configure_in.write(packageVersionedGenericName + ".pc\n")

	for f in libvmime_dist_files:
		if f[-11:] == 'Makefile.in':
			configure_in.write("\t" + f[0:len(f) - 3] + "\n")

	configure_in.write("\t])")
	configure_in.write("""
AC_OUTPUT


#
# Generate vmime/config.hpp
#

echo "
//
// This file was automatically generated by configuration script.
//

#ifndef VMIME_CONFIG_HPP_INCLUDED
#define VMIME_CONFIG_HPP_INCLUDED


// Name of package
#define VMIME_PACKAGE """ + '\\"' + packageName + '\\"' + """

// Version number of package
#define VMIME_VERSION """ + '\\"' + packageVersion + '\\"' + """
#define VMIME_API """ + '\\"' + packageAPI + '\\"' + """

// Target OS and architecture
#define VMIME_TARGET_ARCH \\"${VMIME_TARGET_ARCH}\\"
#define VMIME_TARGET_OS \\"${VMIME_TARGET_OS}\\"

// Set to 1 if debugging should be activated
#define VMIME_DEBUG ${VMIME_DEBUG}

// Byte order (set one or the other, but not both!)
#define VMIME_BYTE_ORDER_BIG_ENDIAN    ${VMIME_BYTE_ORDER_BIG_ENDIAN}
#define VMIME_BYTE_ORDER_LITTLE_ENDIAN ${VMIME_BYTE_ORDER_LITTLE_ENDIAN}

// Generic types
// -- 8-bit
typedef signed ${VMIME_TYPE_INT8} vmime_int8;
typedef unsigned ${VMIME_TYPE_INT8} vmime_uint8;
// -- 16-bit
typedef signed ${VMIME_TYPE_INT16} vmime_int16;
typedef unsigned ${VMIME_TYPE_INT16} vmime_uint16;
// -- 32-bit
typedef signed ${VMIME_TYPE_INT32} vmime_int32;
typedef unsigned ${VMIME_TYPE_INT32} vmime_uint32;

// Options
// -- Wide characters support
#define VMIME_WIDE_CHAR_SUPPORT 0
// -- File-system support
#define VMIME_HAVE_FILESYSTEM_FEATURES 1
// -- Messaging support
#define VMIME_HAVE_MESSAGING_FEATURES ${VMIME_HAVE_MESSAGING_FEATURES}
""")

	# Messaging protocols
	configure_in.write('// -- Built-in messaging protocols\n')
	configure_in.write('#define VMIME_BUILTIN_MESSAGING_PROTOS \\"$VMIME_BUILTIN_MESSAGING_PROTOS\\"\n')

	for p in libvmime_messaging_proto_sources:
		p = string.upper(p[0])
		configure_in.write("#define VMIME_BUILTIN_MESSAGING_PROTO_" + p
			+ " $VMIME_BUILTIN_MESSAGING_PROTO_" + p + "\n")

	# Platform handlers
	configure_in.write('// -- Built-in platform handlers\n')
	configure_in.write('#define VMIME_BUILTIN_PLATFORMS \\"$VMIME_BUILTIN_PLATFORMS\\"\n')

	for p in platforms:
		p = string.upper(p)
		configure_in.write('#define VMIME_BUILTIN_PLATFORM_' + p
			+ " $VMIME_BUILTIN_PLATFORM_" + p + " \n")

	configure_in.write("""

#endif // VMIME_CONFIG_HPP_INCLUDED
" > vmime/config.hpp

AC_MSG_RESULT([
+=================+
|  CONFIGURATION  |
+=================+

Installation prefix      : $prefix
Debugging mode           : $conf_debug
Messaging support        : $conf_messaging
     * protocols         :$VMIME_BUILTIN_MESSAGING_PROTOS
File-system support      : yes
Platform handlers        :$VMIME_BUILTIN_PLATFORMS
])
""")

	configure_in.close()

	os.system('./bootstrap')

	return None

# Custom builder for generating autotools scripts
autotoolsBuilder = Builder(action = generateAutotools)
env.Append(BUILDERS = { 'GenerateAutoTools' : autotoolsBuilder })

env.Alias('autotools', env.GenerateAutoTools('foo_autotools', 'SConstruct'))


#####################
#  Packaging rules  #
#####################

# 'tar' is not available under Windows...
if not (os.name == 'win32' or os.name == 'nt'):
	def createPackage(target, source, env):
		packageFullName = packageName + '-' + packageVersion
		packageFile = packageFullName + '.tar.bz2'

		# Generate autotools-related files
		generateAutotools([], [], env)

		distFiles = []
		distFilesStr = ''

		for f in libvmime_dist_files:
			distFiles.append(packageFullName + '/' + f)
			distFilesStr = distFilesStr + packageFullName + '/' + f + ' '
			print f

		os.system('ln -s . ' + packageFullName)
		os.system('tar jcf ' + packageFile + ' ' + distFilesStr)
		os.system('rm -f ' + packageFullName)

		return None

	# Custom builder for creating package
	createPackageBuilder = Builder(action = createPackage)
	env.Append(BUILDERS = { 'CreatePackage' : createPackageBuilder })

	env.Alias('dist', env.CreatePackage('foo_tar', 'SConstruct'))


###################
#  Documentation  #
###################

doxygenDocPath = '(doxygen-generated-files)'

env.DoxygenDoc(doxygenDocPath, 'vmime.doxygen')
env.Alias('doc', doxygenDocPath)
