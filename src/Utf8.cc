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


#include "Utf8.hh"

namespace turtle {
	
	namespace utf8 {
	
		// This table maps bytes to character classes
		// to reduce the size of the transition table and create bitmasks.
		alignas(256) const std::uint8_t Decoder::m_type[] = {
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x00 - 0x1F
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x20 - 0x3F
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x40 - 0x5F
			 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x60 - 0x7F
			 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 0x80 - 0x9F
			 7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // 0xA0 - 0xBF
			 8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xC0 - 0xDF
			10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8  // 0xE0 - 0xFF
		};
		
		// This table is a transition table that maps a combination
		// of a state of the automaton and a character class to a state.
		alignas(256) const std::uint8_t Decoder::m_transition[] = {
			 0,12,24,36,60,96,84,12,12,12,48,72, // state  0 (ACCEPT)
			12,12,12,12,12,12,12,12,12,12,12,12, // state 12 (REJECT)
			12, 0,12,12,12,12,12, 0,12, 0,12,12, // state 24
			12,24,12,12,12,12,12,24,12,24,12,12, // state 36
			12,12,12,12,12,12,12,24,12,12,12,12, // state 48
			12,24,12,12,12,12,12,12,12,24,12,12, // state 60
			12,12,12,12,12,12,12,36,12,36,12,12, // state 72
			12,36,12,12,12,12,12,36,12,36,12,12, // state 84
			12,36,12,12,12,12,12,12,12,12,12,12  // state 96
		};
	}
	
}
