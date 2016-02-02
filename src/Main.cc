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
#include <fstream>
#include <memory>
#include <chrono>
#include <iomanip>

#include "CommandLine.hh"
#include "Parser.hh"
#include "Uri.hh"
#include "NTriplesWriter.hh"
#include "N3PWriter.hh"
#include "Util.hh"


int main(int argc, char *argv[])
{
	std::cin.sync_with_stdio(false);
	std::cout.sync_with_stdio(false);
	
	char outputBuffer[1024*1024];
	std::cout.rdbuf()->pubsetbuf(outputBuffer, sizeof(outputBuffer));
	
//	char inputBuffer[1024*1024];
//	std::cin.rdbuf()->pubsetbuf(inputBuffer, sizeof(inputBuffer));
	
	std::cin.tie(nullptr);
	
	turtle::CommandLine opt = turtle::CommandLine::parse(argc, argv);
	
	if (opt.error || opt.help) {
		std::cerr << argv[0] << " [-b=base-uri] [-o=output-file] [-f=(nt|n3p|n3p-rdiv)] [input-files] " << std::endl;
		return -1;
	}
	
	std::unique_ptr<std::ostream> out;
	if (opt.output && *opt.output != "-") {
		out = std::unique_ptr<std::ostream>(new std::ofstream(*opt.output));
		
		if (!*out) {
			std::cerr << "error opening \"" << *opt.output << "\"" << std::endl;
			
			return -1;
		}
	}
	
	turtle::TripleSink *s;
	if (opt.format == turtle::CommandLine::N3P)
		s = new turtle::N3PWriter(out ? *out : std::cout);
	else if (opt.format == turtle::CommandLine::N3P_RDIV)
		s = new turtle::N3PWriter(out ? *out : std::cout, true);
	else
		s = new turtle::NTriplesWriter(out ? *out : std::cout);
		
	std::unique_ptr<turtle::TripleSink> sink(s);
	
	typedef std::chrono::high_resolution_clock Clock;
	
	Clock::time_point start = Clock::now();
	
	sink->start();
	
	for (std::string input : opt.inputs) {
		
		std::string uri;
		
		std::unique_ptr<std::ifstream> in;
		if (input != "-") {
			if (!turtle::exists(input)) {
				std::cerr << "\"" << input << "\" not found" << std::endl;
				sink->end();
				
				return -1;
			}
			
			uri = turtle::toUri(input);
			in = std::unique_ptr<std::ifstream>(new std::ifstream(input, std::ios_base::in | std::ios_base::binary));
			if (!*in) {
				std::cerr << "error opening \"" << input << "\"" << std::endl;
				sink->end();
				
				return -1;
			}
		} else {
			uri = "file:///dev/stdin";
		}
		
		std::cerr << "translating " << uri << std::endl;
		
		turtle::Uri baseUri = turtle::Uri::parse(opt.base ? *opt.base : uri);
		
		turtle::Parser parser(in ? in.get() : &std::cin, baseUri, sink.get());
		try {
			parser.parse();
		} catch (turtle::ParseException &e) {
			if (e.line() == -1)
				std::cerr << "parse error: " << e.what() << std::endl;
			else
				std::cerr << "parse error at line " << e.line() << ": " << e.what() << std::endl;
			
			return -1;
		}
	}
	
	sink->end();
	
	Clock::duration d = Clock::now() - start;
	
	double ms = static_cast<double>(1000 * d.count() * Clock::duration::period::num) / static_cast<double>(Clock::duration::period::den);
	
	unsigned count = sink->count();
	
	if (count && ms > 0.0) {
		std::streamsize p = std::cerr.precision();
		std::cerr << "Done: translated " << count << " triples in " << std::fixed << std::setprecision(1) << ms << std::setprecision(0) << " ms (" << (1000.0 * count / ms) << " triples/s)" << std::setprecision(p) <<  std::endl;
	} else
		std::cerr << "Done: translated " << count << " triples" << std::endl;
		
	return 0;

}
