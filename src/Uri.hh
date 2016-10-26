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
		    const Optional<std::string> &fragment);
		
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

		void swap(Uri &uri) noexcept
		{
			m_value.swap(uri.m_value);
			std::swap(m_scheme, uri.m_scheme);       std::swap(m_schemeLength, uri.m_schemeLength);
			std::swap(m_authority, uri.m_authority); std::swap(m_authorityLength, uri.m_authorityLength);
			std::swap(m_host, uri.m_host);           std::swap(m_hostLength, uri.m_hostLength);
			std::swap(m_path, uri.m_path);           std::swap(m_pathLength, uri.m_pathLength);
			std::swap(m_query, uri.m_query);         std::swap(m_queryLength, uri.m_queryLength);
			std::swap(m_fragment, uri.m_fragment);
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
		
		static bool absolute(const std::string &uri) noexcept
		{
			int p = uri.find_first_of(":/?#");
			
			return p > 0 && uri[p] == ':';
		}
		
		Uri resolve(const Uri &reference) const;
			
		explicit operator std::string() const;
			
		friend std::ostream &operator<<(std::ostream &out, const Uri &uri);
		
	};

	inline void swap(Uri &x, Uri &y) noexcept
	{
		x.swap(y);
	}
}

#endif /* N3_URI_HH */
