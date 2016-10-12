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
#include <climits>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

#include <unistd.h>

#ifdef _WIN32
#	include <io.h>     // _setmode
#	include <fcntl.h>  // _O_BINARY
#endif


namespace turtle {

	void useBinaryStreams()
	{
#ifdef _WIN32
		_setmode(_fileno(stdin), _O_BINARY);
		_setmode(_fileno(stdout), _O_BINARY);
#endif
	}

	// TODO this behaves different on Windows and Linux when "file" does not exist
	//(Windows: OK, Linux: fail)
	// Also PATH_MAX is only 255 on Windows
	// Use _getcwd (Windows) getcwd (Linux) in stead?
	std::string toUri(const std::string &file)
	{
		char buf[PATH_MAX]; 
#ifdef _WIN32
		const char *rp = ::_fullpath(buf, file.c_str(), PATH_MAX);
		
		if (!rp)
			throw std::runtime_error("could not determine the absolute path for " + file);
		
		std::string absPath = std::string(rp);
		
		std::replace(absPath.begin(), absPath.end(), '\\', '/');
		
		return "file:///" + absPath;
#else
		const char *rp = ::realpath(file.c_str(), buf);
		
		if (!rp)
			throw std::runtime_error("could not determine the absolute path for " + file);
			
		return "file://" + std::string(rp);
#endif // _WIN32
	}
	
	bool exists(const std::string &fileName)
	{
		return access(fileName.c_str(), F_OK) == 0;
	}
	
}
