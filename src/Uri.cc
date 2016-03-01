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
		
		if (m_authority != std::string::npos) {
			if (!(m_pathLength == 0 || m_value[m_path] == '/'))
				throw UriSyntaxException("path should be empty or start with '/'");
		} else {
			if (m_pathLength >= 2 && m_value[m_path] == '/' && m_value[m_path + 1] == '/')
				throw UriSyntaxException("path starts with '//'");
		}
		
		if (m_scheme == std::string::npos && m_authority == std::string::npos && m_pathLength != 0) {
			for (std::size_t i = 0; i < m_pathLength && m_value[m_path + i] != '/'; ++i) {
				if (m_value[m_path + i] == ':')
					throw UriSyntaxException("relative path reference contains a ':' in the first path segment");
			}
		}
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
					m_query = p + 1;
					parseQuery();
				} else {
					m_fragment = p + 1;
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
			path      = removeDotSegments(reference.path());
			query     = reference.query();
			
			if (!authority && path.empty() && !query) {
				return reference; // correct?
			}
		} else {
			if (reference.m_authority != std::string::npos) {
				authority = reference.authority();
				path      = removeDotSegments(reference.path());
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
						path = removeDotSegments(reference.path());
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

	
	
	std::string Uri::removeDotSegments(std::string input)
	{
		if (input.empty())
			return input;
		
		std::string output;
		output.reserve(input.length());
		
		// TODO iterator base approach
		while (input.length() > 0) {
			if (input.find("../") == 0)
				input.erase(0, 3);
			else if (input.find("./") == 0)
				input.erase(0, 2);
			else if (input.find("/./") == 0) {
				input.erase(0, 2);
			} else if (input.find("/.") == 0 && (input.length() == 2 || input[2] == '/')) {
				input.erase(1, 1);
			} else if (input.find("/../") == 0) {
				input.erase(0, 3);
				std::size_t n = output.rfind('/');
				if (n == std::string::npos)
					output.clear();
				else
					output.erase(n);
			} else if (input.find("/..") == 0 && (input.length() == 3 || input[3] == '/')) {
				input.erase(1, 2);
				std::size_t n = output.rfind('/');
				if (n == std::string::npos)
					output.clear();
				else
					output.erase(n);
			} else if (input.length() == 1 && input[0] == '.') {
				input.erase();
			} else if (input.length() == 2 && input[0] == '.' && input[1] == '.') {
				input.erase();
			} else {
				std::size_t n = input.find('/', 1);
				if (n != std::string::npos) {
					output.append(input, 0, n);
					input.erase(0, n);
				} else {
					output.append(input);
					input.clear();
				}
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
