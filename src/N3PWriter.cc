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

#include "N3PWriter.hh"


namespace turtle {


	void N3PWriter::writePrologue()
	{
		m_out << ":- style_check(-discontiguous)."; endl();
		m_out << ":- style_check(-singleton)."; endl();
		m_out << ":- multifile(exopred/3)."; endl();
		m_out << ":- multifile(implies/3)."; endl();
		m_out << ":- multifile(pfx/2)."; endl();
		m_out << ":- multifile(pred/1)."; endl();
		m_out << ":- multifile(prfstep/8)."; endl();
		m_out << ":- multifile(scope/1)."; endl();
		m_out << ":- multifile(scount/1)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#mu>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#pi>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#sigma>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#biconditional>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#conditional>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#reflexive>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#relabel>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#tactic>'/2)."; endl();
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#transaction>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#first>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#rest>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#implies>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#outputString>'/2)."; endl();
		m_out << ":- multifile('<http://www.w3.org/2002/07/owl#sameAs>'/2)."; endl();
		m_out << "flag('no-skolem', '" << N3PFormatter::SKOLEM_PREFIX << "')."; endl();
	}

	void N3PWriter::writeEpilogue()
	{
		m_out << "scount(" << m_count << ")."; endl();
		m_out << "end_of_file."; endl();
		
		m_out << std::flush;
	}

	void N3PWriter::triple(const Resource &subject, const URIResource &property, const N3Node &object)
	{
		const std::string &uri = property.uri();
		
		if (m_properties.insert(uri).second)
			outputProperty(uri);
		
		outputTriple(subject, property, object);
	}
	
	void N3PWriter::outputProperty(const std::string &uri)
	{
		if (uri.find('\'') == std::string::npos) {
			m_outbuf->sputn(":- dynamic('<", 13); 
			m_outbuf->sputn(uri.c_str(), uri.length());
			m_outbuf->sputn(">'/2).", 6);
			endl();
			m_outbuf->sputn(":- multifile('<", 15);
			m_outbuf->sputn(uri.c_str(), uri.length());
			m_outbuf->sputn(">'/2).", 6);
			endl();
			m_outbuf->sputn("pred('<", 7);
			m_outbuf->sputn(uri.c_str(), uri.length());
			m_outbuf->sputn(">').", 4);
			endl();
		} else {
			m_outbuf->sputn(":- dynamic('<", 13); 
			m_formatter.outputUri(uri);
			m_outbuf->sputn(">'/2).", 6);
			endl();
			m_outbuf->sputn(":- multifile('<", 15);
			m_formatter.outputUri(uri);
			m_outbuf->sputn(">'/2).", 6);
			endl();
			m_outbuf->sputn("pred('<", 7);
			m_formatter.outputUri(uri);
			m_outbuf->sputn(">').", 4);
			endl();
		}
	}

	inline void N3PWriter::outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object)
	{
		property.visit(m_formatter);
		m_outbuf->sputc('(');
		
		subject.visit(m_formatter);
		m_outbuf->sputc(',');
		
		object.visit(m_formatter);
		
		m_outbuf->sputc(')');
		m_outbuf->sputc('.');
		endl();
		
		m_count++;
	}
	
	
	
	
	
	
	
	
	const std::string N3PFormatter::SKOLEM_PREFIX("https://melgi.github.io/.well-known/genid/#");
	const char N3PFormatter::HEX_CHAR[] = "0123456789ABCDEF";	
	
	
	void N3PFormatter::visit(const URIResource &resource)
	{
		m_outbuf->sputc('\'');
		m_outbuf->sputc('<');
		outputUri(resource.uri());
		m_outbuf->sputc('>');
		m_outbuf->sputc('\'');
	}
	
	void N3PFormatter::visit(const BlankNode &blankNode)
	{
		const std::string &id = blankNode.id();
		
		m_outbuf->sputc('\'');
		m_outbuf->sputc('<');
		m_outbuf->sputn(SKOLEM_PREFIX.c_str(), SKOLEM_PREFIX.length());
		m_outbuf->sputn(id.c_str(), id.length());
		m_outbuf->sputc('>');
		m_outbuf->sputc('\'');
	}
	
	void N3PFormatter::visit(const Literal &literal)
	{
		m_outbuf->sputn("literal('", 9);
		output(literal.lexical());
		m_outbuf->sputn("',type('<", 9);
		outputUri(literal.datatype());
		m_outbuf->sputn(">'))", 4);
	}
	
	void N3PFormatter::visit(const BooleanLiteral &literal)
	{
		const std::string &lexical = literal.value() ? BooleanLiteral::VALUE_TRUE.lexical() : BooleanLiteral::VALUE_FALSE.lexical();
		
		m_outbuf->sputn(lexical.c_str(), lexical.length());
	}
	
	void N3PFormatter::visit(const IntegerLiteral &literal)
	{
		const std::string &lexical = literal.lexical();
		
		m_outbuf->sputn(lexical.c_str(), lexical.length());
	}
	
	void N3PFormatter::visit(const DoubleLiteral &literal)
	{
		const std::string &value = literal.lexical();
		
		// values like .5 and -.5 are not allowed in prolog
		// values like 5. and 5.E0 are not allowed in prolog
		
		bool appendZero = false;
		
		std::string s;
		const std::string *sp = &value;
		
		std::size_t p = value.find('.');
		if (p != std::string::npos) {
			++p;
			if (p == value.length()) {
				appendZero = true;
			} else if (value[p] == 'E' || value[p] == 'e') {
				s.reserve(value.length() + 1);
				s = value;
				s.insert(p, 1, '0');
				sp = &s;
			}
		}
		
		if ((*sp)[0] == '.') {
			m_outbuf->sputc('0');
			m_outbuf->sputn(sp->c_str(), sp->length());
			if (appendZero)
				m_outbuf->sputc('0');
		} else if ((*sp)[0] == '-' && (*sp)[1] == '.') {
			m_outbuf->sputc('-');
			m_outbuf->sputc('0');
			m_outbuf->sputn(sp->c_str() + 1, sp->length() - 1);
			if (appendZero)
				m_outbuf->sputc('0');
		} else {
			m_outbuf->sputn(sp->c_str(), sp->length());
			if (appendZero)
				m_outbuf->sputc('0');
		}
	}
	
	void N3PFormatter::visit(const DecimalLiteral &literal)
	{
		const std::string &value = literal.lexical();
		
		if (m_rdivDecimal) {
			std::size_t p = value.find('.');
			if (p == std::string::npos) {
				m_outbuf->sputn(value.c_str(), value.length());
				m_outbuf->sputn(" rdiv 1", 7);
			} else {
				m_outbuf->sputn(value.c_str(), p++);
				std::size_t len = value.length() - p;
				m_outbuf->sputn(value.c_str() + p, len);
				m_outbuf->sputn(" rdiv 1", 7);
				for (std::size_t i = 0; i < len; i++)
					m_outbuf->sputc('0');
			}
		} else {
			// values like .5 and -.5 are not allowed in prolog
			// values like 5. are not allowed in prolog
			
			if (value[0] == '.') {
				m_outbuf->sputc('0');
				m_outbuf->sputn(value.c_str(), value.length());
			} else if (value[0] == '-' && value[1] == '.') {
				m_outbuf->sputc('-');
				m_outbuf->sputc('0');
				m_outbuf->sputn(value.c_str() + 1, value.length() - 1);
			} else {
				m_outbuf->sputn(value.c_str(), value.length());
			}
			
			std::size_t length = value.length();
			if (length > 0 && value[length - 1] == '.')
				m_outbuf->sputc('0');
		}
	}

	void N3PFormatter::visit(const StringLiteral &literal)
	{
		m_outbuf->sputn("literal('", 9);
		output(literal.lexical());
		m_outbuf->sputc('\'');
		const std::string &lang = literal.language();
		if (!lang.empty()) {
			m_outbuf->sputn(",lang('", 7);
			m_outbuf->sputn(lang.c_str(), lang.length());
			m_outbuf->sputc('\'');
			m_outbuf->sputc(')');
		} else {
			m_outbuf->sputn(",type('<", 8);
			m_outbuf->sputn(StringLiteral::TYPE.c_str(), StringLiteral::TYPE.length());
			m_outbuf->sputc('>');
			m_outbuf->sputc('\'');
			m_outbuf->sputc(')');
		}
		
		m_outbuf->sputc(')');
	}
	
	void N3PFormatter::visit(const RDFList &list)
	{ 
		m_outbuf->sputc('[');
		if (!list.empty()) {
			auto i = list.begin();
			(*i)->visit(*this);
			++i;
			//m_count += 2;
			while (i != list.end()) {
				m_outbuf->sputc(',');
				(*i)->visit(*this);
				++i;
				//m_count += 2;
			}
		}
		m_outbuf->sputc(']');
	}
}
