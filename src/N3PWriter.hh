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

#ifndef N3_N3PWRITER_HH
#define N3_N3PWRITER_HH

#include "Parser.hh"
#include <unordered_set>
#include <ostream>
#include <iostream>

namespace turtle {
	
	class N3PWriter;

	class N3PFormatter : public N3NodeVisitor {
		std::ostream &m_out;
		bool m_rdivDecimal; // output decimals as rdivs
	public:
		static const std::string SKOLEM_PREFIX;
		static const char HEX_CHAR[];
		
		explicit N3PFormatter(std::ostream &out, bool rdivDecimal) : N3NodeVisitor(), m_out(out), m_rdivDecimal(rdivDecimal) {}
		
		void visit(const URIResource &resource) { m_out << "'<"; outputUri(resource.uri()); m_out << ">'"; }
		void visit(const BlankNode &blankNode)  { m_out << "'<" << SKOLEM_PREFIX << blankNode.id() << ">'"; }
		void visit(const Literal &literal)
		{
			m_out << "literal('";
			output(literal.lexical());
			m_out << "',type('<";
			outputUri(literal.datatype());
			m_out << ">'))";
		}

		void visit(const BooleanLiteral &literal) { m_out << (literal.value() ? BooleanLiteral::VALUE_TRUE.lexical() : BooleanLiteral::VALUE_FALSE.lexical()); }
		void visit(const IntegerLiteral &literal) { m_out << literal.lexical(); }
		void visit(const DoubleLiteral &literal);
		void visit(const DecimalLiteral &literal);
		
		void visit(const StringLiteral &literal)
		{
			m_out << "literal('";
			output(literal.lexical());
			m_out << '\'';
			const std::string &lang = literal.language();
			if (!lang.empty())
				m_out << ",lang('" << lang << "')";
			else
				m_out << ",type('<" << StringLiteral::TYPE << ">')";
			
			m_out << ')';
		}

		void visit(const RDFList &list)
		{ 
			m_out << '[';
			if (!list.empty()) {
				auto i = list.begin();
				(*i)->visit(*this);
				++i;
				//m_count += 2;
				while (i != list.end()) {
					m_out << ',';
					(*i)->visit(*this);
					++i;
					//m_count += 2;
				}
			}
			m_out << ']';
		}
		
		void output(const std::string &s)
		{
			for (std::size_t i = 0, length = s.length(); i < length; i++) {
				char c = s[i];
				switch (c) {
					case '\n' : m_out << "\\\\n";    break;
					case '\r' : m_out << "\\\\r";    break;
					case '\t' : m_out << "\\\\t";    break;
					case '\f' : m_out << "\\\\f";    break;
					case '\b' : m_out << "\\\\b";    break; // backspace, "\u0008"
					case '"'  : m_out << "\\\\\"";   break;
					case '\'' : m_out << "\\'";      break;
					case '\\' : m_out << "\\\\\\\\"; break;
					default   :
						if (c >= 0 && c <= 0x1F)
							writeHex(c);
						else
							m_out << c;
				}
			}
		}

		void outputUri(const std::string &s)
		{
			if (s.find('\'') == std::string::npos) {
				m_out << s;
			} else {
				for (std::size_t i = 0, length = s.length(); i < length; i++) {
					char c = s[i];
					if (c == '\'')
						m_out << "\\'";
					else
						m_out << c;
				}
			}
		}
		
		void writeHex(char c)
		{
			int hi = (c & 0xF0) >> 4;
			int lo = (c & 0x0F);
			
			m_out << "\\u00" << HEX_CHAR[hi] << HEX_CHAR[lo];
		}
	};

	class N3PWriter : public TripleSink {
		
		std::ostream &m_out;
		N3PFormatter m_formatter;
		std::unordered_set<std::string> m_properties;
		unsigned m_count;
		
		void writePrologue();
		void writeEpilogue();
		void outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object);
		
	public:
		explicit N3PWriter(std::ostream &out, bool rdivDecimal = false) : TripleSink(), m_out(out), m_formatter(out, rdivDecimal), m_properties(), m_count(0) {}
		
		void document(const std::string &source)
		{
			m_out << "scope('<";
			m_formatter.outputUri(source);
			m_out << ">').\n";
		}
		
		void prefix(const std::string &prefix, const std::string &ns)
		{
			m_out << "pfx('";
			m_formatter.output(prefix);
			m_out << ":','<";
			m_formatter.outputUri(ns);
			m_out << ">').\n";
		}
		
		void start() { writePrologue(); }
		void end()   { writeEpilogue(); }
		void triple(const Resource &subject, const URIResource &property, const N3Node &object);
		
		unsigned count() const { return m_count; }
		
	};

}

#endif /* N3_N3PWRITER_HH */
