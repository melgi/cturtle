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

#include "Uri.hh"

// ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?
//       12            3  4          5       6  7        8 9

namespace turtle {

	Uri::Uri(const Optional<std::string> &scheme,
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
	
	void Uri::parseComponents()
	{
		std::size_t p = m_value.find_first_of(":/?#");
		if (p == std::string::npos) {
			m_path       = 0;
			m_pathLength = m_value.length();
		} else {
			char c = m_value[p];
			if (c == ':') {
				if (p == 0) {
					m_path = 0;
					parsePath();
				} else {
					m_scheme       = 0;
					m_schemeLength = p;
					parseAuthority(p + 1);
				}
			} else if (c == '/') {
				parseAuthority(0);
			} else if (c == '?') {
				m_path       = 0;
				m_pathLength = p;
				m_query      = p + 1;
				parseQuery();
			} else if (c == '#') {
				m_path       = 0;
				m_pathLength = p;
				m_fragment   = p + 1;
			}
		}
		
		parseAuthorityComponents();
	}

	void Uri::parseAuthority(std::size_t begin)
	{
		if (m_value.length() - begin >= 2 && m_value[begin] == '/' && m_value[begin + 1] == '/') {
			begin += 2;
			m_authority = begin;
			std::size_t p = m_value.find_first_of("/?#", begin);
			if (p != std::string::npos) {
				m_authorityLength = p - begin;
				char c = m_value[p];
				if (c == '/') {
					m_path = p;
					parsePath();
				} else if (c == '?') {
					m_path       = p;
					m_pathLength = 0;
					m_query      = p + 1;
					parseQuery();
				} else {
					m_path       = p;
					m_pathLength = 0;
					m_fragment   = p + 1;
				}
			} else {
				m_authorityLength = m_value.length() - begin;
				m_path            = m_value.length();
				m_pathLength      = 0;
			}
		} else {
			m_path = begin;
			parsePath();
		}
	}

	void Uri::parsePath()
	{
		std::size_t p = m_value.find_first_of("?#", m_path);
		if (p != std::string::npos) {
			m_pathLength = p - m_path;
			if (m_value[p] == '?') {
				m_query = p + 1;
				parseQuery();
			} else {
				m_fragment = p + 1;
			}
		} else {
			m_pathLength = m_value.length() - m_path;
		}
	}

	void Uri::parseQuery()
	{
		std::size_t p = m_value.find('#', m_query);
		if (p != std::string::npos) {
			m_queryLength = p - m_query;
			m_fragment    = p + 1;
		}
	}
	
	void Uri::parseAuthorityComponents()
	{
		if (m_authority == std::string::npos || m_authorityLength == 0)
			return;
		
		const char *authoritybegin = m_value.c_str() + m_authority;
		const char *authorityEnd   = authoritybegin  + m_authorityLength;
		
		const char *p = nullptr;
		const char *h = std::string::traits_type::find(authoritybegin, m_authorityLength, '@');
		
		if (h)
			++h;
		else
			h = authoritybegin;
		
		if (h < authorityEnd) {
			if (*h == '[') {
				p = h + 1;
				p = std::string::traits_type::find(p, authorityEnd - p, ']');
				if (!p)
					throw UriSyntaxException("unclosed bracket in authority");
				
				if (++p < authorityEnd) {
					if (*p != ':')
						throw UriSyntaxException("illegal authority");
				}
			} else {
				p = std::string::traits_type::find(h, authorityEnd - h, ':');
			}
		}
		
		m_host       = h - m_value.c_str();
		m_hostLength = (p ? p : authorityEnd) - h;
		
		if (m_hostLength == 0)
			throw UriSyntaxException("host is empty " + m_value);
	}

	Uri Uri::resolve(const Uri &reference) const
	{
		if (reference.absolute())
			return reference;

		Optional<std::string> scheme;
		Optional<std::string> authority;
		std::string path;
		Optional<std::string> query;
		
		if (reference.m_scheme != std::string::npos) {
			scheme    = reference.scheme();
			authority = reference.authority();
			path      = removeDotSegments(reference.m_value, reference.m_path, reference.m_pathLength);
			query     = reference.query();
			
			if (!authority && path.empty() && !query) {
				return reference; // correct?
			}
		} else {
			if (reference.m_authority != std::string::npos) {
				authority = reference.authority();
				path      = removeDotSegments(reference.m_value, reference.m_path, reference.m_pathLength);
				query     = reference.query();
			} else {
				if (reference.m_pathLength == 0) {
					path = this->path();
					if (reference.m_query != std::string::npos)
						query = reference.query();
					else
						query = this->query();
				} else {
					if (reference.m_value[reference.m_path] == '/') {
						path = removeDotSegments(reference.m_value, reference.m_path, reference.m_pathLength);
					} else {
						if (m_authority != std::string::npos && m_pathLength == 0) {
							path.push_back('/');
							path.append(reference.m_value, reference.m_path, reference.m_pathLength);
						} else {
							if (m_pathLength == 0) {
								path = reference.path();
							} else {
								std::size_t n = m_value.rfind('/', m_path + m_pathLength);
								if (n == std::string::npos || n < m_path)
									path = reference.path();
								else
									path.append(m_value, m_path, n + 1 - m_path)
									    .append(reference.m_value, reference.m_path, reference.m_pathLength);
							}
						}
						path = removeDotSegments(path);
					}
					query = reference.query();
				}
				authority = this->authority();
			}
			scheme = this->scheme();
		}
		
		return Uri(scheme, authority, path, query, reference.fragment());
	}

	inline bool Uri::startsWith(const char *s, const char *prefix)
	{
		while (*prefix) {
			if (*s++ != *prefix++)
				return false;
		}
		
		return true;
	}
		
	///
	/// While the input buffer is not empty, loop as follows:
	///
	///   A.  If the input buffer begins with a prefix of "../" or "./",
	///       then remove that prefix from the input buffer; otherwise,
	///
	///   B.  if the input buffer begins with a prefix of "/./" or "/.",
	///       where "." is a complete path segment, then replace that
	///       prefix with "/" in the input buffer; otherwise,
	///
	///   C.  if the input buffer begins with a prefix of "/../" or "/..",
	///       where ".." is a complete path segment, then replace that
	///       prefix with "/" in the input buffer and remove the last
	///       segment and its preceding "/" (if any) from the output
	///       buffer; otherwise,
	///
	///   D.  if the input buffer consists only of "." or "..", then remove
	///       that from the input buffer; otherwise,
	///
	///   E.  move the first path segment in the input buffer to the end of
	///       the output buffer, including the initial "/" character (if
	///       any) and any subsequent characters up to, but not including,
	///       the next "/" character or the end of the input buffer.
	///
	std::string Uri::removeDotSegments(const std::string &input, std::size_t pos, std::size_t len)
	{
		if (len == std::string::npos)
			len = input.length() - pos;
			
		if (len == 0)
			return std::string();
			
		std::string output(0, '\0');
		output.reserve(len);
		
		const char *i   = input.c_str() + pos;
		const char *end = i + len;
				
		for (std::size_t left = end - i; left > 0; left = end - i) {
			if (left >= 3 && startsWith(i, "../")) { // A1
				i += 3;
			} else if (left >= 2 && (startsWith(i, "./") || startsWith(i, "/./"))) { // A2, B1
				i += 2;
			} else if (left == 2 && startsWith(i, "/.")) { // B2
				output.push_back('/');
				i = end;
			} else if (left >= 4 && startsWith(i, "/../")) { // C1
				i += 3;
				std::size_t n = output.rfind('/');
				if (n == std::string::npos)
					n = 0;
				output.erase(n);
			} else if (left == 3 && startsWith(i, "/..")) { // C2
				std::size_t n = output.rfind('/');
				if (n == std::string::npos)
					n = 0;
				output.erase(n);
				output.push_back('/');
				i = end;
			} else if (left == 1 && *i == '.') { // D1
				i = end;
			} else if (left == 2 && startsWith(i, "..")) { // D2
				i = end;
			} else { // E
				const char *p = std::string::traits_type::find(i + 1, left - 1, '/');
				if (!p)
					p = end;
				output.append(i, p - i);
				i = p;
			}
		}
		
		return output;
	}
		
	Uri::operator std::string() const
	{
		return m_value;
	}
	
	std::ostream &operator<<(std::ostream &out, const Uri &uri)
	{
		out << uri.m_value;
		
		return out;
	}

}
