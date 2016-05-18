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
	
	struct utf16 {
	
		static const uint16_t HiSurrogateStart = 0xD800;
		static const uint16_t HiSurrogateEnd   = 0xDBFF;
		static const uint16_t LoSurrogateStart = 0xDC00;
		static const uint16_t LoSurrogateEnd   = 0xDFFF;
		
		static bool isHighSurrogate(std::uint16_t i);
		static bool isLowSurrogate(std::uint16_t i);
		static char32_t toChar(std::uint16_t high, std::uint16_t low);
		static std::uint16_t hiSurrogate(char32_t c);
		static std::uint16_t loSurrogate(char32_t c);
				
		// Compatibility Encoding Scheme for UTF-16: 8-Bit (CESU-8)
		// (http://www.unicode.org/reports/tr26/)
		template<typename OutputIterator> static std::size_t cesu8Bytes(char32_t c, OutputIterator i);
	};
	
	
	inline bool utf16::isHighSurrogate(std::uint16_t i)
	{
		return HiSurrogateStart <= i && i <= HiSurrogateEnd;
	}
	
	inline bool utf16::isLowSurrogate(std::uint16_t i)
	{
		return LoSurrogateStart <= i && i <= LoSurrogateEnd;
	}
	
	inline char32_t utf16::toChar(std::uint16_t high, std::uint16_t low)
	{
		std::uint32_t n = (high & 0x003F) << 10 | (low & 0x03FF);
		std::uint32_t m = (high >> 6) & 0x001F;
		
		return (m + 1) << 16 | n;
	}
	
	inline std::uint16_t utf16::hiSurrogate(char32_t c)
	{
		std::uint16_t n = static_cast<std::uint16_t>(c);
		std::uint16_t m = static_cast<std::uint16_t>((c >> 16) & 0x1F) - 1;
		
		return HiSurrogateStart | (m << 6) | (n >> 10);
	}
	
	inline std::uint16_t utf16::loSurrogate(char32_t c)
	{
		return LoSurrogateStart | (c & 0x3FF);
	}
	
	// Compatibility Encoding Scheme for UTF-16: 8-Bit (CESU-8)
	// (http://www.unicode.org/reports/tr26/)
	template<typename OutputIterator> std::size_t utf16::cesu8Bytes(char32_t c, OutputIterator i)
	{
		if (c <= 0xFFFF)
			return utf8Bytes(c, i);
			
		utf8Bytes(hiSurrogate(c), i);
		utf8Bytes(loSurrogate(c), i);
		
		return 6;
	}
	
}

#endif /* N3_UTF16_HH */
