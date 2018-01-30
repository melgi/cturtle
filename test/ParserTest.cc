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

class Graph {
	
	std::vector<Triple> m_triples;
	
public:

	Graph() : m_triples() {}
	
	
	Graph(const Graph &graph) : m_triples(graph.m_triples)
	{
	}
	
	Graph(Graph &&graph) : m_triples()
	{
		m_triples.swap(graph.m_triples);
	}
	
	Graph &operator=(Graph graph)
	{
		swap(graph);
	
		return *this;
	}
	
	Graph &operator=(Graph &&graph) noexcept
	{
		m_triples.swap(graph.m_triples);
	
		return *this;
	}
	
	~Graph()
	{
	}
	
	void swap(Graph &other) noexcept
	{
		m_triples.swap(other.m_triples);
	}
	
	void add(const turtle::Resource &subject, const turtle::URIResource &property, const turtle::N3Node &object)
	{
		m_triples.push_back(Triple(subject, property, object));
	}
	
	const Triple &operator[](std::size_t index) const
	{
		return m_triples[index];
	}
	
	Triple &operator[](std::size_t index)
	{
		return m_triples[index];
	}
	
	std::size_t size() const
	{
		return m_triples.size();
	}

};

class TestSink : public turtle::TripleSink {
	
	Graph m_graph;
	
public:
	
	TestSink() : m_graph()
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
		m_graph.add(subject, property, object);
	}
	
	unsigned count() const { return m_graph.size(); }
	
	const Graph &getResult() const { return m_graph; }
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
	
	const Graph &graph = handler.getResult();
	
	turtle::StringLiteral &literal = dynamic_cast<turtle::StringLiteral &>(graph[0].object());
	
	REQUIRE(literal.lexical() == expected);
	
	turtle::URIResource &resource = dynamic_cast<turtle::URIResource &>(graph[0].subject());
	
	REQUIRE(resource.uri().find(expected) == resource.uri().length() - expected.length());
}

TEST_CASE("surrogate pair", "[utf-16]")
{
	char32_t c = 0x29154;
	
	REQUIRE(turtle::utf16::highSurrogate(c) == 0xD864);
	REQUIRE(turtle::utf16::lowSurrogate(c)  == 0xDD54);
	
	REQUIRE(turtle::utf16::isHighSurrogate(0xD864));
	REQUIRE(turtle::utf16::isLowSurrogate(0xDD54));
	
	REQUIRE(!turtle::utf16::isHighSurrogate(0xDD54));
	REQUIRE(!turtle::utf16::isLowSurrogate(0xD864));
	
	std::string result;
	std::string expected = "\xED\xA1\xA4\xED\xB5\x94";
	
	turtle::utf16::encodeCESU8(c, std::back_inserter(result));
	
	REQUIRE(result == expected);
}

TEST_CASE("utf8", "[utf-8]")
{
	const char32_t REPLACEMENT_CHARACTER = U'\uFFFD';
	
	std::string input = u8"This contains \U00029154 and more \u00df\u6c34\U0001F34C.";
	std::u32string expected = U"This contains \U00029154 and more \u00df\u6c34\U0001F34C.";
	std::u32string result;
	
	turtle::utf8::State state;
	
	bool fail = false;
	auto i = input.cbegin();
	
	while (i < input.cend() && !fail) {
		char32_t c;
		int r = turtle::utf8::decode(&c, i, input.cend(), &state);
		if (r == -1) {
			c = REPLACEMENT_CHARACTER;
			i++;
		} else if (r != -2) {
			i += r;
		} else {
			fail = true;
		}
		result.push_back(c);
	}

	REQUIRE(!fail);
	REQUIRE(result == expected);
}
