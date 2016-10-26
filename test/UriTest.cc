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

#include "../src/Uri.hh"
#include "catch.hpp"


std::string resolve(const turtle::Uri &base, const std::string &reference);
void print(std::ostream &out, const turtle::Uri &uri);


TEST_CASE("resolve", "[uri]") {
	turtle::Uri base("http://a/b/c/d;p?q");
	
	SECTION("normal") {
		REQUIRE("g:h" == resolve(base, "g:h"));
		REQUIRE("http://a/b/c/g" == resolve(base, "g"));
		REQUIRE("http://a/b/c/g" == resolve(base, "./g"));
		REQUIRE("http://a/b/c/g/" == resolve(base, "g/"));
		REQUIRE("http://a/g" == resolve(base, "/g"));
		REQUIRE("http://g" == resolve(base, "//g"));
		REQUIRE("http://a/b/c/d;p?y" == resolve(base, "?y"));
		REQUIRE("http://a/b/c/g?y" == resolve(base, "g?y"));
		REQUIRE("http://a/b/c/d;p?q#s" == resolve(base, "#s"));
		REQUIRE("http://a/b/c/g#s" == resolve(base, "g#s"));
		REQUIRE("http://a/b/c/g?y#s" == resolve(base, "g?y#s"));
		REQUIRE("http://a/b/c/;x" == resolve(base, ";x"));
		REQUIRE("http://a/b/c/g;x" == resolve(base, "g;x"));
		REQUIRE("http://a/b/c/g;x?y#s" == resolve(base, "g;x?y#s"));
		REQUIRE("http://a/b/c/d;p?q" == resolve(base, ""));
		REQUIRE("http://a/b/c/" == resolve(base, "."));
		REQUIRE("http://a/b/c/" == resolve(base, "./"));
		REQUIRE("http://a/b/" == resolve(base, ".."));
		REQUIRE("http://a/b/" == resolve(base, "../"));
		REQUIRE("http://a/b/g" == resolve(base, "../g"));
		REQUIRE("http://a/" == resolve(base, "../.."));
		REQUIRE("http://a/" == resolve(base, "../../"));
		REQUIRE("http://a/g" == resolve( base, "../../g"));
	}
	
	SECTION("abnormal") {
		REQUIRE("http://a/g" == resolve(base, "../../../g"));
		REQUIRE("http://a/g" == resolve(base, "../../../../g"));
	
		REQUIRE("http://a/g" == resolve(base, "/./g"));
		REQUIRE("http://a/g" == resolve(base, "/../g"));
		REQUIRE("http://a/b/c/g." == resolve(base, "g."));
		REQUIRE("http://a/b/c/.g" == resolve(base, ".g"));
		REQUIRE("http://a/b/c/g.." == resolve(base, "g.."));
		REQUIRE("http://a/b/c/..g" == resolve(base, "..g"));
		
		REQUIRE("http://a/b/g" == resolve(base, "./../g"));
		REQUIRE("http://a/b/c/g/" == resolve(base, "./g/."));
		REQUIRE("http://a/b/c/g/h" == resolve(base, "g/./h"));
		REQUIRE("http://a/b/c/h" == resolve(base, "g/../h"));
		REQUIRE("http://a/b/c/g;x=1/y" == resolve(base, "g;x=1/./y"));
		REQUIRE("http://a/b/c/y" == resolve(base, "g;x=1/../y"));
		
		REQUIRE("http://a/b/c/g?y/./x" == resolve(base, "g?y/./x"));
		REQUIRE("http://a/b/c/g?y/../x" == resolve(base, "g?y/../x"));
		REQUIRE("http://a/b/c/g#s/./x" == resolve(base, "g#s/./x"));
		REQUIRE("http://a/b/c/g#s/../x" == resolve(base, "g#s/../x"));
		
		REQUIRE("http:g" == resolve(base, "http:g"));
	}
}

TEST_CASE("absolute", "[uri]") {
	REQUIRE(turtle::Uri::absolute("g:h"));
	REQUIRE_FALSE(turtle::Uri::absolute(":"));
	REQUIRE_FALSE(turtle::Uri::absolute("#:"));
	REQUIRE_FALSE(turtle::Uri::absolute("g?y/./x"));
	REQUIRE_FALSE(turtle::Uri::absolute("foo"));
}

TEST_CASE("parse", "[uri]") {
	turtle::Uri uri("http://user@www.ics.uci.edu:8080/pub/ietf/uri/#Related");
	
	REQUIRE(uri.absolute());
	REQUIRE(uri.scheme()); REQUIRE(*uri.scheme() == "http");
	REQUIRE(uri.authority()); REQUIRE(*uri.authority() == "user@www.ics.uci.edu:8080");
	REQUIRE(uri.userInfo()); REQUIRE(*uri.userInfo() == "user");
	REQUIRE(uri.host()); REQUIRE(*uri.host() == "www.ics.uci.edu");
	REQUIRE(uri.port()); REQUIRE(*uri.port() == "8080");
	REQUIRE(uri.path() == "/pub/ietf/uri/");
	REQUIRE_FALSE(uri.query());
	REQUIRE(uri.fragment()); REQUIRE(*uri.fragment() == "Related");
}

TEST_CASE("empty path 1", "[uri]") {
	turtle::Uri uri("http://example.org#");
	
	REQUIRE(uri.absolute());
	REQUIRE(uri.scheme()); REQUIRE(*uri.scheme() == "http");
	REQUIRE(uri.authority()); REQUIRE(*uri.authority() == "example.org");
	REQUIRE_FALSE(uri.userInfo());
	REQUIRE(uri.host()); REQUIRE(*uri.host() == "example.org");
	REQUIRE_FALSE(uri.port());
	REQUIRE(uri.path() == "");
	REQUIRE_FALSE(uri.query());
	REQUIRE(uri.fragment()); REQUIRE(*uri.fragment() == "");
}

TEST_CASE("empty path 2", "[uri]") {
	turtle::Uri uri("http://example.org?");
	
	REQUIRE(uri.absolute());
	REQUIRE(uri.scheme()); REQUIRE(*uri.scheme() == "http");
	REQUIRE(uri.authority()); REQUIRE(*uri.authority() == "example.org");
	REQUIRE_FALSE(uri.userInfo());
	REQUIRE(uri.host()); REQUIRE(*uri.host() == "example.org");
	REQUIRE_FALSE(uri.port());
	REQUIRE(uri.path() == "");
	REQUIRE(uri.query()); REQUIRE(*uri.query() == "");
	REQUIRE_FALSE(uri.fragment());
}




std::string resolve(const turtle::Uri &base, const std::string &reference)
{
	turtle::Uri r(reference);
	
	return static_cast<std::string>(base.resolve(r));
}

void print(std::ostream &out, const turtle::Uri &uri)
{
	out << "scheme: " << (uri.scheme() ? "\"" + *uri.scheme() + "\"" : "<undefined>") << std::endl;
	out << "authority: " << (uri.authority() ? "\"" + *uri.authority() + "\"" : "<undefined>") << std::endl;
	out << "userInfo: " << (uri.userInfo() ? "\"" + *uri.userInfo() + "\"" : "<undefined>") << std::endl;
	out << "host: " << (uri.host() ? "\"" + *uri.host() + "\"" : "<undefined>") << std::endl;
	out << "port: " << (uri.port() ? "\"" + *uri.port() + "\"" : "<undefined>") << std::endl;
	out << "path: \"" << uri.path() << '"' << std::endl;
	out << "query: " << (uri.query() ? "\"" + *uri.query() + "\"" : "<undefined>") << std::endl;
	out << "fragment: " << (uri.fragment() ? "\"" + *uri.fragment() + "\"" : "<undefined>") << std::endl;
}
