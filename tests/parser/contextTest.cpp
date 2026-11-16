//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "tests/testUtils.hpp"


VMIME_TEST_SUITE_BEGIN(contextTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParsingContextCopy)
		VMIME_TEST(testParsingContextAssignment)
		VMIME_TEST(testParsingContextSwitcher)
		VMIME_TEST(testParsingContextFeedbackNotCopied)
		VMIME_TEST(testGenerationContextAssignment)
		VMIME_TEST(testGenerationContextSwitcher)
	VMIME_TEST_LIST_END


	// Set all the options of a parsing context to non-default values
	static void setParsingOptions(vmime::parsingContext& ctx) {

		ctx.setInternationalizedEmailSupport(true);
		ctx.setHeaderParseErrorRecoveryMethod(vmime::headerParseRecoveryMethod::ASSUME_END_OF_HEADERS);
		ctx.setUseMyHostname(false);
		ctx.setMaxNestingDepth(5);
		ctx.setHeaderFallbackCharset(vmime::charset("iso-8859-1"));
	}

	static void assertParsingOptions(const vmime::parsingContext& ctx) {

		VASSERT_TRUE("intl", ctx.getInternationalizedEmailSupport());
		VASSERT_EQ(
			"recovery",
			vmime::headerParseRecoveryMethod::ASSUME_END_OF_HEADERS,
			ctx.getHeaderParseErrorRecoveryMethod()
		);
		VASSERT_FALSE("hostname", ctx.getUseMyHostname());
		VASSERT_EQ("depth", 5, ctx.getMaxNestingDepth());
		VASSERT_EQ("fallback", vmime::charset("iso-8859-1"), ctx.getHeaderFallbackCharset());
	}

	void testParsingContextCopy() {

		vmime::parsingContext ctx;
		setParsingOptions(ctx);

		vmime::parsingContext copy(ctx);

		assertParsingOptions(copy);
	}

	void testParsingContextAssignment() {

		vmime::parsingContext ctx;
		setParsingOptions(ctx);

		vmime::parsingContext copy;
		copy = ctx;

		assertParsingOptions(copy);
	}

	void testParsingContextSwitcher() {

		vmime::parsingContext ctx;
		setParsingOptions(ctx);

		{
			vmime::parsingContext::switcher <vmime::parsingContext> contextSwitcher(ctx);

			assertParsingOptions(vmime::parsingContext::getDefaultContext());
		}

		const vmime::parsingContext& defaultCtx = vmime::parsingContext::getDefaultContext();

		VASSERT_FALSE("restored intl", defaultCtx.getInternationalizedEmailSupport());
		VASSERT_EQ(
			"restored recovery",
			vmime::headerParseRecoveryMethod::SKIP_LINE,
			defaultCtx.getHeaderParseErrorRecoveryMethod()
		);
		VASSERT_TRUE("restored hostname", defaultCtx.getUseMyHostname());
		VASSERT_EQ("restored depth", 100, defaultCtx.getMaxNestingDepth());
		VASSERT_EQ("restored fallback", vmime::charset("utf-8"), defaultCtx.getHeaderFallbackCharset());
	}

	// The feedback from a previous parsing is not an option
	void testParsingContextFeedbackNotCopied() {

		vmime::parsingContext ctx;

		vmime::header hdr;
		hdr.parse(ctx, "Subject: test\r\nNot a header field\r\n\r\n");

		VASSERT_TRUE("recovery needed", ctx.getHeaderRecoveryNeeded());

		vmime::parsingContext copy(ctx);

		VASSERT_FALSE("copy", copy.getHeaderRecoveryNeeded());

		vmime::parsingContext copy2;
		copy2 = ctx;

		VASSERT_FALSE("assignment", copy2.getHeaderRecoveryNeeded());
	}

	void testGenerationContextAssignment() {

		vmime::generationContext ctx;
		ctx.setWrapMessageId(false);

		vmime::generationContext copy;
		copy = ctx;

		VASSERT_FALSE("wrap", copy.getWrapMessageId());
	}

	void testGenerationContextSwitcher() {

		vmime::generationContext ctx;
		ctx.setWrapMessageId(false);

		{
			vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

			VASSERT_FALSE("wrap", vmime::generationContext::getDefaultContext().getWrapMessageId());
		}

		VASSERT_TRUE("restored wrap", vmime::generationContext::getDefaultContext().getWrapMessageId());
	}

VMIME_TEST_SUITE_END
