//
// Copyright 2016 Giovanni Mels
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <utility>

#include "Parser.hh"
#include "Utf8.hh"
#include "Utf16.hh"
#include "Model.hh"

namespace turtle {

	const std::string Parser::LOCAL_NAME_ESCAPE_CHARS("_~.-!$&\'()*+,;=/?#@%");
	
	// We do not check if uris are valid, this is used when translating \uxxxx escapes to chars
	const std::string Parser::INVALID_ESCAPES("<>\"{}|^`\\");

	inline Uri Parser::resolve(const std::string &uri)
	{
		Uri u(uri);
		if (u.absolute())
			return u;
		
		return m_base.resolve(u);
	}

	inline Uri Parser::resolve(std::string &&uri)
	{
		Uri u(std::move(uri));
		if (u.absolute())
			return u;
		
		return m_base.resolve(u);
	}

	std::string Parser::toUri(const std::string &pname) const
	{
		std::size_t p = pname.find(':');
		if (p == std::string::npos)
			throw ParseException();
		
		std::string prefix = pname.substr(0, p);
		
		auto i = m_prefixMap.find(prefix);
		if (i == m_prefixMap.end())
			throw ParseException("unknown prefix: " + prefix, line());
		
		return i->second + unescape(p + 1, pname);
		// checking for valid uris is redundant here, i->second is a valid uri, concatenating a fragment or path cannot give a invalid uri.
		//return static_cast<std::string>(Uri(i->second + unescape(p + 1, pname)));
	}

	void Parser::turtledoc()
	{
		try {
			while (m_lookAhead != Token::Eof) {
				if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '[' || m_lookAhead == '(') {
					triples();
					match('.');
				} else if (m_lookAhead == Token::Prefix) {
					prefixID();
				} else if (m_lookAhead == Token::Base) {
					base();
				} else if (m_lookAhead == Token::SparqlPrefix) {
					sparqlPrefix();
				} else if (m_lookAhead == Token::SparqlBase) {
					sparqlBase();
				} else
					throw ParseException("expected base, prefix or triple", line());
			}
		} catch (UriSyntaxException &e) {
			throw ParseException(e.what(), line());
		}
	}

	void Parser::base()
	{
		match(Token::Base);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		match('.');
		
		m_base = resolve(std::move(u));
	}

	void Parser::prefixID()
	{
		match(Token::Prefix);
		match(Token::PNameNS);
		std::string prefix = m_lexeme.substr(0, m_lexeme.length() - 1);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		match('.');
		
		std::string ns = static_cast<std::string>(resolve(std::move(u)));
		m_sink->prefix(prefix, ns);
		m_prefixMap[prefix] = ns;
	}

	void Parser::sparqlBase()
	{
		match(Token::SparqlBase);
		match(Token::IriRef);
		
		std::string u = extractUri(m_lexeme);
		
		m_base = resolve(std::move(u));
	}

	void Parser::sparqlPrefix()
	{
		match(Token::SparqlPrefix);
		match(Token::PNameNS);
		std::string prefix = m_lexeme.substr(0, m_lexeme.length() - 1);
		match(Token::IriRef);
		std::string u = extractUri(m_lexeme);
		
		std::string ns = static_cast<std::string>(resolve(std::move(u)));
		m_sink->prefix(prefix, ns);
		m_prefixMap[prefix] = ns;
	}

	void Parser::triples()
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '(') {
			std::unique_ptr<Resource> s = subject(); propertylist(s.get());
		} else if (m_lookAhead == '[') {
			std::unique_ptr<BlankNode> b = blanknodepropertylist(); propertylistopt(b.get());
		} else
			throw ParseException("expected blank node, uri or list as subject", line());
	}

	std::unique_ptr<Resource> Parser::subject()
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			return std::unique_ptr<Resource>(new URIResource(iri()));
		} else if (m_lookAhead == Token::BlankNodeLabel) {
			match(Token::BlankNodeLabel);
			return std::unique_ptr<Resource>(new BlankNode(m_blanks.generate(m_lexeme.substr(2))));
		} else if (m_lookAhead == '(') { 
			return collection();
		} else
			throw ParseException("expected blank node, uri or list as subject", line());
	}

	void Parser::propertylist(const Resource *subject)
	{
		if (m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			property(subject);
			while (m_lookAhead == ';') {
				match(';');
				if (m_lookAhead == 'a' || m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
					property(subject);
				}
			}
		} else
			throw ParseException("expected property", line());
	}

	void Parser::property(const Resource *subject)
	{
		if (m_lookAhead == 'a') {
			match('a');
			objectlist(subject, &RDF::type);
		} else if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			URIResource property(iri());
			objectlist(subject, &property);
		} else
			throw ParseException("expected 'a' or uri as property", line());
	}

	std::string Parser::iri()
	{
		if (m_lookAhead == Token::IriRef) {
			match(Token::IriRef);
			return static_cast<std::string>(resolve(extractUri(m_lexeme)));
		} else if (m_lookAhead == Token::PNameLN) {
			match(Token::PNameLN);
			return toUri(m_lexeme);
		} else if (m_lookAhead == Token::PNameNS) {
			match(Token::PNameNS);
			return toUri(m_lexeme);
		} else
			throw ParseException("expected IRI ref or prefixed name", line());
	}

	void Parser::objectlist(const Resource *subject, const URIResource *property)
	{
		if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
			std::unique_ptr<N3Node> obj = object();
			m_sink->triple(*subject, *property, *obj);
			while (m_lookAhead == ',') {
				match(',');
				if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::BlankNodeLabel || m_lookAhead == Token::PNameNS || m_lookAhead == '[' || m_lookAhead == '(' || m_lookAhead == Token::StringLiteralQuote || m_lookAhead == Token::StringLiteralSingleQuote || m_lookAhead == Token::StringLiteralLongSingleQuote || m_lookAhead == Token::StringLiteralLongQuote || m_lookAhead == Token::True || m_lookAhead == Token::False || m_lookAhead == Token::Integer || m_lookAhead == Token::Decimal || m_lookAhead == Token::Double) {
					std::unique_ptr<N3Node> obj = object();
					m_sink->triple(*subject, *property, *obj);
				} else
					throw ParseException("expected object after ','", line());
			}
		} else
			throw ParseException("expected object", line());
	}


	std::unique_ptr<N3Node> Parser::object()
	{
		if (m_lookAhead == Token::BlankNodeLabel) {
			match(Token::BlankNodeLabel);
			return std::unique_ptr<N3Node>(new BlankNode(m_blanks.generate(m_lexeme.substr(2))));
		} else if (m_lookAhead == Token::PNameLN || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameNS) {
			return std::unique_ptr<N3Node>(new URIResource(iri()));
		} else if (m_lookAhead == Token::StringLiteralQuote) {
			match(Token::StringLiteralQuote);
			std::string lexicalValue = extractString(m_lexeme);
			return dtlang(lexicalValue);
		} else if (m_lookAhead == Token::StringLiteralLongQuote) {
			match(Token::StringLiteralLongQuote);
			std::string lexicalValue = extractString(m_lexeme);
			return dtlang(lexicalValue);
		} else if (m_lookAhead == Token::Integer) {
			match(Token::Integer);
			return std::unique_ptr<Literal>(new IntegerLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Decimal) {
			match(Token::Decimal);
			return std::unique_ptr<Literal>(new DecimalLiteral(m_lexeme));
		} else if (m_lookAhead == Token::Double) {
			match(Token::Double);
			return std::unique_ptr<Literal>(new DoubleLiteral(m_lexeme));
		} else if (m_lookAhead == Token::True) {
			match(Token::True);
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == Token::False) {
			match(Token::False);
			return std::unique_ptr<Literal>(new BooleanLiteral(m_lexeme));
		} else if (m_lookAhead == '[') {
			return blanknodepropertylist();
		} else if (m_lookAhead == '(') {
			return collection();
		} else if (m_lookAhead == Token::StringLiteralSingleQuote) {
			match(Token::StringLiteralSingleQuote);
			std::string lexicalValue = extractString(m_lexeme);
			return dtlang(lexicalValue);
		} else if (m_lookAhead == Token::StringLiteralLongSingleQuote) {
			match(Token::StringLiteralLongSingleQuote);
			std::string lexicalValue = extractString(m_lexeme);
			return dtlang(lexicalValue);
		} else {
			throw ParseException("expected blank node, iri, literal or list", line());
		}
	}
	
	std::unique_ptr<Literal> Parser::dtlang(const std::string &lexicalValue)
	{
		if (m_lookAhead == Token::LangTag) {
			match(Token::LangTag);
			return std::unique_ptr<Literal>(new StringLiteral(lexicalValue, m_lexeme.substr(1)));
		} else if (m_lookAhead == Token::CaretCaret) {
			match(Token::CaretCaret);
			const std::string type = iri();
			if (type == IntegerLiteral::TYPE)
				return std::unique_ptr<Literal>(new IntegerLiteral(lexicalValue)); //TODO valid check
			if (type == DecimalLiteral::TYPE)
				return std::unique_ptr<Literal>(new DecimalLiteral(lexicalValue));
			if (type == BooleanLiteral::TYPE)
				return std::unique_ptr<Literal>(new BooleanLiteral(lexicalValue));
			if (type == DoubleLiteral::TYPE)
				return std::unique_ptr<Literal>(new DoubleLiteral(lexicalValue));
			if (type == StringLiteral::TYPE)
				return std::unique_ptr<Literal>(new StringLiteral(lexicalValue));
			
			return std::unique_ptr<Literal>(new OtherLiteral(lexicalValue, type));
		}
		
		return std::unique_ptr<Literal>(new StringLiteral(lexicalValue));
	}

	std::unique_ptr<RDFList> Parser::collection()
	{
		std::unique_ptr<RDFList> list(new RDFList());
		match('(');
		while (m_lookAhead != ')') {
			list->add(object().release());
		}	
		match(')');
		
		return list;
	}


	std::unique_ptr<BlankNode> Parser::blanknodepropertylist()
	{
		std::unique_ptr<BlankNode> b(new BlankNode(m_blanks.generate()));
		match('['); propertylistopt(b.get()); match(']');
		return b;
	}

	void Parser::propertylistopt(const Resource *subject)
	{
		if (m_lookAhead == 'a' || m_lookAhead == Token::IriRef || m_lookAhead == Token::PNameLN || m_lookAhead == Token::PNameNS)
			propertylist(subject);
	}
	
	
	std::string Parser::unescape(std::size_t start, const std::string &localName)
	{
		std::size_t end = localName.length();
		
		if (localName.find('\\', start) == std::string::npos)
			return localName.substr(start, end - start);
		
		std::string buf;
		
		buf.reserve(end - start);
		
		for (std::size_t i = start; i < end; i++) {
			char c = localName[i];
			
			if (c == '\\') {
				c = localName[++i];
				if (LOCAL_NAME_ESCAPE_CHARS.find(c) != std::string::npos)
					buf.push_back(c);
				else
					throw ParseException("\"" + localName + "\" contains illegal escape \"\\" + c + "\"");
			} else {
				buf.push_back(c);
			}
		}
		
		return buf;
	}
	
	
	
	std::string Parser::extractUri(const std::string &uriLiteral)
	{
		if (uriLiteral.find('\\', 1) == std::string::npos)
			return uriLiteral.substr(1, uriLiteral.length() - 2);
		
		std::string buf;
		buf.reserve(uriLiteral.length());
		auto inserter = std::back_inserter(buf);
		
		std::uint16_t highSurrogate = 0;
		
		for (auto i = uriLiteral.begin() + 1; i < uriLiteral.end() - 1;) {
			char c = *i;
			
			if (c == '\\') {
				++i;
				c = *i;
				switch (c) {
					case 'u' : {
						auto begin = ++i; i += 4; 
						std::string value = std::string(begin, i);
						int v = std::stoi(value, nullptr, 16);
							
						if (utf16::isHighSurrogate(v)) {
							if (highSurrogate)
								throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
							
							highSurrogate = v;
						} else {
						
							if (utf16::isLowSurrogate(v)) {
								if (!highSurrogate)
									throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
								
								v = utf16::toChar(highSurrogate, v);
								utf8::encode(v, inserter);
								highSurrogate = 0;
							} else {
								if (v <= 0x20 || (v < 128 && INVALID_ESCAPES.find(std::string::traits_type::to_char_type(v)) != std::string::npos))
									throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\u" + value + "\"");
								
								utf8::encode(v, inserter);
							}
						}
						
						break;
					}
					case 'U' : {
						if (highSurrogate)
							throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
						
						auto begin = ++i; i += 8;
						std::string value = std::string(begin, i);
						int v = std::stoi(value, nullptr, 16);
						if (v <= 0x20 || (v < 128 && INVALID_ESCAPES.find(std::string::traits_type::to_char_type(v)) != std::string::npos))
							throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\U" + value + "\"");
						utf8::encode(v, inserter);
						break;
					}
					default  :
						throw ParseException("\"" + uriLiteral + "\" contains illegal escape \"\\" + c + "\"");
				}
			} else {
				if (highSurrogate)
					throw ParseException("\"" + uriLiteral + "\" contains an unpaired surrogate");
				inserter = c; // this will actually append c to buf;
				++i;
			}
		}
		
		return buf;
	}


	std::string Parser::extractString(const std::string &stringLiteral)
	{
		// Because of the lexer produced stringLiteral, we can assume that the its value is "well formed":
		// enclosed in matched quotes, escapes are valid, indexes will never go outside the string bounds...
		std::size_t start;
		std::size_t end;
		
		if (stringLiteral.find("\"\"\"") == 0 || stringLiteral.find("'''") == 0) {
			start = 3;
			end   = stringLiteral.length() - 3;
		} else {
			start = 1;
			end   = stringLiteral.length() - 1;
		}
		
		if (stringLiteral.find('\\', start) == std::string::npos)
			return stringLiteral.substr(start, end - start);
		
		std::string buf;
		buf.reserve(end - start);
		
		std::uint16_t highSurrogate = 0;
		
		for (std::size_t i = start; i < end; i++) {
			char c = stringLiteral[i];
			
			if (c == '\\') {
				c = stringLiteral[++i];
				
				if (highSurrogate && c != 'u')
					throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
				
				switch (c) {
					case 'n' : buf.push_back('\n'); break;
					case 'r' : buf.push_back('\r'); break;
					case 't' : buf.push_back('\t'); break;
					case 'f' : buf.push_back('\f'); break;
					case 'b' : buf.push_back('\b'); break; // backspace, "\u0008"
					case '"' : buf.push_back('"');  break;
					case '\'': buf.push_back('\''); break;
					case '\\': buf.push_back('\\'); break;
					case 'u' : {
						std::size_t begin = ++i; i += 3; 
						std::string value = stringLiteral.substr(begin, 4);
						int v = std::stoi(value, nullptr, 16);
						
						if (utf16::isHighSurrogate(v)) {
							if (highSurrogate)
								throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
								
							highSurrogate = v;
						} else {
							if (utf16::isLowSurrogate(v)) {
								if (!highSurrogate)
									throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
									
								v = utf16::toChar(highSurrogate, v);
								utf8::encode(v, std::back_inserter(buf));
								highSurrogate = 0;
							} else {
								utf8::encode(v, std::back_inserter(buf));
							}
						}
						break;
					}
					case 'U' : {
						std::size_t begin = ++i; i += 7;
						std::string value = stringLiteral.substr(begin, 8);
						int v = std::stoi(value, nullptr, 16);
						utf8::encode(v, std::back_inserter(buf));
						break;
					}
					
					default  :
						throw ParseException(stringLiteral + " contains \"\\" + c + "\"");
				}
			} else {
				if (highSurrogate)
					throw ParseException("\"" + stringLiteral + "\" contains an unpaired surrogate");
					
				buf.push_back(c);
			}
		}
		
		return buf;
	}

}
