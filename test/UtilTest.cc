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

#include <string>
#include <ostream>
#include <iostream>

#include "../src/Util.hh"
#include "catch.hpp"

// This test must be executed from the project directory
TEST_CASE("exist", "[file]") {
	REQUIRE(turtle::exists("test/TestMain.cc"));
	REQUIRE(turtle::exists("test/../test/TestMain.cc"));
	REQUIRE(!turtle::exists("foo"));
}


// This test must be executed from the project directory
//TEST_CASE("toUri", "[file]") {
//	std::cout << turtle::toUri("test/TestMain.cc")         << std::endl;
//	std::cout << turtle::toUri("test/../test/TestMain.cc") << std::endl;
//	std::cout << turtle::toUri("foo")                      << std::endl;
//}

