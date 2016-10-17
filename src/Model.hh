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

#ifndef N3_MODEL_HH
#define N3_MODEL_HH

#include <cstddef>
#include <string>
#include <ostream>
#include <vector>
#include <utility>


namespace turtle {

	class URIResource;
	class BlankNode;
	class RDFList;
	class Literal;
	class BooleanLiteral;
	class IntegerLiteral;
	class DoubleLiteral;
	class DecimalLiteral;
	class StringLiteral;

	template<typename T>
	struct Cloneable {
		virtual T *clone() const = 0;
		virtual ~Cloneable() {};
	};

	struct N3NodeVisitor {
		virtual void visit(const URIResource &resource) = 0;
		virtual void visit(const BlankNode &blankNode) = 0;
		virtual void visit(const Literal &literal) = 0;
		virtual void visit(const RDFList &list) = 0;
		virtual void visit(const BooleanLiteral &literal) = 0;
		virtual void visit(const IntegerLiteral &literal) = 0;
		virtual void visit(const DoubleLiteral &literal) = 0;
		virtual void visit(const DecimalLiteral &literal) = 0;
		virtual void visit(const StringLiteral &literal) = 0;
		virtual ~N3NodeVisitor() {}
	};

	struct N3Node : public Cloneable<N3Node> {
		virtual std::ostream &print(std::ostream &out) const = 0;
		virtual void visit(N3NodeVisitor &visitor) const = 0;
	};

	std::ostream &operator<<(std::ostream &out, const N3Node &n);

	struct Resource : public N3Node {
		virtual Resource *clone() const = 0;
	};


	class URIResource : public Resource {
		std::string m_uri;
	public:
		explicit URIResource(const std::string &uri) : Resource(), m_uri(uri) {}
		explicit URIResource(std::string &&uri)      : Resource(), m_uri(std::move(uri)) {}
		
		const std::string &uri() const { return m_uri; }
		
		std::ostream &print(std::ostream &out) const
		{
			out << '<' << m_uri << '>';
			
			return out;
		}
		
		URIResource *clone() const
		{
			return new URIResource(m_uri);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
		
	};

	
	class BlankNode : public Resource {
		std::string m_id;
	public:
		explicit BlankNode(const std::string &id) : Resource(), m_id(id) {}
		explicit BlankNode(std::string &&id)      : Resource(), m_id(std::move(id)) {}
		
		const std::string &id() const { return m_id; }
		
		std::ostream &print(std::ostream &out) const
		{
			out << "_:b" << m_id;
			
			return out;
		}
		
		BlankNode *clone() const
		{
			return new BlankNode(m_id);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};
	

	class RDFList : public Resource {
		std::vector<N3Node *> m_elements;
		
		typedef std::vector<N3Node *>::iterator iterator;
		typedef std::vector<N3Node *>::const_iterator const_iterator;
		
	public:
		RDFList() : m_elements() {}
		
		RDFList(const RDFList &list) : RDFList()
		{
			m_elements.reserve(list.m_elements.size());
			for (N3Node *n : list.m_elements) {
				m_elements.push_back(n->clone());
			}
		}
		
		RDFList(RDFList &&list) : RDFList()
		{
			m_elements.swap(list.m_elements);
		}
		
		RDFList& operator=(RDFList list)
		{
			m_elements.swap(list.m_elements);
			
			return *this;
		}
		
		RDFList& operator=(RDFList &&list) noexcept
		{
			m_elements.swap(list.m_elements);
			
			return *this;
		}
		
		~RDFList()
		{
			for (N3Node *n : m_elements)
				delete n;
		}
		
		void add(N3Node *element)
		{
			m_elements.push_back(element);
		}
		
		N3Node *&operator[](std::size_t index)
		{
			return m_elements[index];
		}

		N3Node * const &operator[](std::size_t index) const
		{
			return m_elements[index];
		}
		
		std::size_t size() const
		{
			return m_elements.size();
		}
		
		iterator begin()
		{
			return m_elements.begin();
		}
		
		iterator end()
		{
			return m_elements.end();
		}
		
		const_iterator begin() const
		{
			return m_elements.begin();
		}
		
		const_iterator end() const
		{
			return m_elements.end();
		}
		
		bool empty() const
		{
			return m_elements.empty();
		}
		
		std::ostream &print(std::ostream &out) const
		{
			out << '(';
			
			for (N3Node *n : m_elements)
				out << ' ' << *n;
			
			out << ')';
			
			return out;
		}
		
		RDFList *clone() const
		{
			return new RDFList(*this);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
		
	};


	class Literal : public N3Node {
	protected:
		std::string m_lexical;
		const std::string *m_datatype;
		
		Literal(const std::string &lexical, const std::string *datatype)     : N3Node(), m_lexical(lexical), m_datatype(datatype) {}
		Literal(std::string &&lexical, const std::string *datatype) noexcept : N3Node(), m_lexical(std::move(lexical)), m_datatype(datatype) {}
		
	public:
		
		const std::string &lexical() const { return m_lexical; }
		
		const std::string &datatype() const { return *m_datatype; }
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
		
	};
	
	class BooleanLiteral : public Literal {
	public:
		explicit BooleanLiteral(const std::string &value) : Literal(value, &TYPE) {}
		
		static const std::string TYPE;
		
		static const BooleanLiteral VALUE_TRUE;
		static const BooleanLiteral VALUE_FALSE;
		static const BooleanLiteral VALUE_1;
		static const BooleanLiteral VALUE_0;
		
		std::ostream &print(std::ostream &out) const
		{
			out << lexical();
			
			return out;
		}
		
		bool value() const { return m_lexical == VALUE_TRUE.m_lexical || m_lexical == "1"; }
		
		BooleanLiteral *clone() const
		{
			return new BooleanLiteral(m_lexical);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};

	class IntegerLiteral : public Literal {
	public:
		static const std::string TYPE;
		
		explicit IntegerLiteral(const std::string &value) : Literal(value, &TYPE) {}
		
		std::ostream &print(std::ostream &out) const
		{
			out << lexical();
			
			return out;
		}
		
		IntegerLiteral *clone() const
		{
			return new IntegerLiteral(m_lexical);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};

	class DoubleLiteral : public Literal {
	public:
		static const std::string TYPE;
		
		explicit DoubleLiteral(const std::string &value) : Literal(value, &TYPE) {}
		
		std::ostream &print(std::ostream &out) const
		{
			out << lexical();
			
			return out;
		}
		
		DoubleLiteral *clone() const
		{
			return new DoubleLiteral(m_lexical);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};

	class DecimalLiteral : public Literal {
	public:
		static const std::string TYPE;
		
		explicit DecimalLiteral(const std::string &value) : Literal(value, &TYPE) {}
		
		std::ostream &print(std::ostream &out) const
		{
			out << lexical();
			
			return out;
		}
		
		DecimalLiteral *clone() const
		{
			return new DecimalLiteral(m_lexical);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};

	class StringLiteral : public Literal {
		
		std::string m_language;

	public:
		static const std::string TYPE;
		
		explicit StringLiteral(const std::string &value, const std::string &language = std::string()) : Literal(value, &TYPE), m_language(language) {}
		explicit StringLiteral(std::string &&value, std::string &&language = std::string()) : Literal(std::move(value), &TYPE), m_language(std::move(language)) {}
		
		const std::string &language() const { return m_language; }
		
		std::ostream &print(std::ostream &out) const
		{
			out << '"' << lexical() << '"';
			
			if (!language().empty())
				out << '@' << language();
			
			return out;
		}
		
		StringLiteral *clone() const
		{
			return new StringLiteral(m_lexical, m_language);
		}
		
		void visit(N3NodeVisitor &visitor) const
		{
			visitor.visit(*this);
		}
	};
	
	class OtherLiteral : public Literal { /* keeps a copy of the type uri */
		
		std::string m_datatype_copy;
		
	public:
		
		explicit OtherLiteral(const std::string &value, const std::string &datatype) : Literal(value, nullptr), m_datatype_copy(datatype)
		{
			m_datatype = &m_datatype_copy;
		}
		
		explicit OtherLiteral(std::string &&value, std::string &&datatype) noexcept : Literal(std::move(value), nullptr), m_datatype_copy(std::move(datatype))
		{
			m_datatype = &m_datatype_copy;
		}
		
		OtherLiteral(const OtherLiteral &other) : Literal(other.m_lexical, nullptr), m_datatype_copy(other.m_datatype_copy)
		{
			m_datatype = &m_datatype_copy;
		}
		
		OtherLiteral(OtherLiteral &&other) noexcept : Literal(std::move(other.m_lexical), nullptr), m_datatype_copy(std::move(other.m_datatype_copy))
		{
			m_datatype = &m_datatype_copy;
		}
		
		OtherLiteral& operator=(OtherLiteral other)
		{
			swap(other);
			
			return *this;
		}
		
		OtherLiteral& operator=(OtherLiteral &&other) noexcept
		{
			swap(other);
			
			return *this;
		}
		
		std::ostream &print(std::ostream &out) const
		{
			out << '"' << lexical() << '"' << '@' << '<' << datatype() << '>';
			
			return out;
		}
		
		OtherLiteral *clone() const
		{
			return new OtherLiteral(m_lexical, m_datatype_copy);
		}

		void swap(OtherLiteral &other)
		{
			m_lexical.swap(other.m_lexical);
			m_datatype_copy.swap(other.m_datatype_copy);

//			m_datatype = &m_datatype_copy;
		}
	};
	
	inline void swap(OtherLiteral &x, OtherLiteral &y)
	{
		x.swap(y);
	}
	
	
	struct RDF {
		static const std::string NS;
		
		static const URIResource type;
		static const URIResource first;
		static const URIResource rest;
		static const URIResource nil;
	};

	struct XSD {
		static const std::string NS;
	};

}

#endif /* N3_MODEL_HH */
