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

#include <unordered_set>
#include <ostream>
#include <iterator>

#include "Parser.hh"
#include "Utf8.hh"
#include "Utf16.hh"

#ifdef _WIN32
#	define CTURTLE_CRLF
#   ifndef CTURTLE_N3P_UTF8
#		define CTURTLE_N3P_CESU8
#   endif
#endif

namespace turtle {
	
	class N3PFormatter : public N3NodeVisitor {
		std::streambuf *m_outbuf;
		
		bool m_rdivDecimal; // output decimals as rdivs
		
	public:
		static const std::string SKOLEM_PREFIX;
		static const char HEX_CHAR[];
		
		N3PFormatter(std::ostream &out, bool rdivDecimal) : N3NodeVisitor(), m_outbuf(out.rdbuf()), m_rdivDecimal(rdivDecimal)
		{
			// nop
		}
		
		void visit(const URIResource &resource);
		void visit(const BlankNode &blankNode);
		void visit(const Literal &literal);
		void visit(const BooleanLiteral &literal);
		void visit(const IntegerLiteral &literal);
		void visit(const DoubleLiteral &literal);
		void visit(const DecimalLiteral &literal);
		void visit(const StringLiteral &literal);
		void visit(const RDFList &list);
			
		void output(const std::string &s)
		{
			for (auto i = s.cbegin(); i != s.cend(); ++i) {
				char c = *i;
				if (c >= 0 && c <= 0x1F) {
					switch (c) {
						case '\n' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('n');  break;
						case '\r' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('r');  break;
						case '\t' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('t');  break;
						case '\f' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('f');  break;
						case '\b' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('b');  break; // backspace, "\u0008"
						default   : writeHex(c);
					}
				} else {
					switch (c) {
						case '"'  : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('"');                         break;
						case '\'' : m_outbuf->sputc('\\'); m_outbuf->sputc('\'');                                               break;
						case '\\' : m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); m_outbuf->sputc('\\'); break;
						default   :
#ifdef CTURTLE_N3P_CESU8
							if ((c & 0xF8) == 0xF0) {
								i += ouputCesu8(i, s.cend()) - 1; 
							} else {
								m_outbuf->sputc(c);
							}
#else /* !CTURTLE_N3P_CESU8 */
							m_outbuf->sputc(c);		
#endif /* CTURTLE_N3P_CESU8 */
					}
				}
			}
		}
		
		void outputUri(const std::string &s)
		{
#ifdef CTURTLE_N3P_CESU8
			for (auto i = s.cbegin(); i != s.cend(); ++i) {
				char c = *i;
				if (c == '\'') {
					m_outbuf->sputc('\\');
					m_outbuf->sputc('\'');
				} else {
					if ((c & 0xF8) == 0xF0) {
						i += ouputCesu8(i, s.cend()) - 1;
					} else {
						m_outbuf->sputc(c);
					}
				}
			}
#else /* !CTURTLE_N3P_CESU8 */
			if (s.find('\'') == std::string::npos) {
				m_outbuf->sputn(s.c_str(), s.length());
			} else {
				for (auto i = s.cbegin(); i != s.cend(); ++i) {
					char c = *i;
					if (c == '\'') {
						m_outbuf->sputc('\\');
						m_outbuf->sputc('\'');
					} else {
						m_outbuf->sputc(c);
					}
				}
			}
#endif /* CTURTLE_N3P_CESU8 */
		}

	private:
	
#ifdef CTURTLE_N3P_CESU8

		template<typename Iterator> std::size_t ouputCesu8(const Iterator &i, const Iterator &end)
		{
			const char32_t REPLACEMENT_CHARACTER = U'\uFFFD';
			
			char32_t cp;
			
			utf8::State state;
			int r = utf8::decode(&cp, i, end, &state);
			
			if (r == -1) {
				cp = REPLACEMENT_CHARACTER;
				r  = 1;
			} else if (r == -2) {
				cp = REPLACEMENT_CHARACTER;
				r  = end - i;
			}
			
			utf16::encodeCESU8(cp, std::ostreambuf_iterator<std::streambuf::char_type>(m_outbuf));
			
			return r;
		}
		
#endif /* CTURTLE_N3P_CESU8 */
	
		void writeHex(char c)
		{
			int hi = (c & 0xF0) >> 4;
			int lo = (c & 0x0F);
			
			m_outbuf->sputn("\\u00", 4);
			
			m_outbuf->sputc(HEX_CHAR[hi]);
			m_outbuf->sputc(HEX_CHAR[lo]);
		}
	};

	class N3PWriter : public TripleSink {
		
		std::ostream &m_out;
		std::streambuf *m_outbuf;
		N3PFormatter m_formatter;
		std::unordered_set<std::string> m_properties;
		unsigned m_count;
		
		void writePrologue();
		void writeEpilogue();
		inline void outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object);
		void outputProperty(const std::string &uri);
		
		void endl()
		{
#ifdef CTURTLE_CRLF
			m_outbuf->sputc('\r');
#endif
			m_outbuf->sputc('\n');
		}
		
	public:
		explicit N3PWriter(std::ostream &out, bool rdivDecimal = false) : TripleSink(), m_out(out), m_outbuf(out.rdbuf()), m_formatter(out, rdivDecimal), m_properties(), m_count(0)
		{
			// nop
		}
		
		void document(const std::string &source)
		{
			m_out << "scope('<";
			m_formatter.outputUri(source);
			m_out << ">').";
			endl();
		}
		
		void prefix(const std::string &prefix, const std::string &ns)
		{
			m_out << "pfx('";
			m_formatter.output(prefix);
			m_out << ":','<";
			m_formatter.outputUri(ns);
			m_out << ">').";
			endl();
		}
		
		void start() { writePrologue(); }
		void end()   { writeEpilogue(); }
		void triple(const Resource &subject, const URIResource &property, const N3Node &object);
		
		unsigned count() const { return m_count; }
		
	};

}

#endif /* N3_N3PWRITER_HH */
