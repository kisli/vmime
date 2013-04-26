#
# SConstruct file for VMime
#
# Deprecated: use only for development purpose.
# Use CMake for distribution/packaging.
#
# Process this file with 'scons' to build the project.
# For more information, please visit: http://www.scons.org/ .
#
# Usage:
#
#   . scons               build the library
#   . scons -h            see available configuration options
#   . scons opt=value     set a configuration option
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

# Package information
packageName = 'libvmime'
packageGenericName = 'vmime'
packageRealName = 'VMime Library'
packageDescription = 'VMime C++ Mail Library (http://www.vmime.org)'
packageMaintainer = 'vincent@vmime.org'

packageVersion = '0.0.0'
packageAPI = '0:0:0'

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
	'charsetConverter_iconv.cpp', 'charsetConverter_iconv.hpp',
	'charsetConverter_icu.cpp', 'charsetConverter_icu.hpp',
	'charsetConverter_idna.cpp', 'charsetConverter_idna.hpp',
	'charsetConverterOptions.cpp', 'charsetConverterOptions.hpp',
	'component.cpp', 'component.hpp',
	'constants.cpp', 'constants.hpp',
	'contentDisposition.cpp', 'contentDisposition.hpp',
	'contentDispositionField.cpp', 'contentDispositionField.hpp',
	'contentHandler.cpp', 'contentHandler.hpp',
	'contentTypeField.cpp', 'contentTypeField.hpp',
	'context.hpp', 'context.cpp',
	'dateTime.cpp', 'dateTime.hpp',
	'defaultAttachment.cpp', 'defaultAttachment.hpp',
	'disposition.cpp', 'disposition.hpp',
	'emailAddress.cpp', 'emailAddress.hpp',
	'emptyContentHandler.cpp', 'emptyContentHandler.hpp',
	'encoding.cpp', 'encoding.hpp',
	'exception.cpp', 'exception.hpp',
	'fileAttachment.cpp', 'fileAttachment.hpp',
	'generatedMessageAttachment.hpp', 'generatedMessageAttachment.cpp',
	'generationContext.hpp', 'generationContext.cpp',
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
	'path.cpp', 'path.hpp',
	'parameter.cpp', 'parameter.hpp',
	'parameterizedHeaderField.cpp', 'parameterizedHeaderField.hpp',
	'parsedMessageAttachment.cpp', 'parsedMessageAttachment.hpp',
	'parserHelpers.hpp',
	'parsingContext.hpp', 'parsingContext.cpp',
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
	'utility/sync/autoLock.hpp',
	'utility/sync/criticalSection.cpp', 'utility/sync/criticalSection.hpp',
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
	'net/tls/gnutls/TLSSession_GnuTLS.cpp', 'net/tls/gnutls/TLSSession_GnuTLS.hpp',
	'net/tls/gnutls/TLSSocket_GnuTLS.cpp', 'net/tls/gnutls/TLSSocket_GnuTLS.hpp',
	'net/tls/openssl/TLSSession_OpenSSL.cpp', 'net/tls/openssl/TLSSession_OpenSSL.hpp',
	'net/tls/openssl/TLSSocket_OpenSSL.cpp', 'net/tls/openssl/TLSSocket_OpenSSL.hpp',
	'net/tls/openssl/OpenSSLInitializer.cpp', 'net/tls/openssl/OpenSSLInitializer.hpp',
	'net/tls/TLSSecuredConnectionInfos.cpp', 'net/tls/TLSSecuredConnectionInfos.hpp',
	'security/cert/certificateChain.cpp', 'security/cert/certificateChain.hpp',
	'security/cert/certificateVerifier.hpp',
	'security/cert/defaultCertificateVerifier.cpp', 'security/cert/defaultCertificateVerifier.hpp',
	'security/cert/certificate.hpp',
	'security/cert/X509Certificate.cpp', 'security/cert/X509Certificate.hpp',
	'security/cert/gnutls/X509Certificate_GnuTLS.cpp', 'security/cert/gnutls/X509Certificate_GnuTLS.hpp',
	'security/cert/openssl/X509Certificate_OpenSSL.cpp', 'security/cert/openssl/X509Certificate_OpenSSL.hpp'
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
			'net/pop3/POP3Response.cpp',     'net/pop3/POP3Response.hpp',
			'net/pop3/POP3Utils.cpp',        'net/pop3/POP3Utils.hpp'
		]
	],
	[
		'smtp',
		[
			'net/smtp/SMTPCommand.cpp',      'net/smtp/SMTPCommand.hpp',
			'net/smtp/SMTPCommandSet.cpp',   'net/smtp/SMTPCommandSet.hpp',
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
		'platforms/posix/posixCriticalSection.cpp', 'platforms/posix/posixCriticalSection.hpp',
		'platforms/posix/posixFile.cpp', 'platforms/posix/posixFile.hpp',
		'platforms/posix/posixHandler.cpp', 'platforms/posix/posixHandler.hpp',
		'platforms/posix/posixSocket.cpp', 'platforms/posix/posixSocket.hpp'
	],
	'windows':
	[
		'platforms/windows/windowsCriticalSection.cpp', 'platforms/windows/windowsCriticalSection.hpp',
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
	'tests/testUtils.hpp',
	'tests/utility/encoder/encoderTestUtils.hpp'
]

libvmimetest_sources = [
	'tests/testRunner.cpp',
	'tests/testUtils.cpp',
	# ==============================  Parser  ==============================
	'tests/parser/attachmentHelperTest.cpp',
	'tests/parser/bodyPartTest.cpp',
	'tests/parser/charsetTest.cpp',
	'tests/parser/charsetFilteredOutputStreamTest.cpp',
	'tests/parser/datetimeTest.cpp',
	'tests/parser/dispositionTest.cpp',
	'tests/parser/emailAddressTest.cpp',
	'tests/parser/headerTest.cpp',
	'tests/parser/headerFieldTest.cpp',
	'tests/parser/htmlTextPartTest.cpp',
	'tests/parser/mailboxGroupTest.cpp',
	'tests/parser/mailboxTest.cpp',
	'tests/parser/mediaTypeTest.cpp',
	'tests/parser/messageIdTest.cpp',
	'tests/parser/messageIdSequenceTest.cpp',
	'tests/parser/pathTest.cpp',
	'tests/parser/parameterTest.cpp',
	'tests/parser/textTest.cpp',
	'tests/parser/wordEncoderTest.cpp',
	# ==============================  Utility  =============================
	'tests/utility/datetimeUtilsTest.cpp',
	'tests/utility/filteredStreamTest.cpp',
	'tests/utility/stringProxyTest.cpp',
	'tests/utility/stringUtilsTest.cpp',
	'tests/utility/pathTest.cpp',
	'tests/utility/urlTest.cpp',
	'tests/utility/smartPtrTest.cpp',
	'tests/utility/encoder/qpEncoderTest.cpp',
	'tests/utility/encoder/b64EncoderTest.cpp',
	'tests/utility/outputStreamStringAdapterTest.cpp',
	'tests/utility/outputStreamSocketAdapterTest.cpp',
	'tests/utility/outputStreamByteArrayAdapterTest.cpp',
	'tests/utility/seekableInputStreamRegionAdapterTest.cpp',
	# ===============================  Misc  ===============================
	'tests/misc/importanceHelperTest.cpp',
	# =============================  Security  =============================
	'tests/security/digest/md5Test.cpp',
	'tests/security/digest/sha1Test.cpp',
	# ===============================  Net  ================================
	'tests/net/pop3/POP3ResponseTest.cpp',
	'tests/net/pop3/POP3UtilsTest.cpp',
	'tests/net/imap/IMAPTagTest.cpp',
	'tests/net/smtp/SMTPTransportTest.cpp',
	'tests/net/smtp/SMTPCommandTest.cpp',
	'tests/net/smtp/SMTPCommandSetTest.cpp',
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
		'yes',
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
env.Append(CXXFLAGS = ['-Wno-long-long'])  # OpenSSL
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
	# GnuTLS
	libgnutls_pc = string.strip(os.popen("pkg-config --list-all | grep '^libgnutls[ ]' | cut -f 1 -d ' '").read())

	if len(libgnutls_pc) == 0:
		libgnutls_pc = string.strip(os.popen("pkg-config --list-all | grep '^gnutls[ ]' | cut -f 1 -d ' '").read())

	if len(libgnutls_pc) == 0:
		print "ERROR: GNU TLS development package is not installed\n"
		Exit(1)

	env.ParseConfig('pkg-config --cflags --libs ' + libgnutls_pc)

	# OpenSSL
	libopenssl_pc = string.strip(os.popen("pkg-config --list-all | grep '^openssl[ ]' | cut -f 1 -d ' '").read())

	if len(libopenssl_pc) == 0:
		print "ERROR: OpenSSL development package is not installed\n"
		Exit(1)

	env.ParseConfig('pkg-config --cflags --libs ' + libopenssl_pc)

env.Append(CXXFLAGS = ['-pthread'])

# Charset conversion library

# -- iconv
if sys.platform == "mac" or sys.platform == "darwin":
	env.Append(LIBS = ['iconv', 'gcrypt'])

# -- ICU
env.Append(LIBS = ['icuuc', 'icudata', 'icui18n'])

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
config_hpp.write('#define VMIME_HAVE_SIZE_T 1\n')
config_hpp.write('\n')

config_hpp.write('// Charset conversion support\n')
config_hpp.write('#define VMIME_CHARSETCONV_LIB_IS_ICONV 1\n')
config_hpp.write('#define VMIME_CHARSETCONV_LIB_IS_ICU 0\n')
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
	config_hpp.write('#define VMIME_TLS_SUPPORT_LIB_IS_GNUTLS 1\n')
	config_hpp.write('#define VMIME_TLS_SUPPORT_LIB_IS_OPENSSL 0\n')
	config_hpp.write('#define VMIME_HAVE_GNUTLS_PRIORITY_FUNCS 1\n')
else:
	config_hpp.write('#define VMIME_HAVE_TLS_SUPPORT 0\n')

config_hpp.write('// -- Messaging support\n')
if env['with_messaging'] == 'yes':
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 1\n')

	config_hpp.write('// -- Messaging protocols\n')

	for proto in messaging_protocols:
		config_hpp.write('#define VMIME_HAVE_MESSAGING_PROTO_' + string.upper(proto) + ' 1\n')

	for p in libvmime_messaging_proto_sources:
		proto = p[0]
		if not proto in messaging_protocols:
			config_hpp.write('#define VMIME_HAVE_MESSAGING_PROTO_' + string.upper(proto) + ' 0\n')
else:
	config_hpp.write('#define VMIME_HAVE_MESSAGING_FEATURES 0\n')

config_hpp.write('// -- Platform-specific code\n')
for platform in platforms:
	config_hpp.write('#define VMIME_PLATFORM_IS_' + string.upper(platform) + ' 1\n')

for platform in libvmime_platforms_sources:
	if not platform in platforms:
		config_hpp.write('#define VMIME_PLATFORM_IS_' + string.upper(platform) + ' 0\n')

config_hpp.write("""
#define VMIME_HAVE_GETADDRINFO 1
#define VMIME_HAVE_GETNAMEINFO 1
#define VMIME_HAVE_PTHREAD 1
#define VMIME_HAVE_GETTID 0
#define VMIME_HAVE_SYSCALL 1
#define VMIME_HAVE_SYSCALL_GETTID 1
#define VMIME_HAVE_LOCALTIME_R 1
#define VMIME_HAVE_LOCALTIME_S 0
#define VMIME_HAVE_GMTIME_R 1
#define VMIME_HAVE_GMTIME_S 0
""")

config_hpp.write('\n')
config_hpp.write('// Miscellaneous flags\n')

if IsProtocolSupported(messaging_protocols, 'sendmail'):
	config_hpp.write('#define VMIME_SENDMAIL_PATH "' + env['sendmail_path'] + '"\n')

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

