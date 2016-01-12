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

#include <string>
#include <ostream>
#include <stdexcept>

#include "Optional.hh"

namespace turtle {
	
	class UriSyntaxException : public std::runtime_error {
	public:
		explicit UriSyntaxException(const std::string &reason = std::string()) : std::runtime_error(reason) {}
		explicit UriSyntaxException(const char *reason)                        : std::runtime_error(reason) {}
	};

	class Uri {
		
		enum class ParseState { Scheme, Authority, Path, Query, Fragment };
		
		Optional<std::string> m_scheme;
		Optional<std::string> m_authority;
		std::string m_path;
		Optional<std::string> m_query;
		Optional<std::string> m_fragment;
		
		std::size_t m_hostBegin;
		std::size_t m_hostLength;
		
		void parseAuthority(const std::string &authority);
		static std::string removeDotSegments(std::string input);
	public:
		Uri(const Optional<std::string> &scheme,
		const Optional<std::string> &authority,
		const std::string &path = std::string(),
		const Optional<std::string> &query    = Optional<std::string>(),
		const Optional<std::string> &fragment = Optional<std::string>())
			: m_scheme(scheme), m_authority(authority), m_path(path), m_query(query), m_fragment(fragment), m_hostBegin(0), m_hostLength(0)
		{
			if (m_authority)
				parseAuthority(*m_authority);
		}
		
		Uri(const std::string &scheme,
		const std::string &authority,
		const std::string &path = std::string(),
		const Optional<std::string> &query    = Optional<std::string>(),
		const Optional<std::string> &fragment = Optional<std::string>())
			: m_scheme(scheme), m_authority(authority), m_path(path), m_query(query), m_fragment(fragment), m_hostBegin(0), m_hostLength(0)
		{
			parseAuthority(*m_authority);
		}
		
		static Uri parse(const std::string &s);
		
		const Optional<std::string> &scheme()    const { return m_scheme;    } 
		const Optional<std::string> &authority() const { return m_authority; }
		const std::string           &path()      const { return m_path;      }
		const Optional<std::string> &query()     const { return m_query;     }
		const Optional<std::string> &fragment()  const { return m_fragment;  }
			
		Optional<std::string> userInfo() const;
		Optional<std::string> host()     const;
		Optional<std::string> port()     const;
			
		bool absolute() const { return static_cast<bool>(m_scheme); }
		
		Uri resolve(const Uri &reference) const;
		
		explicit operator std::string() const;
		
		friend std::ostream &operator<<(std::ostream &out, const Uri &uri); 
	};


}

#endif /* N3_URI_HH */
