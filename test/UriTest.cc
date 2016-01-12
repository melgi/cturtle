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

#include "../Uri.hh"
#include "catch.hpp"


std::string resolve(const turtle::Uri &base, const std::string &reference);
void print(std::ostream &out, const turtle::Uri &uri);


TEST_CASE("resolve", "[uri]") {
	turtle::Uri base = turtle::Uri::parse("http://a/b/c/d;p?q");
	
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

TEST_CASE("parse", "[uri]") {
	turtle::Uri uri = turtle::Uri::parse("http://user@www.ics.uci.edu:8080/pub/ietf/uri/#Related");
	
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

std::string resolve(const turtle::Uri &base, const std::string &reference)
{
	turtle::Uri r = turtle::Uri::parse(reference);
	
	return std::string(base.resolve(r));
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

/*
#include <cwchar>
#include <clocale>

TEST_CASE("test utf8") {
	
	//std::string input = "test \\u20AC 100."
	
	wchar_t euro = 0x20AC;
	
	std::setlocale(LC_ALL, "en_US.utf-8");
	
	std::mbstate_t state {};
	
	std::string mb(MB_CUR_MAX, '\0');
	
	int result = std::wcrtomb(&mb[0], euro, &state);
	
	std::cout << result << " " << mb << std::endl;	
}
*/