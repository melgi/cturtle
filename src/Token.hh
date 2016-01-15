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

#ifndef N3_TOKEN_HH
#define N3_TOKEN_HH

/*
#define IRIREF                           1000
#define PNAME_NS                         1001
#define PNAME_LN                         1002
#define BLANK_NODE_LABEL                 1003
#define LANGTAG                          1004
#define INTEGER                          1005
#define DECIMAL                          1006
#define DOUBLE                           1007
#define STRING_LITERAL_QUOTE             1008
#define STRING_LITERAL_SINGLE_QUOTE      1009
#define STRING_LITERAL_LONG_SINGLE_QUOTE 1010
#define STRING_LITERAL_LONG_QUOTE        1011
#define FALSE                            1012
#define TRUE                             1013
#define PREFIX                           1014
#define BASE                             1015
#define SPARQL_PREFIX                    1016
#define SPARQL_BASE                      1017
#define CARETCARET                       1018
*/

namespace turtle {

	struct Token {
		
		typedef int Type;
		
		static const Type Eof                          = 0;
		
		static const Type IriRef                       = 1000;
		static const Type PNameNS                      = 1001;
		static const Type PNameLN                      = 1002;
		static const Type BlankNodeLabel               = 1003;
		static const Type LangTag                      = 1004;
		static const Type Integer                      = 1005;
		static const Type Decimal                      = 1006;
		static const Type Double                       = 1007;
		static const Type StringLiteralQuote           = 1008;
		static const Type StringLiteralSingleQuote     = 1009;
		static const Type StringLiteralLongSingleQuote = 1010;
		static const Type StringLiteralLongQuote       = 1011;
		static const Type False                        = 1012;
		static const Type True                         = 1013;
		static const Type Prefix                       = 1014;
		static const Type Base                         = 1015;
		static const Type SparqlPrefix                 = 1016;
		static const Type SparqlBase                   = 1017;
		static const Type CaretCaret                   = 1018;
		
	};
	
}

#endif /* N3_TOKEN_HH */
