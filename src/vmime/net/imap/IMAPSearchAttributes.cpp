#include "vmime/net/imap/IMAPSearchAttributes.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPUtils.hpp"


namespace vmime {
namespace net {
namespace imap {

namespace helpers {


class keylessToken : public IMAPSearchToken {

public:

	keylessToken(const char* token)
		: IMAPSearchToken(token) {
	}

	void generate(std::ostringstream& out) const {

		out << m_token;
	}
};


template <class TYPE>
class typedSearchToken : public IMAPSearchToken {

public:

	typedSearchToken(const char* token, const TYPE& keyword)
		: IMAPSearchToken(token)
		, m_keyword(keyword) {

	}

	typedSearchToken(const char* token, const TYPE&& keyword)
		: IMAPSearchToken(token)
		, m_keyword(std::move(keyword)) {

	}

protected:

	TYPE m_keyword;
};


// Represents a string search token with the search string quoted
class stringToken : public typedSearchToken <string> {

public:

	stringToken(const char* token, const string& keyword)
		: typedSearchToken(token, keyword) {

	}

	void generate(std::ostringstream& out) const override {

		out << m_token << " \"" << m_keyword << "\"";
	};
};


class headerToken : public typedSearchToken <const char*> {

public:

	headerToken(const char* token, const char* header)
		: typedSearchToken(token, header) {

	}

	headerToken(const char* token, const char* header, const string& headerKeyword)
		: typedSearchToken(token, header)
		, m_headerKeyword(headerKeyword) {

	}

	void generate(std::ostringstream& out) const override {

		out << m_token << " " << m_keyword << " \"" << m_headerKeyword << "\"";
	};

protected:

	string m_headerKeyword;
};


class dateToken : public typedSearchToken <vmime::datetime> {

public:

	dateToken(const char* token, const vmime::datetime& date)
		: typedSearchToken(token, date) {

	}

	// RFC claims that we need to disregard time information
	void generate(std::ostringstream& out) const override {

		out << m_token << " " << IMAPUtils::searchDate(m_keyword);
	};
};


class numberToken : public typedSearchToken< int > {

public:

	numberToken(const char* token, uint32_t keyword)
		: typedSearchToken(token, keyword) {

	}

	void generate(std::ostringstream& out) const override {

		out << m_token << " " << m_keyword;
	};
};


class flagToken : public typedSearchToken <vmime::net::message::Flags> {

public:

	flagToken(const char* token, vmime::net::message::Flags keyword)
		: typedSearchToken(token, keyword) {

	}

	void generate(std::ostringstream& out) const override {

		out << m_token << " ";

		switch (m_keyword) {
			case vmime::net::message::Flags::FLAG_SEEN: out << "Seen"; break;
			case vmime::net::message::Flags::FLAG_REPLIED: out << "Answered"; break;
			case vmime::net::message::Flags::FLAG_MARKED: out << "Flagged"; break;
			case vmime::net::message::Flags::FLAG_DRAFT: out << "Draft"; break;
			case vmime::net::message::Flags::FLAG_DELETED: out << "Deleted"; break;
			default: throw exceptions::operation_not_supported();
		}
	};
};


class tokenMessageSetEnumerator : public messageSetEnumerator {

public:

	tokenMessageSetEnumerator()
		: m_first(true) {

		m_oss.imbue(std::locale::classic());
	}

	void enumerateNumberMessageRange(const vmime::net::numberMessageRange& range) {

		if (!m_first) {
			m_oss << ",";
		}

		if (range.getFirst() == range.getLast()) {
			m_oss << range.getFirst();
		} else if (range.getLast() == size_t(-1)) {
			m_oss << range.getFirst() << ":*";
		} else {
			m_oss << range.getFirst() << ":" << range.getLast();
		}

		m_first = false;
	}

	void enumerateUIDMessageRange(const vmime::net::UIDMessageRange& range) {

		if (!m_first) {
			m_oss << ",";
		}

		if (range.getFirst() == range.getLast()) {
			m_oss << range.getFirst();
		} else if (range.getLast() == size_t(-1)) {
			m_oss << range.getFirst() << ":*";
		} else {
			m_oss << range.getFirst() << ":" << range.getLast();
		}

		m_first = false;
	}

	const std::string str() const {

		return m_oss.str();
	}

private:

	std::ostringstream m_oss;
	bool m_first;
};


class messageSetToken : public typedSearchToken <vmime::net::messageSet> {

public:

	messageSetToken(const char *token, const vmime::net::messageSet& keyword)
		: typedSearchToken(token, keyword) {

	}

	messageSetToken(const char *token, const vmime::net::messageSet&& keyword)
		: typedSearchToken(token, std::move(keyword)) {

	}

	void generate(std::ostringstream& out) const override {

		if (*m_token) {
			out << m_token << " (";
		}
		else {
			out << "(";
		}
		tokenMessageSetEnumerator enu;
		m_keyword.enumerate(enu);
		out << enu.str();
		out << ")";
	}
};


// Contains a list of tokens which the server will interpret as AND
class tokenVectorToken : public typedSearchToken <std::vector <vmime::shared_ptr <const IMAPSearchToken>>> {

public:

	tokenVectorToken(const char* token, const std::vector <vmime::shared_ptr <const IMAPSearchToken>>& tokensAndKeywords)
		: typedSearchToken(token, tokensAndKeywords) {

		if (0 == m_keyword.size()) {
			throw exceptions::invalid_argument();
		}
	}

	tokenVectorToken(const char* token, const std::vector <vmime::shared_ptr <const IMAPSearchToken>>&& tokensAndKeywords)
		: typedSearchToken(token, tokensAndKeywords) {

		if (0 == m_keyword.size()) {
			throw exceptions::invalid_argument();
		}
	}

	void generate(std::ostringstream& out) const override {

		out << m_token;

		if (*m_token) {
			out << " (";
		} else {
			out << "(";
		}

		m_keyword[0]->generate(out);

		for (size_t i = 1; i < m_keyword.size(); i++) {
			out << " ";
			m_keyword[i]->generate(out);
		}

		out << ")";
	};
};


// A pair of tokens, used with OR
class tokenPairToken : public typedSearchToken <std::pair <vmime::shared_ptr <const IMAPSearchToken>, vmime::shared_ptr <const IMAPSearchToken>>> {

public:

	tokenPairToken(const char* token, const std::pair <vmime::shared_ptr <const IMAPSearchToken>, vmime::shared_ptr <const IMAPSearchToken>>& pair)
		: typedSearchToken(token, pair) {

	}

	void generate(std::ostringstream& out) const override {

		out << m_token << " ";
		m_keyword.first->generate(out);
		out << " ";
		m_keyword.second->generate(out);
	};
};


} // helpers


IMAPSearchTokenPtr IMAPSearchTokenFactory::AND(const std::vector <IMAPSearchTokenPtr> &&keywords) {

	return vmime::make_shared <helpers::tokenVectorToken>("", std::move(keywords));
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::ANSWERED() {

	return vmime::make_shared <helpers::keylessToken>("ANSWERED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::BCC(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("BCC", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::BEFORE(const datetime& keyword) {

	return vmime::make_shared <helpers::dateToken>("BEFORE", keyword);
}

IMAPSearchTokenPtr IMAPSearchTokenFactory::BODY(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("BODY", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::CC(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("CC", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::DELETED() {

	return vmime::make_shared <helpers::keylessToken>("DELETED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::DRAFT() {

	return vmime::make_shared <helpers::keylessToken>("DRAFT");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::FLAGGED() {

	return vmime::make_shared <helpers::keylessToken>("FLAGGED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::FROM(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("FROM", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::HEADER(const char* fieldName) {

	return vmime::make_shared <helpers::headerToken>("HEADER", fieldName);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::HEADER(const char* fieldName, const string& fieldContents) {

	return vmime::make_shared <helpers::headerToken>("HEADER", fieldName, fieldContents);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::KEYWORD(vmime::net::message::Flags flag) {

	return vmime::make_shared <helpers::flagToken>("KEYWORD", flag);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::LARGER(uint32_t size) {

	return vmime::make_shared <helpers::numberToken>("LARGER", size);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::MESSAGESET(const vmime::net::messageSet& set) {

	return vmime::make_shared <helpers::messageSetToken>("", set);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::MESSAGESET(const vmime::net::messageSet&& set) {

	return vmime::make_shared <helpers::messageSetToken>("", std::move(set));
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::NEW() {

	return vmime::make_shared <helpers::keylessToken>("NEW");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::NOT(const IMAPSearchTokenPtr& token) {

	return vmime::make_shared <helpers::tokenVectorToken>("NOT", std::vector <vmime::shared_ptr <const IMAPSearchToken>>({token}));
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::OLD() {

	return vmime::make_shared <helpers::keylessToken>("OLD");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::ON(const datetime& date) {

	return vmime::make_shared <helpers::dateToken>("ON", date);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::OR(const IMAPSearchTokenPtr& tokenA, const IMAPSearchTokenPtr& tokenB) {

	return vmime::make_shared <helpers::tokenPairToken>("OR", std::make_pair(tokenA, tokenB));
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::RECENT() {

	return vmime::make_shared <helpers::keylessToken>("RECENT");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SEEN() {

	return vmime::make_shared <helpers::keylessToken>("SEEN");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SENTBEFORE(const datetime& date) {

	return vmime::make_shared <helpers::dateToken>("SENTBEFORE", date);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SENTON(const datetime& date) {

	return vmime::make_shared <helpers::dateToken>("SENTON", date);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SENTSINCE(const datetime& date) {

	return vmime::make_shared <helpers::dateToken>("SENTSINCE", date);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SINCE(const datetime& date) {

	return vmime::make_shared <helpers::dateToken>("SINCE", date);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SMALLER(uint32_t size) {

	return vmime::make_shared <helpers::numberToken>("SMALLER", size);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::SUBJECT(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("SUBJECT", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::TEXT(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("TEXT", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::TO(const string& keyword) {

	return vmime::make_shared <helpers::stringToken>("TO", keyword);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UID(const vmime::net::messageSet& set) {

	return vmime::make_shared <helpers::messageSetToken>("UID", set);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UID(const vmime::net::messageSet&& set) {

	return vmime::make_shared <helpers::messageSetToken>("UID", std::move(set));
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNANSWERED() {

	return vmime::make_shared <helpers::keylessToken>("UNANSWERED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNDELETED() {

	return vmime::make_shared <helpers::keylessToken>("UNDELETED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNDRAFT() {

	return vmime::make_shared <helpers::keylessToken>("UNDRAFT");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNFLAGGED() {

	return vmime::make_shared <helpers::keylessToken>("UNFLAGGED");
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNKEYWORD(vmime::net::message::Flags flag) {

	return vmime::make_shared <helpers::flagToken>("UNKEYWORD", flag);
}


IMAPSearchTokenPtr IMAPSearchTokenFactory::UNSEEN() {

	return vmime::make_shared <helpers::keylessToken>("UNSEEN");
}


IMAPSearchAttributes::IMAPSearchAttributes(std::vector <vmime::shared_ptr <const IMAPSearchToken>>&& tokens)
	: m_andTokens(std::move(tokens)) {
}


void IMAPSearchAttributes::add(const vmime::shared_ptr <const IMAPSearchToken>& token) {

	m_andTokens.push_back(token);
}


std::vector <string> IMAPSearchAttributes::generate() const {

	std::vector< string > keys;

	for (auto& token : m_andTokens) {

		std::ostringstream key;
		key.imbue(std::locale::classic());

		token->generate(key);

		keys.push_back(key.str());
	}

	return keys;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP
