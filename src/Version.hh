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

#ifndef CTURTLE_VERSION_HH
#define CTURTLE_VERSION_HH


#define CTURTLE_MAJOR_VERSION     1
#define CTURTLE_MINOR_VERSION     0
#define CTURTLE_PATCH_VERSION     3
//#define CTURTLE_QUALIFIER_VERSION "SNAPSHOT"



#define CTURTLE_STR(x) #x

#ifdef CTURTLE_QUALIFIER_VERSION
#	define CTURTLE_VERSION(MAJ, MIN, PATCH) CTURTLE_STR(MAJ) "." CTURTLE_STR(MIN) "." CTURTLE_STR(PATCH) "-" CTURTLE_QUALIFIER_VERSION
#else
#	define CTURTLE_VERSION(MAJ, MIN, PATCH) CTURTLE_STR(MAJ) "." CTURTLE_STR(MIN) "." CTURTLE_STR(PATCH)
#endif

#define CTURTLE_VERSION_STR       CTURTLE_VERSION(CTURTLE_MAJOR_VERSION, CTURTLE_MINOR_VERSION, CTURTLE_PATCH_VERSION)

#endif /* CTURTLE_VERSION_HH */
