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
#include <iostream>

#include "Parser.hh"
#include "Model.hh"

namespace turtle {
	
	class NTripleFormatter : public N3NodeVisitor {
		std::ostream &m_out;
		
		void output(const Literal &literal)
		{
			m_out << '"';
			output(literal.lexical());
			m_out << "\"^^<" << literal.datatype() << '>';
		}
		
		void output(const std::string &s)
		{
			for (std::size_t i = 0, length = s.length(); i < length; i++) {
				char c = s[i];
				
				switch (c) {
					case '\n' : m_out << "\\n";  break;
					case '\r' : m_out << "\\r";  break;
					case '\t' : m_out << "\\t";  break;
					case '\f' : m_out << "\\f";  break;
					case '\b' : m_out << "\\b";  break; // backspace, "\u0008"
					case '"'  : m_out << "\\\""; break;
					case '\\' : m_out << "\\\\"; break;
					default   : m_out << c;
				}
			}
		}
		
	public:
		explicit NTripleFormatter(std::ostream &out) : N3NodeVisitor(), m_out(out) {}
		
		void visit(const URIResource &resource) { m_out << '<' << resource.uri() << '>'; }
		void visit(const BlankNode &blankNode) { m_out << "_:b" << blankNode.id(); }
		void visit(const Literal &literal) { output(literal); }

		void visit(const BooleanLiteral &literal) { output(literal); }
		void visit(const IntegerLiteral &literal) { output(literal); }
		void visit(const DoubleLiteral &literal) { output(literal); }
		void visit(const DecimalLiteral &literal) { output(literal); }
		
		void visit(const StringLiteral &literal)
		{
			m_out << '"';
			output(literal.lexical());
			m_out << '"';
			if (!literal.language().empty())
				m_out << '@' << literal.language();
		}

		void visit(const RDFList &list)
		{ 
			//m_out << '(';
			//for (auto i = list.begin(); i != list.end(); ++i) {
			//	m_out  << **i << ' ';
			//}
			//m_out << ')';
		}
	};


	class NTriplesWriter : public TripleSink {
		std::ostream &m_out;
		NTripleFormatter m_formatter;
		BlankNodeIdGenerator m_idgen;
		unsigned m_count;
		
		void rawTriple(const Resource &subject, const URIResource &property, const N3Node &object);
		
		std::unique_ptr<BlankNode> triples(const RDFList &list);
		
	public:
		NTriplesWriter(std::ostream &out) : TripleSink(), m_out(out), m_formatter(out), m_idgen(), m_count(0) {}
		
		void start()
		{
			// nop
		}
		
		void end()
		{
			// nop
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
