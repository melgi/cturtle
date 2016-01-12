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

	const std::string N3PFormatter::SKOLEM_PREFIX("http://aca.agfa.net/.well-known/genid/#");
	const char N3PFormatter::HEX_CHAR[] = "0123456789ABCDEF";

	void N3PWriter::writePrologue()
	{
		m_out << ":- style_check(-discontiguous).\n";
		m_out << ":- style_check(-singleton).\n";
		m_out << ":- multifile(exopred/3).\n";
		m_out << ":- multifile(implies/3).\n";
		m_out << ":- multifile(pfx/2).\n";
		m_out << ":- multifile(pred/1).\n";
		m_out << ":- multifile(prfstep/8).\n";
		m_out << ":- multifile(scope/1).\n";
		m_out << ":- multifile(scount/1).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#mu>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#pi>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/fl-rules#sigma>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#biconditional>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#conditional>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#reflexive>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#relabel>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#tactic>'/2).\n";
		m_out << ":- multifile('<http://eulersharp.sourceforge.net/2003/03swap/log-rules#transaction>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#first>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#rest>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#implies>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/2000/10/swap/log#outputString>'/2).\n";
		m_out << ":- multifile('<http://www.w3.org/2002/07/owl#sameAs>'/2).\n";
		m_out << "flag('no-skolem', '" << N3PFormatter::SKOLEM_PREFIX << "').\n";
	}

	void N3PWriter::writeEpilogue()
	{
		m_out << "scount(" << m_count << ").\n";
		m_out << "end_of_file.\n" << std::flush;
	}

	void N3PWriter::triple(const Resource &subject, const URIResource &property, const N3Node &object)
	{
		if (m_properties.insert(property.uri()).second) { // TODO 3 times formatted
			m_out << ":- dynamic('<";
			m_formatter.outputUri(property.uri());
			m_out << ">'/2).\n";
			m_out << ":- multifile('<";
			m_formatter.outputUri(property.uri());
			m_out << ">'/2).\n";
			m_out << "pred('<";
			m_formatter.outputUri(property.uri());
			m_out << ">').\n";
		}
			
		outputTriple(subject, property, object);
	}

	void N3PWriter::outputTriple(const N3Node &subject, const URIResource &property, const N3Node &object)
	{
		property.visit(m_formatter);
		m_out << '(';
		
		subject.visit(m_formatter);
		m_out << ',';
		
		object.visit(m_formatter);
		
		m_out << ").\n";
		
		m_count++;
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
			m_out << '0' << *sp;
			if (appendZero)
				m_out << '0';
		} else if ((*sp)[0] == '-' && (*sp)[1] == '.') {
			m_out << "-0" << (*sp).substr(1);
			if (appendZero)
				m_out << '0';
		} else {
			m_out << *sp;
			if (appendZero)
				m_out << '0';
		}
	}
	
	void N3PFormatter::visit(const DecimalLiteral &literal)
	{
		const std::string &value = literal.lexical();
		
		// values like .5 and -.5 are not allowed in prolog
		// values like 5. are not allowed in prolog
		
		if (value[0] == '.') {
			m_out << '0' << value;
		} else if (value[0] == '-' && value[1] == '.') {
			m_out << "-0" << value.substr(1);
		} else {
			m_out << value;
		}
		
		std::size_t length = value.length();
		if (length > 0 && value[length - 1] == '.')
			m_out << '0';
	}

}
