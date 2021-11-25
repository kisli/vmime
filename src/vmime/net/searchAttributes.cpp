#include "vmime/net/searchAttributes.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES

namespace vmime {
namespace net {

namespace helpers {

class keylessToken : public searchToken {

public:
	keylessToken(const char* token)
		: searchToken(token) {
	}

	void generate(std::ostringstream& out) const {
		out << m_token;
	}
};

template < class TYPE >
class typedSearchToken : public searchToken {

public:
	typedSearchToken(const char* token, const TYPE& keyword)
		: searchToken(token)
		, m_keyword(keyword) {
	}

	typedSearchToken(const char* token, const TYPE&& keyword)
		: searchToken(token)
		, m_keyword(std::move(keyword)) {
	}

    
protected:
	TYPE   m_keyword;
};

// Represents a string search token with the search string quoted
class stringToken : public typedSearchToken< string > {

public:
	stringToken(const char* token, const string& keyword)
		: typedSearchToken(token, keyword) {
	}

	void generate(std::ostringstream& out) const override {
		out << m_token << " \"" << m_keyword << "\"";
	};
};

class headerToken : public typedSearchToken< const char* > {

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

class dateToken : public typedSearchToken< vmime::datetime > {

public:
	dateToken(const char* token, const vmime::datetime& date)
		: typedSearchToken(token, date) {
	}

	// RFC claims that we need to disregard time information
	void generate(std::ostringstream& out) const override {
		out << m_token << " " << m_keyword.getDate();
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

class flagToken : public typedSearchToken< vmime::net::message::Flags > {

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


class messageSetToken : public typedSearchToken< vmime::net::messageSet > {

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
class tokenVectorToken : public typedSearchToken< std::vector< vmime::shared_ptr< const searchToken > > > {

public:
	tokenVectorToken(const char* token, const std::vector< vmime::shared_ptr< const searchToken > >& tokensAndKeywords)
		: typedSearchToken(token, tokensAndKeywords) {

		if (0 == m_keyword.size()) {
			throw exceptions::invalid_argument();
		}
	}

	tokenVectorToken(const char* token, const std::vector< vmime::shared_ptr< const searchToken > >&& tokensAndKeywords)
		: typedSearchToken(token, tokensAndKeywords) {

		if (0 == m_keyword.size()) {
			throw exceptions::invalid_argument();
		}
	}

	void generate(std::ostringstream& out) const override {
		out << m_token;
		if (*m_token)
			out << " (";
		else
			out << "(";

		m_keyword[0]->generate(out);
		for (size_t i = 1; i < m_keyword.size(); i++) {
			out << " ";
			m_keyword[i]->generate(out);
		}

		out << ")";
	};
};

// A pair of tokens, used with OR
class tokenPairToken : public typedSearchToken< std::pair< vmime::shared_ptr< const searchToken >, vmime::shared_ptr< const searchToken > > > {

public:
	tokenPairToken(const char* token, const std::pair< vmime::shared_ptr< const searchToken >, vmime::shared_ptr< const searchToken > >& pair)
		: typedSearchToken(token, pair) {
	}

	void generate(std::ostringstream& out) const override {
		out << m_token << " ";
		m_keyword.first->generate(out);
		out << " ";
		m_keyword.second->generate(out);
	};
};

} // namespace helpers

searchTokenPtr searchTokenFactory::AND(const std::vector< searchTokenPtr >&&keywords) {
	return vmime::make_shared<helpers::tokenVectorToken>("", std::move(keywords));
}

searchTokenPtr searchTokenFactory::ANSWERED() {
	return vmime::make_shared<helpers::keylessToken>("ANSWERED");
}

searchTokenPtr searchTokenFactory::BCC(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("BCC", keyword);
}

searchTokenPtr searchTokenFactory::BEFORE(const datetime& keyword) {
	return vmime::make_shared<helpers::dateToken>("BEFORE", keyword);
}

searchTokenPtr searchTokenFactory::BODY(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("BODY", keyword);
}

searchTokenPtr searchTokenFactory::CC(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("CC", keyword);
}

searchTokenPtr searchTokenFactory::DELETED() {
	return vmime::make_shared<helpers::keylessToken>("DELETED");
}

searchTokenPtr searchTokenFactory::DRAFT() {
	return vmime::make_shared<helpers::keylessToken>("DRAFT");
}

searchTokenPtr searchTokenFactory::FLAGGED() {
	return vmime::make_shared<helpers::keylessToken>("FLAGGED");
}

searchTokenPtr searchTokenFactory::FROM(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("FROM", keyword);
}

searchTokenPtr searchTokenFactory::HEADER(const char* fieldName) {
	return vmime::make_shared<helpers::headerToken>("HEADER", fieldName);
}

searchTokenPtr searchTokenFactory::HEADER(const char* fieldName, const string& fieldContents) {
	return vmime::make_shared<helpers::headerToken>("HEADER", fieldName, fieldContents);
}

searchTokenPtr searchTokenFactory::KEYWORD(vmime::net::message::Flags flag) {
	return vmime::make_shared<helpers::flagToken>("KEYWORD", flag);
}

searchTokenPtr searchTokenFactory::LARGER(uint32_t size) {
	return vmime::make_shared<helpers::numberToken>("LARGER", size);
}

searchTokenPtr searchTokenFactory::MESSAGESET(const vmime::net::messageSet& set) {
	return vmime::make_shared<helpers::messageSetToken>("", set);
}

searchTokenPtr searchTokenFactory::MESSAGESET(const vmime::net::messageSet&& set) {
	return vmime::make_shared<helpers::messageSetToken>("", std::move(set));
}

searchTokenPtr searchTokenFactory::NEW() {
	return vmime::make_shared<helpers::keylessToken>("NEW");
}

searchTokenPtr searchTokenFactory::NOT(const searchTokenPtr& token) {
	return vmime::make_shared<helpers::tokenVectorToken>("NOT", std::vector< vmime::shared_ptr< const searchToken > >({token}));
}

searchTokenPtr searchTokenFactory::OLD() {
	return vmime::make_shared<helpers::keylessToken>("OLD");
}

searchTokenPtr searchTokenFactory::ON(const datetime& date) {
	return vmime::make_shared<helpers::dateToken>("ON", date);
}

searchTokenPtr searchTokenFactory::OR(const searchTokenPtr& tokenA, const searchTokenPtr& tokenB) {
	return vmime::make_shared<helpers::tokenPairToken>("OR", std::make_pair(tokenA, tokenB));
}

searchTokenPtr searchTokenFactory::RECENT() {
	return vmime::make_shared<helpers::keylessToken>("RECENT");
}

searchTokenPtr searchTokenFactory::SEEN() {
	return vmime::make_shared<helpers::keylessToken>("SEEN");
}

searchTokenPtr searchTokenFactory::SENTBEFORE(const datetime& date) {
	return vmime::make_shared<helpers::dateToken>("SENTBEFORE", date);
}

searchTokenPtr searchTokenFactory::SENTON(const datetime& date) {
	return vmime::make_shared<helpers::dateToken>("SENTON", date);
}

searchTokenPtr searchTokenFactory::SENTSINCE(const datetime& date) {
	return vmime::make_shared<helpers::dateToken>("SENTSINCE", date);
}

searchTokenPtr searchTokenFactory::SINCE(const datetime& date) {
	return vmime::make_shared<helpers::dateToken>("SINCE", date);
}

searchTokenPtr searchTokenFactory::SMALLER(uint32_t size) {
	return vmime::make_shared<helpers::numberToken>("SMALLER", size);
}

searchTokenPtr searchTokenFactory::SUBJECT(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("SUBJECT", keyword);
}

searchTokenPtr searchTokenFactory::TEXT(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("TEXT", keyword);
}

searchTokenPtr searchTokenFactory::TO(const string& keyword) {
	return vmime::make_shared<helpers::stringToken>("TO", keyword);
}

searchTokenPtr searchTokenFactory::UID(const vmime::net::messageSet& set) {
	return vmime::make_shared<helpers::messageSetToken>("UID", set);
}

searchTokenPtr searchTokenFactory::UID(const vmime::net::messageSet&& set) {
	return vmime::make_shared<helpers::messageSetToken>("UID", std::move(set));
}

searchTokenPtr searchTokenFactory::UNANSWERED() {
	return vmime::make_shared<helpers::keylessToken>("UNANSWERED");
}

searchTokenPtr searchTokenFactory::UNDELETED() {
	return vmime::make_shared<helpers::keylessToken>("UNDELETED");
}

searchTokenPtr searchTokenFactory::UNDRAFT() {
	return vmime::make_shared<helpers::keylessToken>("UNDRAFT");
}

searchTokenPtr searchTokenFactory::UNFLAGGED() {
	return vmime::make_shared<helpers::keylessToken>("UNFLAGGED");
}

searchTokenPtr searchTokenFactory::UNKEYWORD(vmime::net::message::Flags flag) {
	return vmime::make_shared<helpers::flagToken>("UNKEYWORD", flag);
}

searchTokenPtr searchTokenFactory::UNSEEN() {
	return vmime::make_shared<helpers::keylessToken>("UNSEEN");
}

searchAttributes::searchAttributes(std::vector< vmime::shared_ptr< const searchToken > >&& tokens)
	: m_andTokens(std::move(tokens)) {
}

void searchAttributes::add(const vmime::shared_ptr< const searchToken >& token) {
	m_andTokens.push_back(token);
}

std::vector< string > searchAttributes::generate() const {

	std::vector< string > keys;

	for (auto& token : m_andTokens) {
		std::ostringstream key;
		key.imbue(std::locale::classic());

		token->generate(key);

		keys.push_back(key.str());
	}

	return keys;
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
