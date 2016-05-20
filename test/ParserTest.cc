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

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <sstream>

#include "../src/Uri.hh"
#include "../src/Parser.hh"
#include "../src/NTriplesWriter.hh"
#include "../src/Utf16.hh"
#include "catch.hpp"


class Triple {
	turtle::Resource *m_subject;
	turtle::URIResource *m_property;
	turtle::N3Node *m_object;
	
public:

	Triple(const turtle::Resource &subject, const turtle::URIResource &property, const turtle::N3Node &object)
		: m_subject(subject.clone()), m_property(property.clone()), m_object(object.clone())
	{
		// nop
	}
	
	Triple(const Triple &triple)
		: m_subject(triple.m_subject->clone()), m_property(triple.m_property->clone()), m_object(triple.m_object->clone())
	{
		// nop
	}
	
	Triple(Triple&& triple) : m_subject(), m_property(), m_object()
	{
		swap(*this, triple);
	}
	
	Triple &operator=(Triple triple)
	{
		swap(*this, triple);
		
		return *this;
	}
	
	~Triple()
	{
		delete m_subject;
		delete m_property;
		delete m_object;
	}
	
	turtle::Resource    &subject()  const { return *m_subject;  }
	turtle::URIResource &property() const { return *m_property; }
	turtle::N3Node      &object()   const { return *m_object;   }
	
	friend void swap(Triple& first, Triple& second)
    {
		std::swap(first.m_subject, second.m_subject);
        std::swap(first.m_property, second.m_property);
		std::swap(first.m_object, second.m_object);
    }
	
};


class TestSink : public turtle::TripleSink {
				
		std::vector<Triple> m_triples;
		
public:

		TestSink() : m_triples()
		{
			// nop
		}

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
		
		void triple(const turtle::Resource &subject, const turtle::URIResource &property, const turtle::N3Node &object)
		{
			m_triples.push_back(Triple(subject, property, object));
		}
		
		unsigned count() const { return m_triples.size(); }
		
		const std::vector<Triple> &getResult() const { return m_triples; }
	};


TEST_CASE("escaping", "[parser]")
{
	turtle::Uri base("http://localhost/test");
	
	// U+29154 CJK UNIFIED IDEOGRAPH-29154
	std::stringstream input("PREFIX ex: <http://example.org#>\n<http://localhost/test#\\uD864\\uDD54> ex:value \"\\uD864\\uDD54\".\n");
	std::string expected = "\xF0\xA9\x85\x94";
	
	TestSink handler;
	
	handler.start();
	turtle::Parser parser(&input, base, &handler);
	parser.parse();
	handler.end();

	REQUIRE(handler.count() == 1);
	
	const std::vector<Triple> &graph = handler.getResult();
	
	turtle::StringLiteral &literal = dynamic_cast<turtle::StringLiteral &>(graph[0].object());
		
	REQUIRE(literal.lexical() == expected);

	turtle::URIResource &resource = dynamic_cast<turtle::URIResource &>(graph[0].subject());
	
	REQUIRE(resource.uri().find(expected) == resource.uri().length() - expected.length());
}

TEST_CASE("surrogate pair", "[utf-16]")
{
	char32_t c = 0x29154;
	
	REQUIRE(turtle::utf16::hiSurrogate(c) == 0xD864);
	REQUIRE(turtle::utf16::loSurrogate(c) == 0xDD54);
	
	std::string result;
	std::string expected = "\xED\xA1\xA4\xED\xB5\x94";
	
	turtle::utf16::encodeCESU8(c, std::back_inserter(result));

	REQUIRE(result == expected);
}
