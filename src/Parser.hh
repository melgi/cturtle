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

#ifndef N3_PARSER_H
#define N3_PARSER_H


#include <map>
#include <memory>
#include <stdexcept>
#include <FlexLexer.h>
#include "Uri.hh"
#include "Token.hh"
#include "Model.hh"
#include "BlankNodeIdGenerator.hh"

namespace turtle {

	
	class ParseException : public std::runtime_error {
		int m_line;
	public:
		explicit ParseException(const std::string &message = std::string(), int line = -1) : std::runtime_error(message), m_line(line) {}
		int line() const noexcept { return m_line; }
	};
	
	struct TripleSink {
		
		virtual void start() = 0;
		virtual void end() = 0;
		virtual void document(const std::string &source) = 0;
		virtual void prefix(const std::string &prefix, const std::string &ns) = 0;
		virtual void triple(const Resource &subject, const URIResource &property, const N3Node &object) = 0;
		virtual unsigned count() const = 0;
		
		virtual ~TripleSink() {}
	};
	
	class DefaultTripleSink : public TripleSink {
		unsigned m_count;
	public:
		DefaultTripleSink() : TripleSink(), m_count(0) {}
		virtual void start() {}
		virtual void end() {}
		virtual void document(const std::string &source) {}
		virtual void prefix(const std::string &prefix, const std::string &ns) {}
		virtual void triple(const Resource &subject, const URIResource &property, const N3Node &object) { ++m_count; }
		virtual unsigned count() const { return m_count; }
	};
	
	
	/*
	 * Grammar:
	 * 
	 * turtledoc        -> statementlist
	 * statementlist    -> EPSILON | statement statementlist
	 * statement        -> directive | triples POINT
	 * directive        -> prefixid | base | sparqlprefix | sparqlbase
	 * prefixid         -> PREFIX PNAME_NS IRIREF POINT
	 * base             -> BASE IRIREF POINT
	 * sparqlbase       -> SPARQLBASE IRIREF
	 * sparqlprefix     -> SPARQLPREFIX PNAME_NS IRIREF
	 * triples          -> subject propertylist | blanknodepropertylist propertylistopt
	 * propertylistopt  -> EPSILON | propertylist
	 * propertylist     -> property propertyrest
	 * propertyrest     -> EPSILON | SEMICOLON propertylist2
	 * propertylist2    -> propertyopt | propertyrest
	 * propertyopt      -> EPSILON | property
	 * property         -> verb objectlist
	 * objectlist       -> object objectrest
	 * objectrest       -> EPSILON | COMMA objectlist
	 * verb             -> iri | A
	 * subject          -> BLANK_NODE_LABEL | iri | collection 
	 * object           -> BLANK_NODE_LABEL | iri | collection |  blanknodepropertylist | literal
	 * literal          -> rdfliteral | numericliteral | booleanliteral
	 * blanknodepropertylist -> LBRACKET  propertylistopt RBRACKET
	 * collection       -> LPAREN  objectopt RPAREN
	 * objectopt        -> EPSILON | object objectopt
	 * numericliteral   -> INTEGER | DECIMAL | DOUBLE
	 * rdfliteral       -> string dtlang
	 * dtlang           -> EPSILON | LANGTAG | CARETCARET iri
	 * booleanliteral   -> TRUE | FALSE
	 * string           -> STRING_LITERAL_QUOTE | STRING_LITERAL_SINGLE_QUOTE | STRING_LITERAL_LONG_SINGLE_QUOTE | STRING_LITERAL_LONG_QUOTE
	 * iri              -> IRIREF | prefixedname
	 * prefixedname     -> PNAME_LN | PNAME_NS
	 * 
	 * see http://hackingoff.com/compilers/ll-1-parser-generator
	 */
	class Parser {
		
		static const std::string LOCAL_NAME_ESCAPE_CHARS;
		static const std::string INVALID_ESCAPES;
		
		::yyFlexLexer m_lexer;
		
		Uri m_base;
		TripleSink *m_sink;
		std::map<std::string, std::string> m_prefixMap;
		
		BlankNodeIdGenerator m_blanks;
		
		Token::Type m_lookAhead;
		std::string m_lexeme;
		
		Token::Type nextToken() { return m_lexer.yylex(); }
		
		void match(Token::Type token)
		{
			m_lexeme.assign(m_lexer.YYText(), m_lexer.YYLeng());
			if (m_lookAhead == token)
				m_lookAhead = nextToken();
			else
				throw ParseException("expected different symbol");
		}
		
		Uri resolve(const std::string &uri);
		std::string toUri(const std::string &pname) const;
		
		void turtledoc();
		void base();
		void prefixID();
		void sparqlBase();
		void sparqlPrefix();
		void triples();
		std::unique_ptr<Resource> subject();
		void propertylist(const Resource *subject);
		void property(const Resource *subject);
		std::string iri();
		void objectlist(const Resource *subject, const URIResource *property);
		std::unique_ptr<N3Node> object();
		std::unique_ptr<Literal> literal();
		std::unique_ptr<Literal> dtlang(const std::string &lexicalValue);
		std::unique_ptr<RDFList> collection();
		std::unique_ptr<BlankNode> blanknodepropertylist();
		void propertylistopt(const Resource *subject);
		
		static std::string unescape(std::size_t start, const std::string  &localName);
		static std::string extractUri(const std::string &uriLiteral);
		static std::string extractString(const std::string &stringLiteral);
		
	public:
		Parser(std::istream *in, const Uri &base, TripleSink *sink) : m_lexer(in), m_base(base), m_sink(sink), m_prefixMap(), m_blanks(), m_lookAhead(0), m_lexeme() {}
		
		void parse()
		{
			m_sink->document(static_cast<std::string>(m_base));
			m_lookAhead = nextToken();
			
			turtledoc();
		}

		int line() const { return m_lexer.lineno(); }
	};

}

#endif /* N3_PARSER_H */
