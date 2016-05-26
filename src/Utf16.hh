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

#ifndef N3_UTF16_HH
#define N3_UTF16_HH

#include "Utf8.hh"

namespace turtle {
	
	namespace utf16 {
	
		const uint16_t HI_SURROGATE_START = 0xD800;
		const uint16_t HI_SURROGATE_END   = 0xDBFF;
		const uint16_t LO_SURROGATE_START = 0xDC00;
		const uint16_t LO_SURROGATE_END   = 0xDFFF;
		
		const uint16_t SURROGATE_MASK     = 0xFC00;
		
		bool isHighSurrogate(std::uint16_t i);
		bool isLowSurrogate(std::uint16_t i);
		char32_t toChar(std::uint16_t high, std::uint16_t low);
		std::uint16_t highSurrogate(char32_t c);
		std::uint16_t lowSurrogate(char32_t c);
		
		// Compatibility Encoding Scheme for UTF-16: 8-Bit (CESU-8)
		// (http://www.unicode.org/reports/tr26/)
		template<typename OutputIterator> std::size_t encodeCESU8(char32_t c, OutputIterator i);
	};
	
	inline bool utf16::isHighSurrogate(std::uint16_t i)
	{
		return (i & SURROGATE_MASK) == HI_SURROGATE_START;
	}
	
	inline bool utf16::isLowSurrogate(std::uint16_t i)
	{
		return (i & SURROGATE_MASK) == LO_SURROGATE_START;
	}
	
	inline char32_t utf16::toChar(std::uint16_t high, std::uint16_t low)
	{
		std::uint32_t n = (high & 0x003F) << 10 | (low & 0x03FF);
		std::uint32_t m = (high >> 6) & 0x001F;
		
		return (m + 1) << 16 | n;
	}
	
	inline std::uint16_t utf16::highSurrogate(char32_t c)
	{
		std::uint16_t n = static_cast<std::uint16_t>(c);
		std::uint16_t m = static_cast<std::uint16_t>((c >> 16) & 0x001F) - 1;
		
		return HI_SURROGATE_START | (m << 6) | (n >> 10);
	}
	
	inline std::uint16_t utf16::lowSurrogate(char32_t c)
	{
		return LO_SURROGATE_START | (c & 0x3FF);
	}
	
	// Compatibility Encoding Scheme for UTF-16: 8-Bit (CESU-8)
	// (http://www.unicode.org/reports/tr26/)
	template<typename OutputIterator> std::size_t utf16::encodeCESU8(char32_t c, OutputIterator i)
	{
		if (c <= 0xFFFF)
			return utf8::encode(c, i);
		
		if (c >= 0x0010FFFF)
			return 0;
		
		utf8::encode(highSurrogate(c), i);
		utf8::encode(lowSurrogate(c), i);
		
		return 6;
	}
	
}

#endif /* N3_UTF16_HH */
