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

#ifndef N3_NTRIPLESWRITER_HH
#define N3_NTRIPLESWRITER_HH


#include <ostream>
#include <memory>
#include <string>

#include "Parser.hh"
#include "Model.hh"

#ifdef _WIN32
#	define CTURTLE_CRLF
#endif

namespace turtle {
	
	class NTripleFormatter : public N3NodeVisitor {
		
		std::streambuf *m_outbuf;
		
		void output(const Literal &literal)
		{
			m_outbuf->sputc('"');
			output(literal.lexical());
			m_outbuf->sputn("\"^^<", 4);
			
			const std::string datatype = literal.datatype();
			m_outbuf->sputn(datatype.c_str(), datatype.length());
			m_outbuf->sputc('>');
		}
		
		void output(const std::string &s)
		{
			for (auto i = s.cbegin(); i != s.cend(); ++i) {
				switch (*i) {
					case '\n' : m_outbuf->sputc('\\'); m_outbuf->sputc('n');  break;
					case '\r' : m_outbuf->sputc('\\'); m_outbuf->sputc('r');  break;
					case '\t' : m_outbuf->sputc('\\'); m_outbuf->sputc('t');  break;
					case '\f' : m_outbuf->sputc('\\'); m_outbuf->sputc('f');  break;
					case '\b' : m_outbuf->sputc('\\'); m_outbuf->sputc('b');  break; // backspace, "\u0008"
					case '"'  : m_outbuf->sputc('\\'); m_outbuf->sputc('"');  break;
					case '\\' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); break;
					default   : m_outbuf->sputc(*i);
				}
			}
		}
		
	public:
		
		explicit NTripleFormatter(std::ostream &out) : N3NodeVisitor(), m_outbuf(out.rdbuf())
		{
			// nop
		}
		
		void visit(const URIResource &resource)
		{
			const std::string &uri = resource.uri();
			
			m_outbuf->sputc('<');
			m_outbuf->sputn(uri.c_str(), uri.length());
			m_outbuf->sputc('>');
		}
		
		void visit(const BlankNode &blankNode)
		{
			const std::string &id = blankNode.id();
			
			m_outbuf->sputn("_:b", 3);
			m_outbuf->sputn(id.c_str(), id.length());
		}
		
		void visit(const Literal &literal)        { output(literal); }
		
		void visit(const BooleanLiteral &literal) { output(literal); }
		void visit(const IntegerLiteral &literal) { output(literal); }
		void visit(const DoubleLiteral &literal)  { output(literal); }
		void visit(const DecimalLiteral &literal) { output(literal); }
		
		void visit(const StringLiteral &literal)
		{
			m_outbuf->sputc('"');
			output(literal.lexical());
			m_outbuf->sputc('"');
			
			const std::string &lang = literal.language();
			if (!lang.empty()) {
				m_outbuf->sputc('@');
				m_outbuf->sputn(lang.c_str(), lang.length());
			}
		}

		void visit(const RDFList &list)
		{ 
			// nop
		}
	};


	class NTriplesWriter : public TripleSink {
		
		std::streambuf *m_outbuf;
		NTripleFormatter m_formatter;
		BlankNodeIdGenerator m_idgen;
		unsigned m_count;
		
		inline void rawTriple(const Resource &subject, const URIResource &property, const N3Node &object);
		
		std::unique_ptr<BlankNode> triples(const RDFList &list);
		
	public:
		explicit NTriplesWriter(std::ostream &out) : TripleSink(), m_outbuf(out.rdbuf()), m_formatter(out), m_idgen(), m_count(0)
		{
			// nop
		}
		
		void start()
		{
			// nop
		}
		
		void end()
		{
			m_outbuf->pubsync(); // flush
		}
		
		void document(const std::string &source)
		{
			// nop
		}
		
		void prefix(const std::string &prefix, const std::string &ns)
		{
			// nop
		}
		
		void triple(const Resource &subject, const URIResource &property, const N3Node &object);
		
		unsigned count() const { return m_count; }
		
	};

}

#endif /* N3_NTRIPLESWRITER_HH */
