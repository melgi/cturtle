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

#include "BlankNodeIdGenerator.hh"

#include <random>
#include <chrono>
#include <cstdint>

namespace turtle {
	
	void BlankNodeIdGenerator::initialize()
	{
		typedef std::chrono::high_resolution_clock Clock;
		
		std::default_random_engine generator;
		
		constexpr int max = 'Z' - 'A' + '9' - '0' - 1; 
		std::uniform_int_distribution<int> distribution(0, max);
		
		Clock::time_point t = Clock::now();
		Clock::duration d = t.time_since_epoch();
		generator.seed(d.count() + reinterpret_cast<std::uintptr_t>(this));
		
		m_prefix.clear();
		m_prefix.reserve(m_length);
		
		for (std::size_t i = 0; i < m_length; i++) {
			int n = distribution(generator);  // generates number in the range 0..35
		
			if (n < 10)
				m_prefix.push_back(n + '0');
			else
				m_prefix.push_back(n - 10 + 'A');
		}
		
		m_c = 0;
	}

}
