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
	
	Uri Uri::parse(const std::string &s)
	{
		Optional<std::string> scheme;
		Optional<std::string> authority;
		std::string           path;
		Optional<std::string> query;
		Optional<std::string> fragment;
		
		ParseState state = ParseState::Scheme;
		std::size_t i = 0, j = 0;
		while (i < s.length()) {
			char c = s[i];
			
			if (state == ParseState::Scheme) {
				if (c == ':') {
					if (i == 0) {
						state  = ParseState::Path;
					} else {
						scheme = s.substr(0, i);
						j      = ++i;
						state  = ParseState::Authority;
					}
				} else if (c == '/') {
					if (i == 0) {
						j     = i;
						state = ParseState::Authority;
					} else {
						state = ParseState::Path;
					}
				} else if (c == '?') {
					path  = s.substr(0, i);
					j     = ++i;
					state = ParseState::Query;
				} else if (c == '#') {
					path  = s.substr(0, i);
					j     = ++i;
					state = ParseState::Fragment;
				} else
					++i;
			} else if (state == ParseState::Authority) {
				if (c == '/') {
					if (i != j && i != j + 1) {
						if (s[j] == '/' && s[j + 1] == '/') {
							j += 2;
							authority = s.substr(j, i - j);
							j         = i;
						}
						state = ParseState::Path;
					} else {
						++i;
					}
				} else if (c == '?') {
					if (s[j] == '/' && s[j + 1] == '/') {
						j += 2;
						authority = s.substr(j, i - j);
						j = ++i;
					} else {
						path = s.substr(j, i - j);
						j    = ++i;
					}
					state = ParseState::Query;
				} else if (c == '#') {
					if (s[j] == '/' && s[j + 1] == '/') {
						j += 2;
						authority = s.substr(j, i - j);
						j         = ++i;
					} else {
						path = s.substr(j, i - j);
						j    = ++i;
					}
					state = ParseState::Fragment;
				} else
					++i;
			} else if (state == ParseState::Path) {
				if (c == '?') {
					path  = s.substr(j, i - j);
					j     = ++i;
					state = ParseState::Query;
				} else if (c == '#') {
					path  = s.substr(j, i - j);
					j     = ++i;
					state = ParseState::Fragment;
				} else
					++i;
			} else if (state == ParseState::Query) {
				if (c == '#') {
					query = s.substr(j, i - j);
					j     = ++i;
					state = ParseState::Fragment;
				} else
					++i;
			} else {
				++i;
			}
		}
		
		if (state == ParseState::Scheme) {
			path = s.substr(j);
		} else if (state == ParseState::Authority) {
			if (s.length() - j >= 2 && s[j] == '/' && s[j + 1] == '/')
				authority = s.substr(j + 2);
			else
				path = s.substr(j);
		} else if (state == ParseState::Path) {
			path = s.substr(j);
		} else if (state == ParseState::Query) {
			query = s.substr(j);
		} else if (state == ParseState::Fragment) {
			fragment = s.substr(j);
		}
		
		
		if (authority) {
			if (!(path.empty() || path[0] == '/'))
				throw UriSyntaxException("path should be empty or start with '/'");
		} else {
			if (path.length() >= 2 && path[0] == '/' && path[1] == '/')
				throw UriSyntaxException("path starts with '//'");
		}
		
		if (!scheme && !authority && !path.empty()) {
			for (auto i = path.begin(); i != path.end() && *i != '/'; ++i) {
				if (*i == ':')
					throw UriSyntaxException("relative path reference contains a ':' in the first path segment");
			}
		}
		
		return Uri(scheme, authority, path, query, fragment);
	}
	
	void Uri::parseAuthority(const std::string &authority)
	{
		if (authority.empty())
			return;
		
		std::size_t a = authority.find('@'); // first position of '@' in authority, or std::string:npos if not found
		std::size_t c = std::string::npos; // last position of ':' in authority, or std::string:npos if not found or if c would be less than a
		
		std::size_t b = a + 1;
		if (b < authority.length() && authority[b] == '[') {
			b = authority.find(']', b + 1);
			if (b == std::string::npos)
				throw UriSyntaxException("unclosed bracket in authority");
			b++;
			if (b < authority.length()) {
				if (authority[b] == ':')
					c = b;
				else
					throw UriSyntaxException("illegal authority");
			}
		} else {
			c = authority.rfind(':');
		
			if (c != std::string::npos && a != std::string::npos && c < a)
				c = std::string::npos;
		}
		
		m_hostBegin  =  a == std::string::npos ? 0 : a + 1;
		m_hostLength = (c == std::string::npos ? authority.length() : c) - m_hostBegin;
		
		if (m_hostLength == 0)
			throw UriSyntaxException("host is empty");
	}
	
	Uri Uri::resolve(const Uri &reference) const
	{
		if (reference.absolute())
			return reference;
		
		//TODO prevent copying
		Optional<std::string> scheme;
		Optional<std::string> authority;
		std::string path;
		Optional<std::string> query;
		
		if (reference.scheme()) {
			scheme    = reference.scheme();
			authority = reference.authority();
			path      = removeDotSegments(reference.path());
			query     = reference.query();
			
			if (!authority && path.empty() && !query) {
				return reference; // correct?
			}
		} else {
			if (reference.authority()) {
				authority = reference.authority();
				path      = removeDotSegments(reference.path());
				query     = reference.query();
			} else {
				if (reference.path().empty()) {
					path = m_path;
					if (reference.query())
						query = reference.query();
					else
						query = m_query;
				} else {
					if (reference.path()[0] == '/') {
						path = removeDotSegments(reference.path());
					} else {
						if (m_authority && m_path.empty()) {
							path.append("/").append(reference.path());
						} else {
							if (m_path.empty())
								path = reference.path();
							else {
								std::size_t n = m_path.rfind('/');
								if (n == std::string::npos)
									path = reference.path();
								else
									path.append(m_path.substr(0, n + 1)).append(reference.path());
							}
						}
						path = removeDotSegments(path);
					}
					query = reference.query();
				}
				authority = m_authority;
			}
			scheme = m_scheme;
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
	
	Optional<std::string> Uri::userInfo() const
	{
		return m_authority &&  (m_hostBegin > 0) ? Optional<std::string>(m_authority->substr(0, m_hostBegin - 1)) : Optional<std::string>();
	}
	
	Optional<std::string> Uri::host() const
	{
		if (!m_authority)
			return Optional<std::string>();
		
		if (m_hostLength == m_authority->length())
			return m_authority;
		
		return Optional<std::string>(m_authority->substr(m_hostBegin, m_hostLength));
	}
	
	Optional<std::string> Uri::port() const
	{
		if (!m_authority)
			return Optional<std::string>();
		
		std::size_t n = m_hostBegin + m_hostLength;
		
		return n < m_authority->length() ? Optional<std::string>(m_authority->substr(n + 1)) : Optional<std::string>();
	}
	
	Uri::operator std::string() const
	{
		std::string result;
		
		std::size_t size = m_path.length();
		if (m_scheme) {
			size += m_scheme->length();
			size++;
		}
		
		if (m_authority) {
			size += m_authority->length();
			size += 2;
		}
		
		if (m_query) {
			size += m_query->length();
			size++;
		}
		
		if (m_fragment) {
			size += m_fragment->length();
			size++;
		}
		
		result.reserve(size);
		
		if (m_scheme) {
			result.append(*m_scheme);
			result.push_back(':');
		}
		
		if (m_authority)
			result.append("//").append(*m_authority);
			
		result.append(m_path);
		
		if (m_query) {
			result.push_back('?');
			result.append(*m_query);
		}
		
		if (m_fragment) {
			result.push_back('#');
			result.append(*m_fragment);
		}
		
		return result;
	}
	
	std::ostream &operator<<(std::ostream &out, const Uri &uri)
	{
		if (uri.scheme())
			out << *uri.scheme() << ':';
		
		if (uri.authority())
			out << "//" << *uri.authority();
		
		out << uri.path();
		
		if (uri.query())
			out << '?' << *uri.query();
		
		if (uri.fragment())
			out << '#' << *uri.fragment();
		
		return out;
	}


}
