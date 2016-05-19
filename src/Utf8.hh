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

namespace turtle {
	
	template<typename OutputIterator>
	std::size_t utf8Bytes(char32_t c, OutputIterator i)
	{
		std::size_t size = 0;
		if (c <= 0x7F) {
			*i++ = static_cast<char>(c);
			size = 1;
		} else if (0x0080 <= c && c <= 0x07FF) {
			char c0 = static_cast<char>(0x80 | (c & 0x3F));
			c >>= 6;
			char c1 = static_cast<char>(0xC0 | (c & 0x1F));
			*i++ = c1;
			*i++ = c0;
			size = 2;
		} else if (0x0800 <= c && c <= 0xFFFF) {
			char c0 = static_cast<char>(0x80 | (c & 0x3F));
			c >>= 6;
			char c1 = static_cast<char>(0x80 | (c & 0x3F));
			c >>= 6;
			char c2 = static_cast<char>(0xE0 | (c & 0x0F));
			*i++ = c2;
			*i++ = c1;
			*i++ = c0;
			size = 3;
		} else if (0x00010000 <= c && c <= 0x0010FFFF) {
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


	template<typename InputIterator>
	std::size_t utf8Decode(char32_t &v, InputIterator pos, InputIterator end)
	{
		std::size_t len = 0;
		bool valid = true;
		char c = *pos;
		if ((c & 0x80) == 0) {
			v = static_cast<char32_t>(c);
			len++;
		} else if ((c & 0xE0) == 0xC0) {
			v = static_cast<char32_t>(c) & 0x1F;
			len++;
			if (++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && v < 0x80)
				valid = false;
		} else if ((c & 0xF0) == 0xE0) {
			v = static_cast<char32_t>(c) & 0x0F;
			len++;
			if (++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && ++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && v < 0x0800)
				valid = false;
		} else if ((c & 0xF8) == 0xF0) {
			v = static_cast<char32_t>(c) & 0x07;
			len++;
			if (valid && ++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && ++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && ++pos < end) {
				c = *pos;
				if ((c & 0xC0) == 0x80) {
					v <<= 6;
					v |= static_cast<char32_t>(c) & 0x3F;
					len++;
				} else {
					valid = false;
				}
			} else
				valid = false;
			
			if (valid && v < 0x10000)
				valid = false;
		} else {
			v = 0xFFFD;
			valid = false;
			len++;
		}
		
		if (!valid)
			v = 0xFFFD;
		
		return len;
	}

}

#endif /* N3_UTF8_HH */
