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

#include "NTriplesWriter.hh"

#include <utility> // std::move

namespace turtle {
	
	std::unique_ptr<BlankNode> NTriplesWriter::triples(const RDFList &list)
	{
		std::string id = m_idgen.generate();
		
		std::unique_ptr<BlankNode> head(new BlankNode(id));
			
		for (std::size_t i = 0; i < list.size(); i++) {
			const N3Node *node = list[i];
			
			std::unique_ptr<BlankNode> nestedList;
			if (const RDFList *rl = dynamic_cast<const RDFList *>(node)) {
				if (rl->empty()) {
					node = &RDF::nil;
				} else {
					nestedList = triples(*rl);
					node = nestedList.get();
				}
			}
				
			rawTriple(*head, RDF::first, *node);
			
			if (i == list.size() - 1) {
				rawTriple(*head, RDF::rest, RDF::nil);
			} else {
				std::unique_ptr<BlankNode> rest(new BlankNode(m_idgen.generate()));
				rawTriple(*head, RDF::rest, *rest);
				head = std::move(rest);
			}
		}
			
		return std::unique_ptr<BlankNode>(new BlankNode(id));
	}

	void NTriplesWriter::triple(const Resource &subject, const URIResource &property, const N3Node &object)
	{
		std::unique_ptr<BlankNode> sp; // prevent sp.get() getting deleted
		const Resource *s = &subject;
		if (const RDFList *list = dynamic_cast<const RDFList *>(s)) {
			if (list->empty()) {
				s = &RDF::nil;
			} else {
				sp = triples(*list); 
				s = sp.get();
			}
		}
		
		std::unique_ptr<BlankNode> op; // prevent op.get() getting deleted
		const N3Node *o = &object;
		if (const RDFList *list = dynamic_cast<const RDFList *>(o)) {
			if (list->empty()) {
				o = &RDF::nil;
			} else {
				op = triples(*list);
				o = op.get();
			}
		}
		
		rawTriple(*s, property, *o);
	}

	inline void NTriplesWriter::rawTriple(const Resource &subject, const URIResource &property, const N3Node &object)
	{
		m_count++;
		subject.visit(m_formatter);
		m_outbuf->sputc(' ');
		property.visit(m_formatter);
		m_outbuf->sputc(' ');
		object.visit(m_formatter);
		m_outbuf->sputc(' ');
		m_outbuf->sputc('.');
		
#ifdef CTURTLE_CRLF
		m_outbuf->sputc('\r');
#endif
		m_outbuf->sputc('\n');
	}

}
