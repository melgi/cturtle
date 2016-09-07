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

#ifndef N3_UTF8_HH
#define N3_UTF8_HH

#include <cstdint>
#include <cstddef>

namespace turtle {
	
	namespace utf8 {
	
		template<typename OutputIterator>
		std::size_t encode(char32_t c, OutputIterator i)
		{
			std::size_t size = 0;
			
			if (c <= 0x7F) {
				*i++ = static_cast<char>(c);
				size = 1;
			} else if (c <= 0x07FF) {
				char c0 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c1 = static_cast<char>(0xC0 | (c & 0x1F));
				*i++ = c1;
				*i++ = c0;
				size = 2;
			} else if (c <= 0xFFFF) {
				char c0 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c1 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c2 = static_cast<char>(0xE0 | (c & 0x0F));
				*i++ = c2;
				*i++ = c1;
				*i++ = c0;
				size = 3;
			} else if (c <= 0x0010FFFF) {
				char c0 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c1 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c2 = static_cast<char>(0x80 | (c & 0x3F));
				c >>= 6;
				char c3 = static_cast<char>(0xF0 | (c & 0x07));
				*i++ = c3;
				*i++ = c2;
				*i++ = c1;
				*i++ = c0;
				size = 4;
			}
			
			return size;
		}

		
		
		

		// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
		// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
		class Decoder {
		
			static const uint8_t m_type[];
			static const uint8_t m_transition[];
				
		public:

			static const uint32_t ACCEPT =  0;
			static const uint32_t REJECT = 12;
		
			
			static uint32_t decode(char32_t *codepoint, uint8_t byte, uint32_t *state)
			{
				uint32_t type = m_type[byte];

				*codepoint = (*state != ACCEPT) ?
					(byte & 0x3Fu) | (*codepoint << 6) :
					(0xFF >> type) & (byte);

				*state = m_transition[*state + type];
				
				return *state;
			}
		};
		
		class State {
		
			char32_t m_codepoint;
			uint32_t m_state;
			
		public:
			State() : m_codepoint(0), m_state(Decoder::ACCEPT) {}
			
			template<typename InputIterator> friend	std::size_t decode(char32_t *c32, InputIterator begin, InputIterator end, State *state);
		};
	
		template<typename InputIterator>
		std::size_t decode(char32_t *c32, InputIterator begin, InputIterator end, State *state)
		{
			InputIterator pos = begin;
				
			while (pos < end) {
				if (!Decoder::decode(&state->m_codepoint, *pos++, &state->m_state)) {
					*c32 = state->m_codepoint;
					return pos - begin;
				} else if (state->m_state == Decoder::REJECT) {
					return -1;
				}
			}
			
			return -2;
		}
		
	}
	
}

#endif /* N3_UTF8_HH */
