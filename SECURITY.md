# Security Policy

## Supported Versions

Only the `master` branch is supported. Security fixes are made there and
ship with the next release; they are not backported to older releases.

## Reporting a Vulnerability

Please do not open a public issue or pull request for a security problem.
Send an email to Vincent Richard <vincent@vmime.org> instead.

A good report includes:

* the affected part of VMime (parser, IMAP/POP3/SMTP client, TLS...) and the
  commit you tested;
* how to reproduce it: a sample message, a server transcript or a small
  program;
* what you think the impact is.

You should get an answer within a few days. Once the problem is confirmed,
the fix is usually made in a public pull request. If you think it should stay
private until the fix is ready, just say so in your report.

Unless you prefer to stay anonymous, you will be credited with a
`Reported-by:` line in the commit that fixes the problem. We do not request
CVE identifiers or publish advisories by default; if you need one, tell us.

## Scope

VMime handles untrusted data: messages from any sender and responses from any
mail server. Examples of what we consider a security issue:

* memory errors (out-of-bounds read or write, use-after-free...) or crashes
  when parsing a message, a header or a server response;
* infinite loops or excessive memory use triggered by crafted input;
* TLS certificate verification accepting a certificate it should reject;
* injection: data passed by the application producing extra headers in a
  generated message, or extra IMAP/POP3/SMTP commands.

Not in scope:

* bugs in the libraries VMime uses (GnuTLS, OpenSSL, GNU SASL, ICU, iconv):
  please report them to these projects;
* the sample programs in `examples/`;
* problems that require the application to misuse the API, for example by
  installing a certificate verifier that accepts every certificate.
