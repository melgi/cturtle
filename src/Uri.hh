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

#ifndef N3_URI_HH
#define N3_URI_HH

#include <cstddef>
#include <string>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "Optional.hh"

namespace turtle {
	
	class UriSyntaxException : public std::runtime_error {
	public:
		explicit UriSyntaxException(const std::string &reason = std::string()) : std::runtime_error(reason) {}
		explicit UriSyntaxException(const char *reason)                        : std::runtime_error(reason) {}
	};

	///
	/// Non-validating URI parser, see https://tools.ietf.org/html/rfc3986
	///
	class Uri {

		std::string m_value;
		
		std::size_t m_scheme;    std::size_t m_schemeLength;
		std::size_t m_authority; std::size_t m_authorityLength;
		std::size_t m_host;      std::size_t m_hostLength;
		std::size_t m_path;      std::size_t m_pathLength;
		std::size_t m_query;     std::size_t m_queryLength;
		std::size_t m_fragment;
		
		Uri(const Optional<std::string> &scheme,
		    const Optional<std::string> &authority,
		    const std::string &path,
		    const Optional<std::string> &query,
		    const Optional<std::string> &fragment)
				: m_value(),
				  m_scheme(std::string::npos),    m_schemeLength(std::string::npos),
				  m_authority(std::string::npos), m_authorityLength(std::string::npos),
				  m_host(std::string::npos),      m_hostLength(std::string::npos),
				  m_path(std::string::npos),      m_pathLength(std::string::npos),
				  m_query(std::string::npos),     m_queryLength(std::string::npos),
				  m_fragment(std::string::npos)
		
		{
			if (scheme) {
				m_scheme       = 0;
				m_schemeLength = scheme->length();
				m_value.append(*scheme).push_back(':');
			}
			
			if (authority) {
				m_value.append("//");
				m_authority       = m_value.length();
				m_authorityLength = authority->length();
				m_value.append(*authority);
				parseAuthorityComponents();
			}
			
			m_path       = m_value.length();
			m_pathLength = path.length();
			m_value.append(path);
			
			if (query) {
				m_value.push_back('?');
				m_query       = m_value.length();
				m_queryLength = query->length();
				m_value.append(*query);
			}
			
			if (fragment) {
				m_value.push_back('#');
				m_fragment = m_value.length();
				m_value.append(*fragment);
			}
		}
		
		void parseComponents();
		void parseAuthority(std::size_t begin);
		void parseAuthorityComponents();
		void parsePath();
		void parseQuery();
		
		static std::string removeDotSegments(const std::string &input, std::size_t pos = 0, std::size_t len = std::string::npos);
		static bool startsWith(const char *s, const char *prefix);

	public:
	
		Uri(const std::string &value)
			: m_value(value),
			  m_scheme(std::string::npos),    m_schemeLength(std::string::npos),
			  m_authority(std::string::npos), m_authorityLength(std::string::npos),
			  m_host(std::string::npos),      m_hostLength(std::string::npos),
			  m_path(std::string::npos),      m_pathLength(std::string::npos),
			  m_query(std::string::npos),     m_queryLength(std::string::npos),
			  m_fragment(std::string::npos)
		{
			parseComponents();
		}
		
		Uri(std::string &&value)
			: m_value(std::move(value)),
			  m_scheme(std::string::npos),    m_schemeLength(std::string::npos),
			  m_authority(std::string::npos), m_authorityLength(std::string::npos),
			  m_host(std::string::npos),      m_hostLength(std::string::npos),
			  m_path(std::string::npos),      m_pathLength(std::string::npos),
			  m_query(std::string::npos),     m_queryLength(std::string::npos),
			  m_fragment(std::string::npos)
		{
			parseComponents();
		}
		
		Optional<std::string> scheme() const
		{
			return m_scheme != std::string::npos ? Optional<std::string>(m_value.substr(m_scheme, m_schemeLength)) : Optional<std::string>::none;
		}
		
		Optional<std::string> authority() const
		{
			return m_authority != std::string::npos ? Optional<std::string>(m_value.substr(m_authority, m_authorityLength)) : Optional<std::string>::none;
		}
		
		std::string path() const
		{
			return m_value.substr(m_path, m_pathLength);
		}
		
		Optional<std::string> query() const
		{
			return m_query != std::string::npos ? Optional<std::string>(m_value.substr(m_query, m_queryLength)) : Optional<std::string>::none;
		}
		
		Optional<std::string> fragment() const
		{
			return m_fragment != std::string::npos ? Optional<std::string>(m_value.substr(m_fragment)) : Optional<std::string>::none;
		}
		
		Optional<std::string> userInfo() const
		{
			return m_authority != std::string::npos && (m_host > m_authority) ? Optional<std::string>(m_value.substr(m_authority, m_host - 1 - m_authority)) : Optional<std::string>::none;
		}
	
		Optional<std::string> host() const
		{
			return m_host != std::string::npos ? Optional<std::string>(m_value.substr(m_host, m_hostLength)) : Optional<std::string>::none;
		}
		
		Optional<std::string> port() const
		{
			if (m_authority == std::string::npos)
				return Optional<std::string>::none;
			
			std::size_t n = m_host + m_hostLength;
			
			return n < m_authority + m_authorityLength ? Optional<std::string>(m_value.substr(n + 1, m_authority + m_authorityLength - n - 1)) : Optional<std::string>::none;
		}
		
		bool absolute() const { return m_scheme != std::string::npos; }
			
		Uri resolve(const Uri &reference) const;
			
		explicit operator std::string() const;

		explicit operator const std::string&() const &;
		explicit operator std::string&&() &&;
			
		friend std::ostream &operator<<(std::ostream &out, const Uri &uri);
		
	};


}

#endif /* N3_URI_HH */
