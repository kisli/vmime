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

# Package version number
packageVersionMajor = 0
packageVersionMinor = 9
packageVersionMicro = 2

# API version number (libtool)
#
# Increment this number only immediately before a public release.
# This is independent from package version number.
#
# See: http://www.gnu.org/software/libtool/manual.html#Libtool-versioning
#
# . Implementation changed (eg. bug/security fix):  REVISION++
# . Interfaces added/removed/changed:               CURRENT++, REVISION=0
# . Interfaces added (upward-compatible changes):   AGE++
# . Interfaces removed:                             AGE=0
#
packageAPICurrent   = 0
packageAPIRevision  = 0
packageAPIAge       = 0

# Package information
packageName = 'libvmime'
packageGenericName = 'vmime'
packageRealName = 'VMime Library'
packageDescription = 'VMime C++ Mail Library (http://www.vmime.org)'
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
	# ==============================  Parser  ==============================
	'address.cpp', 'address.hpp',
	'addressList.cpp', 'addressList.hpp',
	'attachment.hpp',
	'attachmentHelper.cpp', 'attachmentHelper.hpp',
	'base.cpp', 'base.hpp',
	'body.cpp', 'body.hpp',
	'bodyPart.cpp', 'bodyPart.hpp',
	'bodyPartAttachment.cpp', 'bodyPartAttachment.hpp',
	'charset.cpp', 'charset.hpp',
	'charsetConverter.cpp', 'charsetConverter.hpp',
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
	'encoding.cpp', 'encoding.hpp',
	'exception.cpp', 'exception.hpp',
	'fileAttachment.cpp', 'fileAttachment.hpp',
	'generatedMessageAttachment.hpp', 'generatedMessageAttachment.cpp',
	'header.cpp', 'header.hpp',
	'headerFieldFactory.cpp', 'headerFieldFactory.hpp',
	'headerField.cpp', 'headerField.hpp',
	'headerFieldValue.hpp',
	'htmlTextPart.cpp', 'htmlTextPart.hpp',
	'mailbox.cpp', 'mailbox.hpp',
	'mailboxField.cpp', 'mailboxField.hpp',
	'mailboxGroup.cpp', 'mailboxGroup.hpp',
	'mailboxList.cpp', 'mailboxList.hpp',
	'mediaType.cpp', 'mediaType.hpp',
	'messageAttachment.hpp',
	'messageBuilder.cpp', 'messageBuilder.hpp',
	'message.cpp', 'message.hpp',
	'messageId.cpp', 'messageId.hpp',
	'messageIdSequence.cpp', 'messageIdSequence.hpp',
	'messageParser.cpp', 'messageParser.hpp',
	'object.cpp', 'object.hpp',
	'options.cpp', 'options.hpp',
	'path.cpp', 'path.hpp',
	'parameter.cpp', 'parameter.hpp',
	'parameterizedHeaderField.cpp', 'parameterizedHeaderField.hpp',
	'parsedMessageAttachment.cpp', 'parsedMessageAttachment.hpp',
	'parserHelpers.hpp',
	'plainTextPart.cpp', 'plainTextPart.hpp',
	'platform.cpp', 'platform.hpp',
	'propertySet.cpp', 'propertySet.hpp',
	'relay.cpp', 'relay.hpp',
	'stringContentHandler.cpp', 'stringContentHandler.hpp',
	'streamContentHandler.cpp', 'streamContentHandler.hpp',
	'text.cpp', 'text.hpp',
	'textPartFactory.cpp', 'textPartFactory.hpp',
	'textPart.hpp',
	'types.hpp',
	'word.cpp', 'word.hpp',
	'wordEncoder.cpp', 'wordEncoder.hpp',
	'vmime.hpp',
	# ==============================  Utility  =============================
	'utility/childProcess.hpp',
	'utility/file.hpp',
	'utility/datetimeUtils.cpp', 'utility/datetimeUtils.hpp',
	'utility/path.cpp', 'utility/path.hpp',
	'utility/progressListener.cpp', 'utility/progressListener.hpp',
	'utility/random.cpp', 'utility/random.hpp',
	'utility/smartPtr.cpp', 'utility/smartPtr.hpp',
	'utility/smartPtrInt.cpp', 'utility/smartPtrInt.hpp',
	'utility/stream.cpp', 'utility/stream.hpp',
	'utility/streamUtils.cpp', 'utility/streamUtils.hpp',
	'utility/filteredStream.cpp', 'utility/filteredStream.hpp',
	'utility/inputStream.cpp', 'utility/inputStream.hpp',
	'utility/inputStreamAdapter.cpp', 'utility/inputStreamAdapter.hpp',
	'utility/inputStreamByteBufferAdapter.cpp', 'utility/inputStreamByteBufferAdapter.hpp',
	'utility/inputStreamPointerAdapter.cpp', 'utility/inputStreamPointerAdapter.hpp',
	'utility/inputStreamSocketAdapter.cpp', 'utility/inputStreamSocketAdapter.hpp',
	'utility/inputStreamStringAdapter.cpp', 'utility/inputStreamStringAdapter.hpp',
	'utility/inputStreamStringProxyAdapter.cpp', 'utility/inputStreamStringProxyAdapter.hpp',
	'utility/seekableInputStream.hpp',
	'utility/seekableInputStreamRegionAdapter.cpp', 'utility/seekableInputStreamRegionAdapter.hpp',
	'utility/outputStream.cpp', 'utility/outputStream.hpp',
	'utility/outputStreamAdapter.cpp', 'utility/outputStreamAdapter.hpp',
	'utility/outputStreamByteArrayAdapter.cpp', 'utility/outputStreamByteArrayAdapter.hpp',
	'utility/outputStreamSocketAdapter.cpp', 'utility/outputStreamSocketAdapter.hpp',
	'utility/outputStreamStringAdapter.cpp', 'utility/outputStreamStringAdapter.hpp',
	'utility/parserInputStreamAdapter.cpp', 'utility/parserInputStreamAdapter.hpp',
	'utility/stringProxy.cpp', 'utility/stringProxy.hpp',
	'utility/stringUtils.cpp', 'utility/stringUtils.hpp',
	'utility/url.cpp', 'utility/url.hpp',
	'utility/urlUtils.cpp', 'utility/urlUtils.hpp',
	# -- encoder
	'utility/encoder/encoder.cpp', 'utility/encoder/encoder.hpp',
	'utility/encoder/sevenBitEncoder.cpp', 'utility/encoder/sevenBitEncoder.hpp',
	'utility/encoder/eightBitEncoder.cpp', 'utility/encoder/eightBitEncoder.hpp',
	'utility/encoder/b64Encoder.cpp', 'utility/encoder/b64Encoder.hpp',
	'utility/encoder/binaryEncoder.cpp', 'utility/encoder/binaryEncoder.hpp',
	'utility/encoder/defaultEncoder.cpp', 'utility/encoder/defaultEncoder.hpp',
	'utility/encoder/encoderFactory.cpp', 'utility/encoder/encoderFactory.hpp',
	'utility/encoder/qpEncoder.cpp', 'utility/encoder/qpEncoder.hpp',
	'utility/encoder/uuEncoder.cpp', 'utility/encoder/uuEncoder.hpp',
	# ===============================  MDN  ================================
	'mdn/MDNHelper.cpp', 'mdn/MDNHelper.hpp',
	'mdn/MDNInfos.cpp', 'mdn/MDNInfos.hpp',
	'mdn/receivedMDNInfos.cpp', 'mdn/receivedMDNInfos.hpp',
	'mdn/sendableMDNInfos.cpp', 'mdn/sendableMDNInfos.hpp',
	# ===============================  Misc  ===============================
	'misc/importanceHelper.cpp', 'misc/importanceHelper.hpp',
	# =============================  Security  =============================
	'security/authenticator.hpp',
	'security/defaultAuthenticator.cpp', 'security/defaultAuthenticator.hpp',
	# -- digest
	'security/digest/messageDigest.cpp', 'security/digest/messageDigest.hpp',
	'security/digest/messageDigestFactory.cpp', 'security/digest/messageDigestFactory.hpp',
	'security/digest/md5/md5MessageDigest.cpp', 'security/digest/md5/md5MessageDigest.hpp',
	'security/digest/sha1/sha1MessageDigest.cpp', 'security/digest/sha1/sha1MessageDigest.hpp'
]

libvmime_security_sasl_sources = [
	'security/sasl/SASLContext.cpp', 'security/sasl/SASLContext.hpp',
	'security/sasl/SASLSession.cpp', 'security/sasl/SASLSession.hpp',
	'security/sasl/SASLMechanism.hpp',
	'security/sasl/SASLMechanismFactory.cpp', 'security/sasl/SASLMechanismFactory.hpp',
	'security/sasl/SASLSocket.cpp', 'security/sasl/SASLSocket.hpp',
	'security/sasl/SASLAuthenticator.hpp',
	'security/sasl/defaultSASLAuthenticator.cpp', 'security/sasl/defaultSASLAuthenticator.hpp',
	'security/sasl/builtinSASLMechanism.cpp', 'security/sasl/builtinSASLMechanism.hpp'
]

libvmime_examples_sources = [
	'examples/README',
#	'examples/Makefile.am',    # not generated
#	'examples/Makefile.in',
	'examples/example1.cpp',
	'examples/example2.cpp',
	'examples/example3.cpp',
	'examples/example4.cpp',
	'examples/example5.cpp',
	'examples/example6.cpp',
	'examples/example7.cpp'
]

libvmime_messaging_sources = [
	'net/builtinServices.inl',
	'net/connectionInfos.hpp',
	'net/defaultConnectionInfos.cpp', 'net/defaultConnectionInfos.hpp',
	'net/events.cpp', 'net/events.hpp',
	'net/folder.cpp', 'net/folder.hpp',
	'net/message.cpp', 'net/message.hpp',
	'net/securedConnectionInfos.hpp',
	'net/service.cpp', 'net/service.hpp',
	'net/serviceFactory.cpp', 'net/serviceFactory.hpp',
	'net/serviceInfos.cpp', 'net/serviceInfos.hpp',
	'net/serviceRegistration.inl',
	'net/session.cpp', 'net/session.hpp',
	'net/socket.hpp',
	'net/store.hpp',
	'net/timeoutHandler.hpp',
	'net/transport.cpp', 'net/transport.hpp'
]

libvmime_net_tls_sources = [
	'net/tls/TLSSession.cpp', 'net/tls/TLSSession.hpp',
	'net/tls/TLSSocket.cpp', 'net/tls/TLSSocket.hpp',
	'net/tls/TLSSecuredConnectionInfos.cpp', 'net/tls/TLSSecuredConnectionInfos.hpp',
	'security/cert/certificateChain.cpp', 'security/cert/certificateChain.hpp',
	'security/cert/certificateVerifier.hpp',
	'security/cert/defaultCertificateVerifier.cpp', 'security/cert/defaultCertificateVerifier.hpp',
	'security/cert/certificate.hpp',
	'security/cert/X509Certificate.cpp', 'security/cert/X509Certificate.hpp'
]

libvmime_messaging_proto_sources = [
	[
		'pop3',
		[
			'net/pop3/POP3ServiceInfos.cpp', 'net/pop3/POP3ServiceInfos.hpp',
			'net/pop3/POP3Store.cpp',        'net/pop3/POP3Store.hpp',
			'net/pop3/POP3SStore.cpp',       'net/pop3/POP3SStore.hpp',
			'net/pop3/POP3Folder.cpp',       'net/pop3/POP3Folder.hpp',
			'net/pop3/POP3Message.cpp',      'net/pop3/POP3Message.hpp',
			'net/pop3/POP3Utils.cpp',        'net/pop3/POP3Utils.hpp'
		]
	],
	[
		'smtp',
		[
			'net/smtp/SMTPResponse.cpp',     'net/smtp/SMTPResponse.hpp',
			'net/smtp/SMTPServiceInfos.cpp', 'net/smtp/SMTPServiceInfos.hpp',
			'net/smtp/SMTPTransport.cpp',    'net/smtp/SMTPTransport.hpp',
			'net/smtp/SMTPSTransport.cpp',   'net/smtp/SMTPSTransport.hpp'
		]
	],
	[
		'imap',
		[
			'net/imap/IMAPServiceInfos.cpp', 'net/imap/IMAPServiceInfos.hpp',
			'net/imap/IMAPConnection.cpp',   'net/imap/IMAPConnection.hpp',
			'net/imap/IMAPStore.cpp',        'net/imap/IMAPStore.hpp',
			'net/imap/IMAPSStore.cpp',       'net/imap/IMAPSStore.hpp',
			'net/imap/IMAPFolder.cpp',       'net/imap/IMAPFolder.hpp',
			'net/imap/IMAPMessage.cpp',      'net/imap/IMAPMessage.hpp',
			'net/imap/IMAPTag.cpp',          'net/imap/IMAPTag.hpp',
			'net/imap/IMAPUtils.cpp',        'net/imap/IMAPUtils.hpp',
			'net/imap/IMAPMessagePartContentHandler.cpp', 'net/imap/IMAPMessagePartContentHandler.hpp',
			'net/imap/IMAPStructure.cpp',    'net/imap/IMAPStructure.hpp',
			'net/imap/IMAPPart.cpp',         'net/imap/IMAPPart.hpp',
			'net/imap/IMAPParser.hpp',
		]
	],
	[
		'maildir',
		[
			'net/maildir/maildirServiceInfos.cpp', 'net/maildir/maildirServiceInfos.hpp',
			'net/maildir/maildirStore.cpp',        'net/maildir/maildirStore.hpp',
			'net/maildir/maildirFolder.cpp',       'net/maildir/maildirFolder.hpp',
			'net/maildir/maildirMessage.cpp',      'net/maildir/maildirMessage.hpp',
			'net/maildir/maildirUtils.cpp',        'net/maildir/maildirUtils.hpp',
			'net/maildir/maildirFormat.cpp',       'net/maildir/maildirFormat.hpp',
			'net/maildir/format/kmailMaildirFormat.cpp',    'net/maildir/format/kmailMaildirFormat.hpp',
			'net/maildir/format/courierMaildirFormat.cpp',  'net/maildir/format/courierMaildirFormat.hpp'
		]
	],
	[
		'sendmail',
		[
			'net/sendmail/sendmailServiceInfos.cpp', 'net/sendmail/sendmailServiceInfos.hpp',
			'net/sendmail/sendmailTransport.cpp',    'net/sendmail/sendmailTransport.hpp'
		]
	]
]

libvmime_platforms_sources = {
	'posix':
	[
		'platforms/posix/posixChildProcess.cpp', 'platforms/posix/posixChildProcess.hpp',
		'platforms/posix/posixFile.cpp', 'platforms/posix/posixFile.hpp',
		'platforms/posix/posixHandler.cpp', 'platforms/posix/posixHandler.hpp',
		'platforms/posix/posixSocket.cpp', 'platforms/posix/posixSocket.hpp'
	],
	'windows':
	[
		'platforms/windows/windowsFile.cpp', 'platforms/windows/windowsFile.hpp',
		'platforms/windows/windowsHandler.cpp', 'platforms/windows/windowsHandler.hpp',
		'platforms/windows/windowsSocket.cpp', 'platforms/windows/windowsSocket.hpp'
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
	'README.msvc',
	'README.refcounting',
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

libvmimetest_common = [
	'tests/testUtils.hpp'
]

libvmimetest_sources = [
	'tests/testRunner.cpp',
	'tests/testUtils.cpp',
	# ==============================  Parser  ==============================
	'tests/parser/attachmentHelperTest.cpp',
	'tests/parser/bodyPartTest.cpp',
	'tests/parser/charsetTest.cpp',
	'tests/parser/datetimeTest.cpp',
	'tests/parser/dispositionTest.cpp',
	'tests/parser/headerTest.cpp',
	'tests/parser/htmlTextPartTest.cpp',
	'tests/parser/mailboxTest.cpp',
	'tests/parser/mediaTypeTest.cpp',
	'tests/parser/messageIdTest.cpp',
	'tests/parser/messageIdSequenceTest.cpp',
	'tests/parser/pathTest.cpp',
	'tests/parser/parameterTest.cpp',
	'tests/parser/textTest.cpp',
	# ==============================  Utility  =============================
	'tests/utility/datetimeUtilsTest.cpp',
	'tests/utility/filteredStreamTest.cpp',
	'tests/utility/stringProxyTest.cpp',
	'tests/utility/stringUtilsTest.cpp',
	'tests/utility/pathTest.cpp',
	'tests/utility/urlTest.cpp',
	'tests/utility/smartPtrTest.cpp',
	'tests/utility/encoderTest.cpp',
	# ===============================  Misc  ===============================
	'tests/misc/importanceHelperTest.cpp',
	# =============================  Security  =============================
	'tests/security/digest/md5Test.cpp',
	'tests/security/digest/sha1Test.cpp',
	# ===============================  Net  ================================
	'tests/net/smtp/SMTPTransportTest.cpp',
	'tests/net/smtp/SMTPResponseTest.cpp',
	'tests/net/maildir/maildirStoreTest.cpp'
]

libvmime_autotools = [
	'm4/acx_pthread.m4',
	'm4/iconv.m4',
	'm4/lib-ld.m4',
	'm4/lib-link.m4',
	'm4/lib-prefix.m4',
	'm4/libgnutls.m4',
	'autotools/install-sh',
#	'autotools/mkinstalldirs',
	'autotools/missing',
	'autotools/config.guess',
	'autotools/config.sub',
	'autotools/ltmain.sh',
#	'autotools/depcomp',
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

libvmime_all_sources = [] + libvmime_sources + libvmime_messaging_sources + libvmime_security_sasl_sources + libvmime_net_tls_sources

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
libvmime_dist_files += libvmimetest_sources
libvmime_dist_files += libvmimetest_common
libvmime_dist_files += libvmime_autotools


#################
#  Set options  #
#################

EnsureSConsVersion(0, 98, 1)

SetOption('implicit_cache', 1)

try:
	Decider('MD5-timestamp')
except:
	SourceSignatures('MD5')
	TargetSignatures('build')


#############
#  Options  #
#############

# Try to guess some default values
defaultSendmailPath = WhereIs("sendmail")

if defaultSendmailPath == None:
	defaultSendmailPath = ''


# Command line options
opts = Variables('options.cache')

opts.AddVariables(
	(
		'prefix',
		'Installation prefix directory',
		'/usr'
	),
	EnumVariable(
		'debug',
		'Debug version (useful for developers only)',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'with_messaging',
		'Messaging support (connection to mail store/transport servers)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
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
		    + 'Currently available protocols: pop3, smtp, imap, maildir, sendmail.',
		'"pop3 smtp imap maildir sendmail"'
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
	EnumVariable(
	 	'with_sasl',
		'Enable SASL support (requires GNU SASL library)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'with_tls',
		'Enable TLS support (requires GNU TLS library)',
		'yes',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	),
	(
		'sendmail_path',
		'Specifies the path to sendmail.',
		defaultSendmailPath
	),
	EnumVariable(
		'byte_order',
		'Byte order (Big Endian or Little Endian)',
		sys.byteorder,
		allowed_values = ('big', 'little'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'pf_8bit_type',
		'The C-language 8-bit type for your platform',
		'char',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'pf_16bit_type',
		'The C-language 16-bit type for your platform',
		'short',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'pf_32bit_type',
		'The C-language 32-bit type for your platform',
		'int',
		allowed_values = ('char', 'short', 'int', 'long'),
		map = { },
		ignorecase = 1
	),
	EnumVariable(
		'build_tests',
		'Build unit tests (run with "scons run-tests")',
		'no',
		allowed_values = ('yes', 'no'),
		map = { },
		ignorecase = 1
	)
)



###############################
#  Configuration Environment  #
###############################

try:
	env = Environment(variables = opts)
except TypeError:
	env = Environment(options = opts)

env.Append(ENV = os.environ)
env.Append(ENV = {'PATH' : os.environ['PATH']})

env.Append(CPPPATH = [ '.' ])

env.Append(CPPDEFINES = ['_REENTRANT=1'])

env.Append(CXXFLAGS = ['-W'])
env.Append(CXXFLAGS = ['-Wall'])
env.Append(CXXFLAGS = ['-ansi'])
env.Append(CXXFLAGS = ['-pedantic'])
env.Append(CXXFLAGS = ['-Wpointer-arith'])
env.Append(CXXFLAGS = ['-Wold-style-cast'])
env.Append(CXXFLAGS = ['-Wconversion'])
env.Append(CXXFLAGS = ['-Wcast-align'])
#env.Append(CXXFLAGS = ['-Wshadow'])

env.Append(TARFLAGS = ['-c'])
env.Append(TARFLAGS = ['--bzip2'])

if env['debug'] == 'yes':
	env.Append(CXXFLAGS = ['-g'])
	env.Append(CXXFLAGS = ['-O0'])
else:
	env.Append(CXXFLAGS = ['-O2'])
	#-fomit-frame-pointer -fmove-all-movables -fstrict-aliasing

#env.Append(LIBS = ['additional-lib-here'])

if env['with_sasl'] == 'yes':
	libgsasl_pc = string.strip(os.popen("pkg-config --list-all | grep '^libgsasl[ ]' | cut -f 1 -d ' '").read())

	if len(libgsasl_pc) == 0:
		print "ERROR: GNU SASL development package is not installed\n"
		Exit(1)

	env.ParseConfig('pkg-config --cflags --libs ' + libgsasl_pc)

if env['with_tls'] == 'yes':
	libgnutls_pc = string.strip(os.popen("pkg-config --list-all | grep '^libgnutls[ ]' | cut -f 1 -d ' '").read())

	if len(libgnutls_pc) == 0:
		libgnutls_pc = string.strip(os.popen("pkg-config --list-all | grep '^gnutls[ ]' | cut -f 1 -d ' '").read())

	if len(libgnutls_pc) == 0:
		print "ERROR: GNU TLS development package is not installed\n"
		Exit(1)

	env.ParseConfig('pkg-config --cflags --libs ' + libgnutls_pc)

env.Append(CXXFLAGS = ['-pthread'])

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

# Sendmail transport is only available on POSIX platforms
if os.name != 'posix':
	if IsProtocolSupported(messaging_protocols, 'sendmail'):
		print "WARNING: ignoring 'sendmail' support (only available on POSIX platforms)\n"

		newProtoList = [ ]

		for p in messaging_protocols:
			if string.upper(p) != "SENDMAIL":
				newProtoList.append(p)

		messaging_protocols = newProtoList

# Check sendmail path
if IsProtocolSupported(messaging_protocols, 'sendmail'):
	if env['sendmail_path'] == '':
		print "ERROR: no path specified for 'sendmail'"
		Exit(1)


###########################
#  Configuration summary  #
###########################

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
print "SASL support             : " + env['with_sasl']
print "TLS/SSL support          : " + env['with_tls']

if IsProtocolSupported(messaging_protocols, 'sendmail'):
	print "Sendmail path            : " + env['sendmail_path']

print ""


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

config_hpp.write('// -- File-system support\n')
if env['with_filesystem'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_FILESYSTEM_FEATURES 0\n')

config_hpp.write('// -- SASL support\n')
if env['with_sasl'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_SASL_SUPPORT 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_SASL_SUPPORT 0\n')

config_hpp.write('// -- TLS/SSL support\n')
if env['with_tls'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_TLS_SUPPORT 1\n')
	config_hpp.write('#define HAVE_GNUTLS_PRIORITY_FUNCS 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_TLS_SUPPORT 0\n')

config_hpp.write('// -- Messaging support\n')
if env['with_messaging'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 1\n')

	config_hpp.write('// -- Built-in messaging protocols\n')
	config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTOS "' +
		string.replace(env['with_messaging_protocols'], '"', '') + '"\n')

	for proto in messaging_protocols:
		config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(proto) + ' 1\n')

	for p in libvmime_messaging_proto_sources:
		proto = p[0]
		if not proto in messaging_protocols:
			config_hpp.write('#define VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(proto) + ' 0\n')
else:
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 0\n')

config_hpp.write('// -- Built-in platform handlers\n')
config_hpp.write('#define VMIME_BUILTIN_PLATFORMS "' +
	string.replace(env['with_platforms'], '"', '') + '"\n')

for platform in platforms:
	config_hpp.write('#define VMIME_BUILTIN_PLATFORM_' + string.upper(platform) + ' 1\n')

for platform in libvmime_platforms_sources:
	if not platform in platforms:
		config_hpp.write('#define VMIME_BUILTIN_PLATFORM_' + string.upper(platform) + ' 0\n')

config_hpp.write('\n')
config_hpp.write('// Miscellaneous flags\n')

if IsProtocolSupported(messaging_protocols, 'sendmail'):
	config_hpp.write('#define VMIME_SENDMAIL_PATH "' + env['sendmail_path'] + '"\n')

config_hpp.write("""

// Additional defines
#define VMIME_HAVE_GETADDRINFO 1
#define VMIME_HAVE_PTHREAD 1


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

# -- SASL support
if env['with_sasl'] == 'yes':
	for file in libvmime_security_sasl_sources:
		libvmime_sel_sources.append(file)

# -- TLS support
if env['with_tls'] == 'yes':
	for file in libvmime_net_tls_sources:
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
		env = env.Clone()
		env.Append(LIBS = ['cppunit', 'dl', packageVersionedGenericName + '-debug', 'pthread'])
		env.Append(LIBPATH=['.'])

		if sys.platform == "mac" or sys.platform == "darwin":
			env.Append(LIBS = ['iconv', 'gcrypt'])

		Default(
			env.Program(
				target = 'run-tests',
				source = libvmimetest_sources
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

vmime_pc_requires = ''
vmime_pc_libs = ''

if env['with_sasl'] == 'yes':
	vmime_pc_requires = vmime_pc_requires + "libgsasl "
	vmime_pc_libs = vmime_pc_libs + "-lgsasl "

vmime_pc.write("prefix=" + env['prefix'] + "\n")
vmime_pc.write("exec_prefix=" + env['prefix'] + "\n")
vmime_pc.write("libdir=" + env['prefix'] + "/lib\n")
vmime_pc.write("includedir=" + env['prefix'] + "/include\n")
vmime_pc.write("\n")
vmime_pc.write("Name: " + packageRealName + "\n")
vmime_pc.write("Description: " + packageDescription + "\n")
vmime_pc.write("Version: " + packageVersion + "\n")
vmime_pc.write("Requires: " + vmime_pc_requires + "\n")
vmime_pc.write("Libs: -L${libdir} -l" + packageVersionedGenericName + " " + vmime_pc_libs + "\n")
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
	vmime_pc_in.write("# DO NOT EDIT!\n")
	vmime_pc_in.write("\n")
	vmime_pc_in.write("prefix=@prefix@\n")
	vmime_pc_in.write("exec_prefix=@exec_prefix@\n")
	vmime_pc_in.write("libdir=@libdir@\n")
	vmime_pc_in.write("includedir=@includedir@\n")
	vmime_pc_in.write("\n")
	vmime_pc_in.write("Name: @GENERIC_LIBRARY_NAME@\n")
	vmime_pc_in.write("Description: " + packageDescription + "\n")
	vmime_pc_in.write("Version: @VERSION@\n")
	vmime_pc_in.write("Requires: @GSASL_REQUIRED@\n")
	vmime_pc_in.write("Libs: -L${libdir} -l@GENERIC_VERSIONED_LIBRARY_NAME@ @GSASL_LIBS@ @LIBGNUTLS_LIBS@ @LIBICONV@ @PTHREAD_LIBS@ @LIBICONV@ @PTHREAD_LIBS@ @VMIME_ADDITIONAL_PC_LIBS@\n")
	#vmime_pc_in.write("Cflags: -I${includedir}/@GENERIC_VERSIONED_LIBRARY_NAME@\n")
	vmime_pc_in.write("Cflags: -I${includedir}/ @LIBGNUTLS_CFLAGS@\n")
	vmime_pc_in.close()

	# Generate 'Makefile.am'
	Makefile_am = open("Makefile.am", 'w')
	Makefile_am.write("""
# File automatically generated by SConstruct ('scons autotools')
# DO NOT EDIT!

BINDING =
INCLUDE = vmime
#examples tests

SUBDIRS = src $(INCLUDE) $(BINDING)

DIST_SUBDIRS = $(SUBDIRS) autotools

#AUTOMAKE_OPTIONS = dist-bzip2
AUTOMAKE_OPTIONS = no-dist

pkgconfigdir = $(VMIME_PKGCONFIGDIR)
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
# DO NOT EDIT!
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
# DO NOT EDIT!

AUTOMAKE_OPTIONS = no-dependencies foreign
INTERNALS =
INCLUDES = -I$(prefix)/include -I$(top_srcdir) @PKGCONFIG_CFLAGS@ @EXTRA_CFLAGS@
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

	# -- SASL support
	x = selectFilesFromSuffixNot(libvmime_security_sasl_sources, '.hpp')
	sourceFiles += x

	Makefile_am.write("\n")
	Makefile_am.write("if VMIME_HAVE_SASL_SUPPORT\n")
	Makefile_am.write(packageVersionedName + "_la_SOURCES += " + buildMakefileFileList(x, 1) + "\n")
	Makefile_am.write("endif\n")

	# -- TLS support
	x = selectFilesFromSuffixNot(libvmime_net_tls_sources, '.hpp')
	sourceFiles += x

	Makefile_am.write("\n")
	Makefile_am.write("if VMIME_HAVE_TLS_SUPPORT\n")
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
# DO NOT EDIT!

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

VMIME_ADDITIONAL_DEFINES=""
VMIME_ADDITIONAL_PC_LIBS=""


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

OST_LIB_PTHREAD  # from GNU Commons C++

AM_SANITY_CHECK
AM_PROG_LIBTOOL
AM_PROG_CC_C_O

AM_ICONV


#
# Some checks
#

# -- iconv
AC_MSG_CHECKING([if an usable version of iconv exists (required)])

if test "x$am_cv_func_iconv" = "xyes"; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_ERROR(no usable version of iconv has been found)
fi

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

if test -x mytest -a "$cross_compiling" != yes; then
	myresult=`./mytest`
	if test "X$myresult" = "XPASS"; then
		AC_MSG_RESULT(yes)
	else
		AC_MSG_RESULT(no)
		AC_ERROR([
===================================================================
ERROR: This platform lacks support of construction of global
objects in shared libraries.

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
   class foo { public: virtual ~foo() { } };
   class bar : public foo { public: virtual ~bar() { } };
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
$ac_cv_sizeof_long)
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
     AC_HELP_STRING([--enable-debug], [Turn on debugging, default: disabled]),
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
     AC_HELP_STRING([--enable-messaging], [Enable messaging support and connection to mail servers, default: enabled]),
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

# ** SASL

AC_ARG_ENABLE(sasl,
     AC_HELP_STRING([--enable-sasl], [Enable SASL support with GNU SASL, default: enabled]),
     [case "${enableval}" in
       yes) conf_sasl=yes ;;
       no)  conf_sasl=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-sasl) ;;
      esac],
     [conf_sasl=yes])

if test "x$conf_sasl" = "xyes"; then
	# -- GNU SASL Library (http://www.gnu.org/software/gsasl/)
	AC_CHECK_HEADER(gsasl.h,
		AC_CHECK_LIB(gsasl, gsasl_check_version,
			[have_gsasl=yes AC_SUBST(GSASL_AVAIL_LIBS, -lgsasl) AC_SUBST(GSASL_AVAIL_REQUIRED, libgsasl)],
			 have_gsasl=no),
		have_gsasl=no)

	if test "x$have_gsasl" = "xyes"; then
		AM_CONDITIONAL(VMIME_HAVE_SASL_SUPPORT, true)
		VMIME_HAVE_SASL_SUPPORT=1

		GSASL_REQUIRED=${GSASL_AVAIL_REQUIRED}
		GSASL_LIBS=${GSASL_AVAIL_LIBS}
	else
		AC_MSG_ERROR(can't find an usable version of GNU SASL library)
	fi
else
	AM_CONDITIONAL(VMIME_HAVE_SASL_SUPPORT, false)
	VMIME_HAVE_SASL_SUPPORT=0

	GSASL_REQUIRED=
	GSASL_LIBS=
fi

AC_SUBST(GSASL_REQUIRED)
AC_SUBST(GSASL_LIBS)

# ** TLS

AC_ARG_ENABLE(tls,
     AC_HELP_STRING([--enable-tls], [Enable TLS/SSL support with GNU TLS, default: enabled]),
     [case "${enableval}" in
       yes) conf_tls=yes ;;
       no)  conf_tls=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-tls) ;;
      esac],
     [conf_tls=yes])

if test "x$conf_tls" = "xyes"; then
	# -- GNU TLS Library (http://www.gnu.org/software/gnutls/)
	PKG_CHECK_MODULES([LIBGNUTLS], [gnutls >= 1.2.0], have_gnutls=yes, have_gnutls=no)

	if test "x$have_gnutls" = "xyes"; then
		AM_CONDITIONAL(VMIME_HAVE_TLS_SUPPORT, true)
		VMIME_HAVE_TLS_SUPPORT=1
	else
		AC_MSG_ERROR(can't find an usable version of GNU TLS library)
	fi

	# -- check for gnutls_priority_set_direct() function
	if test "x$have_gnutls" = "xyes"; then
		AC_MSG_CHECKING(for gnutls_priority_set_direct)

		LIBS_save="$LIBS"
		LIBS="$LIBS $LIBGNUTLS_LIBS"
		CPPFLAGS_save="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $LIBGNUTLS_CFLAGS"

		AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <gnutls/gnutls.h>],
		                                [gnutls_session s; gnutls_priority_set_direct(s, NULL, NULL);])],
		               [have_gnutls_priority_funcs=yes],
		               [have_gnutls_priority_funcs=no])

		CPPFLAGS="$CPPFLAGS_save"
		LIBS="$LIBS_save"

		AC_MSG_RESULT([$have_gnutls_priority_funcs])

		if test "x$have_gnutls_priority_funcs" = "xyes"; then
			AM_CONDITIONAL(HAVE_GNUTLS_PRIORITY_FUNCS, true)
			HAVE_GNUTLS_PRIORITY_FUNCS=1
		else
			AM_CONDITIONAL(HAVE_GNUTLS_PRIORITY_FUNCS, false)
			HAVE_GNUTLS_PRIORITY_FUNCS=0
		fi
	else
		AM_CONDITIONAL(HAVE_GNUTLS_PRIORITY_FUNCS, false)
		HAVE_GNUTLS_PRIORITY_FUNCS=0
	fi
else
	AM_CONDITIONAL(VMIME_HAVE_TLS_SUPPORT, false)
	VMIME_HAVE_TLS_SUPPORT=0

	AM_CONDITIONAL(HAVE_GNUTLS_PRIORITY_FUNCS, false)
	HAVE_GNUTLS_PRIORITY_FUNCS=0
fi

AC_SUBST(LIBGNUTLS_CFLAGS)
AC_SUBST(LIBGNUTLS_LIBS)

# ** platform handlers

VMIME_BUILTIN_PLATFORMS=''
VMIME_DETECT_PLATFORM=''

case "x${target_os}" in
xwin* | xmingw* | xcygwin*)
	VMIME_DETECT_PLATFORM='windows'
	;;
x*)
	# Default is POSIX
	VMIME_DETECT_PLATFORM='posix'
	;;
esac

# ** messaging protocols

VMIME_BUILTIN_MESSAGING_PROTOS=''

""")

	for proto in libvmime_messaging_proto_sources:
		p = proto[0]

		configure_in.write("AC_ARG_ENABLE(messaging-proto-" + p + ",\n")
		configure_in.write("     AC_HELP_STRING([--enable-messaging-proto-" + p
			+ "], [Enable built-in support for protocol '" + p + "'"
			+ ", default: enabled]),\n")
		configure_in.write('     [case "${enableval}" in\n')
		configure_in.write('       yes) conf_messaging_proto_' + p + '=yes ;;\n')
		configure_in.write('       no)  conf_messaging_proto_' + p + '=no ;;\n')
		configure_in.write('       *) AC_MSG_ERROR(bad value ${enableval} for '
			+ '--enable-messaging-proto-' + p + ') ;;\n')
		configure_in.write('      esac],\n')
		configure_in.write('     [conf_messaging_proto_' + p + '=yes])\n')

		configure_in.write('if test "x$conf_messaging_proto_' + p + '" = "xyes"; then\n')

		if p == 'sendmail':  # sendmail only on POSIX platforms
			configure_in.write('if test "x$VMIME_DETECT_PLATFORM" = "xposix"; then\n')

		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + ', true)\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + '=1\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTOS="$VMIME_BUILTIN_MESSAGING_PROTOS ' + p + '"\n')

		if p == 'sendmail':  # sendmail only on POSIX platforms
			configure_in.write('else\n');
			configure_in.write('	AC_MSG_WARN(' + p +' is only available on POSIX platforms)\n');
			configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + ', false)\n')
			configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + '=0\n')
			configure_in.write('fi\n');

		configure_in.write('else\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + ', false)\n')
		configure_in.write('	VMIME_BUILTIN_MESSAGING_PROTO_' + string.upper(p) + '=0\n')
		configure_in.write('fi\n\n')

	configure_in.write("""


#
# System mail
#

AC_PATH_PROG(SENDMAIL, sendmail, /usr/sbin/sendmail, /usr/sbin:/usr/lib)


#
# Detect some platform-specific stuff
#

# -- MLang (Windows)
if test "x$VMIME_DETECT_PLATFORM" = "xwindows"; then
	AC_CHECK_HEADER(mlang.h, [VMIME_ADDITIONAL_DEFINES="$VMIME_ADDITIONAL_DEFINES HAVE_MLANG_H"])
fi

# -- Link with Winsock (Windows)
if test "x$VMIME_DETECT_PLATFORM" = "xwindows"; then
	VMIME_ADDITIONAL_PC_LIBS="$VMIME_ADDITIONAL_PC_LIBS -lws2_32"
fi

# -- getaddrinfo (POSIX)
if test "x$VMIME_DETECT_PLATFORM" = "xposix"; then
	AC_CHECK_HEADERS(netdb.h sys/types.h sys/socket.h,)
	AC_CHECK_FUNC(getaddrinfo, [VMIME_ADDITIONAL_DEFINES="$VMIME_ADDITIONAL_DEFINES HAVE_GETADDRINFO"])
fi

# -- pthreads (POSIX)

ACX_PTHREAD([VMIME_ADDITIONAL_DEFINES="$VMIME_ADDITIONAL_DEFINES HAVE_PTHREAD"])


""")


	for p in libvmime_platforms_sources:
		configure_in.write('if test "x$VMIME_DETECT_PLATFORM" = "x' + p + '"; then\n')
		configure_in.write('	conf_platform_' + p + '=yes\n')
		configure_in.write('else\n')
		configure_in.write('	conf_platform_' + p + '=no\n')
		configure_in.write('fi\n\n')

		configure_in.write("AC_ARG_ENABLE(platform-" + p + ",\n")
		configure_in.write("     AC_HELP_STRING([--enable-platform-" + p
			+ "], [Compile built-in platform handler for '" + p + "' "
			+ ", default: disabled, except if default for your platform]),\n")
		configure_in.write('     [case "${enableval}" in\n')
		configure_in.write('       yes) conf_platform_' + p + '=yes ;;\n')
		configure_in.write('       no)  conf_platform_' + p + '=no ;;\n')
		configure_in.write('       *) AC_MSG_ERROR(bad value ${enableval} for '
			+ '--enable-platform-' + p + ') ;;\n')
		configure_in.write('      esac],\n')
		#configure_in.write('     [conf_platform_' + p + '=yes])\n')
		configure_in.write('      [])\n')

		configure_in.write('if test "x$conf_platform_' + p + '" = "xyes"; then\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_PLATFORM_' + string.upper(p) + ', true)\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORM_' + string.upper(p) + '=1\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORMS="$VMIME_BUILTIN_PLATFORMS ' + p + '"\n')
		configure_in.write('else\n')
		configure_in.write('	AM_CONDITIONAL(VMIME_BUILTIN_PLATFORM_' + string.upper(p) + ', false)\n')
		configure_in.write('	VMIME_BUILTIN_PLATFORM_' + string.upper(p) + '=0\n')
		configure_in.write('fi\n\n')

	configure_in.write("""

#
# Workarounds for some platforms
#

# -- pkgconfigdir
case "x${target_os}" in
xfreebsd*)

	VMIME_PKGCONFIGDIR='$(prefix)/libdata/pkgconfig'
	;;

x*)

	VMIME_PKGCONFIGDIR='$(libdir)/pkgconfig'
	;;
esac

AC_SUBST(VMIME_PKGCONFIGDIR)

# -- libtool 'CXX' tag
case "x${target_os}" in
xfreebsd*)
	# FIXME: temporary hack until I find a better solution
	# to make libtool use the C++ tag...
	LIBTOOL="$LIBTOOL --tag=CXX"
	;;
esac


#
# Flags
#

LIBRARY_LD_FLAGS="\$(top_builddir)/src/\$(LIBRARY_NAME).la"
AC_SUBST(LIBRARY_LD_FLAGS)

PKGCONFIG_CFLAGS=""
PKGCONFIG_LIBS=""

AC_SUBST(PKGCONFIG_CFLAGS)
AC_SUBST(PKGCONFIG_LIBS)

EXTRA_CFLAGS="$EXTRA_CFLAGS -D_REENTRANT=1 -D_THREAD_SAFE=1 $LIBGNUTLS_CFLAGS"
EXTRA_LIBS="$GSASL_LIBS $LIBGNUTLS_LIBS"

CFLAGS=""
CXXFLAGS=""

# -- Debug
if test x$VMIME_DEBUG = x1 ; then
	# -g
	OLD_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -g"
	AC_MSG_CHECKING(whether cc accepts -g)
	AC_TRY_COMPILE(,,echo yes,echo no; CXXFLAGS="$OLD_CXXFLAGS")
else
	# -O2
	OLD_CXXFLAGS="$CXXFLAGS"
	CXXFLAGS="$CXXFLAGS -O2"
	AC_MSG_CHECKING(whether cc accepts -O2)
	AC_TRY_COMPILE(,,echo yes,echo no; CXXFLAGS="$OLD_CXXFLAGS")
fi

# -- HACK: add -fPIC or -fpic on static library object files
EXTRA_CFLAGS="$EXTRA_CFLAGS $lt_prog_compiler_pic"



#
# Check to see if the compiler can handle some flags
#

""")

	compilerFlags = [ '-ansi', '-pedantic', '-W', '-Wall', '-Wpointer-arith', '-Wold-style-cast', '-Wconversion' ]

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

AC_SUBST(VMIME_ADDITIONAL_PC_LIBS)

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
// -- File-system support
#define VMIME_HAVE_FILESYSTEM_FEATURES 1
// -- SASL support
#define VMIME_HAVE_SASL_SUPPORT ${VMIME_HAVE_SASL_SUPPORT}
// -- TLS support
#define VMIME_HAVE_TLS_SUPPORT ${VMIME_HAVE_TLS_SUPPORT}
#define HAVE_GNUTLS_PRIORITY_FUNCS ${HAVE_GNUTLS_PRIORITY_FUNCS}
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

	for p in libvmime_platforms_sources:
		p = string.upper(p)
		configure_in.write('#define VMIME_BUILTIN_PLATFORM_' + p
			+ " $VMIME_BUILTIN_PLATFORM_" + p + " \n")

	configure_in.write("""
" > vmime/config.hpp

# Miscellaneous flags
echo "// Miscellaneous flags" >> vmime/config.hpp
echo "#define VMIME_SENDMAIL_PATH \\"$SENDMAIL\\"" >> vmime/config.hpp
echo "" >> vmime/config.hpp

# Additional defines
echo "// Additional defines" >> vmime/config.hpp

for d in $VMIME_ADDITIONAL_DEFINES ; do
	echo "#define VMIME_$d 1" >> vmime/config.hpp
done


echo "" >> vmime/config.hpp
echo "#endif // VMIME_CONFIG_HPP_INCLUDED" >> vmime/config.hpp


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
SASL support             : $conf_sasl
TLS/SSL support          : $conf_tls

Please check 'vmime/config.hpp' to ensure the configuration is correct.
])
""")

	configure_in.close()

	os.system('bash bootstrap')

	return None


# Custom builder for generating autotools scripts
autotoolsBuilder = Builder(action = generateAutotools)
env.Append(BUILDERS = { 'GenerateAutoTools' : autotoolsBuilder })

env.Alias('autotools', env.GenerateAutoTools('foo_autotools', 'SConstruct'))



################################
#  Generate MSVC project files #
################################

MSVC_filesDone = []
MSVC_dupCounter = 1      # counter for duplicate file names

def generateMSVC(target, source, env):
	# vmime.sln
	vmime_sln = open("vmime.sln", 'w')
	vmime_sln.write("""Microsoft Visual Studio Solution File, Format Version 8.00
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "vmime", "vmime.vcproj", "{B2B47E11-DB57-49E1-8895-F03BDF78A221}"
	ProjectSection(ProjectDependencies) = postProject
	EndProjectSection
EndProject
Global
	GlobalSection(SolutionConfiguration) = preSolution
		Debug = Debug
		Release = Release
	EndGlobalSection
	GlobalSection(ProjectConfiguration) = postSolution
		{B2B47E11-DB57-49E1-8895-F03BDF78A221}.Debug.ActiveCfg = Debug|Win32
		{B2B47E11-DB57-49E1-8895-F03BDF78A221}.Debug.Build.0 = Debug|Win32
		{B2B47E11-DB57-49E1-8895-F03BDF78A221}.Release.ActiveCfg = Release|Win32
		{B2B47E11-DB57-49E1-8895-F03BDF78A221}.Release.Build.0 = Release|Win32
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
	EndGlobalSection
	GlobalSection(ExtensibilityAddIns) = postSolution
	EndGlobalSection
EndGlobal
""")
	vmime_sln.close();

	# vmime.vcproj
	vmime_vcproj = open("vmime.vcproj", 'w')
	vmime_vcproj.write("""<?xml version="1.0" encoding="Windows-1252"?>
<!-- This file has been automatically generated from SConstruct -->
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.10"
	Name="vmime"
	ProjectGUID="{B2B47E11-DB57-49E1-8895-F03BDF78A221}"
	RootNamespace="vmime"
	Keyword="Win32Proj">
	<Platforms>
		<Platform Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="Debug"
			IntermediateDirectory="Debug"
			ConfigurationType="4"
			CharacterSet="2">
			<Tool Name="VCCLCompilerTool"
			      Optimization="0"
			      AdditionalIncludeDirectories="."
			      PreprocessorDefinitions="WIN32;_DEBUG;_LIB"
			      MinimalRebuild="TRUE"
			      BasicRuntimeChecks="3"
			      RuntimeLibrary="5"
			      RuntimeTypeInfo="TRUE"
			      UsePrecompiledHeader="0"
			      WarningLevel="3"
			      Detect64BitPortabilityProblems="TRUE"
			      DebugInformationFormat="4"
			      DisableSpecificWarnings="4101;4244;4250;4267;4355"/>
			<Tool Name="VCCustomBuildTool"/>
			<Tool Name="VCLibrarianTool"
			      AdditionalDependencies="iconv.lib ws2_32.lib"
			      OutputFile="$(OutDir)/vmime.lib"/>
			<Tool Name="VCMIDLTool"/>
			<Tool Name="VCPostBuildEventTool"/>
			<Tool Name="VCPreBuildEventTool"
			      CommandLine="copy $(InputDir)config.hpp.msvc $(InputDir)vmime\config.hpp"/>
			<Tool Name="VCPreLinkEventTool"/>
			<Tool Name="VCResourceCompilerTool"/>
			<Tool Name="VCWebServiceProxyGeneratorTool"/>
			<Tool Name="VCXMLDataGeneratorTool"/>
			<Tool Name="VCManagedWrapperGeneratorTool"/>
			<Tool Name="VCAuxiliaryManagedWrapperGeneratorTool"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="Release"
			IntermediateDirectory="Release"
			ConfigurationType="4"
			CharacterSet="2">
			<Tool Name="VCCLCompilerTool"
			      AdditionalIncludeDirectories="."
			      PreprocessorDefinitions="WIN32;NDEBUG;_LIB"
			      RuntimeLibrary="4"
			      RuntimeTypeInfo="TRUE"
			      UsePrecompiledHeader="0"
			      WarningLevel="3"
			      Detect64BitPortabilityProblems="TRUE"
			      DebugInformationFormat="3"
			      DisableSpecificWarnings="4101;4244;4250;4267;4355"/>
			<Tool Name="VCCustomBuildTool"/>
			<Tool Name="VCLibrarianTool"
			      AdditionalDependencies="iconv.lib ws2_32.lib"
			      OutputFile="$(OutDir)/vmime.lib"/>
			<Tool Name="VCMIDLTool"/>
			<Tool Name="VCPostBuildEventTool"/>
			<Tool Name="VCPreBuildEventTool"
			      CommandLine="copy $(InputDir)config.hpp.msvc $(InputDir)vmime\config.hpp"/>
			<Tool Name="VCPreLinkEventTool"/>
			<Tool Name="VCResourceCompilerTool"/>
			<Tool Name="VCWebServiceProxyGeneratorTool"/>
			<Tool Name="VCXMLDataGeneratorTool"/>
			<Tool Name="VCManagedWrapperGeneratorTool"/>
			<Tool Name="VCAuxiliaryManagedWrapperGeneratorTool"/>
		</Configuration>
	</Configurations>
	<References>
	</References>
""")

	# Source files
	all_sources = libvmime_sel_sources

	# -- Remove all platform files and re-add files for "windows" only
	for i in range(len(all_sources)):
		if string.find(all_sources[i], 'platforms/') != -1:
			all_sources[i] = ''

	for f in libvmime_platforms_sources['windows']:
		all_sources.append(f)

	# -- Prepend with 'src' (for source files) or 'vmime' (for includes)
	for i in range(len(all_sources)):
		f = all_sources[i]
		if f[-4:] == '.cpp':
			all_sources[i] = 'src/' + f
		else:
			all_sources[i] = 'vmime/' + f

	# -- Replace '/' with '\'
	for i in range(len(all_sources)):
		all_sources[i] = string.replace(all_sources[i], '/', '\\')

	all_sources.sort()

	# -- Sort by directory
	filesInDir = {}

	for f in all_sources:
		if len(f) != 0:
			comps = re.split('\\\\', f)
			l = len(comps) - 1

			tmp = filesInDir

			for i in range(len(comps) - 1):
				d = '*' + comps[i]

				if not tmp.has_key(d):
					tmp[d] = {}

				tmp = tmp[d]

			tmp['file%i' % len(tmp)] = f

	# -- Output files in filters
	vmime_vcproj.write("""
	<Files>
		<Filter Name="Source Files"
		        Filter="cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
		        UniqueIdentifier="{4FC737F1-C7A5-4376-A066-2A32D752A2FF}">
""")

	def MSVC_OutputFiles(filesInDir):
		global MSVC_filesDone, MSVC_dupCounter

		for k in filesInDir.keys():
			f = filesInDir[k]

			# Directory
			if k[0] == '*':
				vmime_vcproj.write('<Filter Name="' + k[1:] + '">\n')
				MSVC_OutputFiles(f)
				vmime_vcproj.write('</Filter>\n')
			# File
			else:
				fn = f[string.rfind(f, '\\') + 1:]

				if len(fn) != 0:
					if fn in MSVC_filesDone:
						# File (duplicate filename)
						vmime_vcproj.write('<File RelativePath=".\\' + f + '">\n')
						vmime_vcproj.write("""	<FileConfiguration Name="Debug|Win32">
		<Tool Name="VCCLCompilerTool" ObjectFile="$(IntDir)/$(InputName)""" + str(MSVC_dupCounter) + """.obj"/>
	</FileConfiguration>
	<FileConfiguration Name="Release|Win32">
		<Tool Name="VCCLCompilerTool" ObjectFile="$(IntDir)/$(InputName)""" + str(MSVC_dupCounter) + """.obj"/>
	</FileConfiguration>
""")
						vmime_vcproj.write('</File>')
						MSVC_dupCounter = MSVC_dupCounter + 1
					else:
						# File
						vmime_vcproj.write('<File RelativePath=".\\' + f + '"/>\n')
						MSVC_filesDone.append(fn)


	MSVC_OutputFiles(filesInDir)

	vmime_vcproj.write("""		</Filter>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
""")
	vmime_vcproj.close();

	# config.hpp.msvc
	config_hpp_msvc = open("config.hpp.msvc", 'w')
	config_hpp_msvc.write("""
//
// This file was automatically generated by configuration script.
//

#ifndef VMIME_CONFIG_HPP_INCLUDED
#define VMIME_CONFIG_HPP_INCLUDED


// Name of package
#define VMIME_PACKAGE """ + '"' + packageName + '"' + """

// Version number of package
#define VMIME_VERSION """ + '"' + packageVersion + '"' + """
#define VMIME_API """ + '"' + packageAPI + '"' + """

// Target OS and architecture
#define VMIME_TARGET_ARCH "i686"
#define VMIME_TARGET_OS "windows"

// Set to 1 if debugging should be activated
#ifdef _DEBUG
#    define VMIME_DEBUG 1
#else
#    define VMIME_DEBUG 0
#endif

// Byte order (set one or the other, but not both!)
#define VMIME_BYTE_ORDER_BIG_ENDIAN    0
#define VMIME_BYTE_ORDER_LITTLE_ENDIAN 1

// Generic types
// -- 8-bit
typedef signed char vmime_int8;
typedef unsigned char vmime_uint8;
// -- 16-bit
typedef signed short vmime_int16;
typedef unsigned short vmime_uint16;
// -- 32-bit
typedef signed int vmime_int32;
typedef unsigned int vmime_uint32;


// Options
// -- File-system support
#define VMIME_HAVE_FILESYSTEM_FEATURES 1
// -- SASL support
#define VMIME_HAVE_SASL_SUPPORT 1
// -- TLS/SSL support
#define VMIME_HAVE_TLS_SUPPORT 1
// -- Messaging support
#define VMIME_HAVE_MESSAGING_FEATURES 1
// -- Built-in messaging protocols
#define VMIME_BUILTIN_MESSAGING_PROTOS "pop3 smtp imap maildir"
#define VMIME_BUILTIN_MESSAGING_PROTO_POP3 1
#define VMIME_BUILTIN_MESSAGING_PROTO_SMTP 1
#define VMIME_BUILTIN_MESSAGING_PROTO_IMAP 1
#define VMIME_BUILTIN_MESSAGING_PROTO_MAILDIR 1
// -- Built-in platform handlers
#define VMIME_BUILTIN_PLATFORMS "windows"
#define VMIME_BUILTIN_PLATFORM_WINDOWS 1

// Miscellaneous
#define VMIME_INLINE_TEMPLATE_SPECIALIZATION 1
#define VMIME_NO_MULTIPLE_INHERITANCE 1
""")

	for p in libvmime_platforms_sources:
		if not (p == 'windows'):
			p = string.upper(p)
			config_hpp_msvc.write('#define VMIME_BUILTIN_PLATFORM_' + p + ' 0\n')

	config_hpp_msvc.write("""

#endif // VMIME_CONFIG_HPP_INCLUDED
""")
	config_hpp_msvc.close()

	libvmime_dist_files.append("vmime.sln")
	libvmime_dist_files.append("vmime.vcproj")
	libvmime_dist_files.append("config.hpp.msvc")

	return None



# Custom builder for generating MSVC project files
msvcBuilder = Builder(action = generateMSVC)
env.Append(BUILDERS = { 'GenerateMSVC' : msvcBuilder })

env.Alias('msvc', env.GenerateMSVC('foo_msvc', 'SConstruct'))



#####################
#  Packaging rules  #
#####################

def appendAdditionalDistFiles():
	# Generate autotools-related files
	generateAutotools([], [], env)
	# Generate MSVC-related files
	generateMSVC([], [], env)


# 'tar' is not available under Windows...
if not (os.name == 'win32' or os.name == 'nt'):
	def createPackage(target, source, env):
		packageFullName = packageName + '-' + packageVersion
		packageFile = packageFullName + '.tar.bz2'

		appendAdditionalDistFiles()

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


################
#  Unit tests  #
################

def runTests(target, source, env):
	os.system("./run-tests")
	return None


runTestsBuilder = Builder(action = runTests)
env.Append(BUILDERS = { 'RunTests' : runTestsBuilder })

env.Alias('run-tests', env.RunTests('foo', 'SConstruct'))

