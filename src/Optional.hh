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

#ifndef N3_OPTIONAL_HH
#define N3_OPTIONAL_HH


#include <stdexcept>
#include <utility>

namespace turtle {

	class InvalidOptionalAccessException : public std::runtime_error {
	public:
		explicit InvalidOptionalAccessException(const std::string &message = std::string()) : std::runtime_error(message) {}
	};

	template<typename T>
	class Optional {
		T    m_value;
		bool m_present;
		
		void valid() const { if (!m_present) throw InvalidOptionalAccessException(); }
	public:
		typedef T ValueType;
		
		static const Optional<T> none;
		
		Optional()               : m_value(T()),              m_present(false) {}
		Optional(const T &value) : m_value(value),            m_present(true)  {}
		Optional(T &&value)      : m_value(std::move(value)), m_present(true)  {}
		
		operator bool() const        { return m_present; }
		
		const T &operator*() const   { return m_value; }
		T &operator*()               { return m_value; }
		const T *operator->() const  { return &m_value; }
		T *operator->()              { return &m_value; }
		const T &value() const       { valid(); return m_value; }
		T &value()                   { valid(); return m_value; }
		
		friend bool operator==(const Optional &a, const Optional &b)
		{
			if (!a.m_present)
				return !b.m_present;
			
			if (!b.m_present)
				return false;
			
			return a.m_value == b.m_value;
		}
	};
	
	template<typename T> const Optional<T> Optional<T>::none = Optional<T>();

}

#endif /* N3_OPTIONAL_HH */
