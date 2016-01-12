/*
 * Copyright 2016 Giovanni Mels
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

%top {

#include "Token.hh"

}

%option 8bit
%option c++
%option noyywrap
%option batch
%option yylineno
%option nodefault
/*%option prefix="Turtle"*/
/*%option yyclass="N3Lexer"*/


DIGIT            [0-9]

UTF8CONT         [\x80-\xBF]
TWOBYTECHAR      \xC3[\x80-\x96\x98-\xB6\xB8-\xBF]|[\xC4-\xCB]{UTF8CONT}|\xCD[\xB0-\xBD\xBF]|[\xCE-\xDF]{UTF8CONT}
THREEBYTECHAR    \xE0[\xA0-\xBF]{UTF8CONT}|\xE1{UTF8CONT}{UTF8CONT}|\xE2\x80[\x8C-\x8D]|\xE2[\x81-\x85]{UTF8CONT}|\xE2\x86[\x80-\x8F]|\xE2[\xB0-\xBE]{UTF8CONT}|\xE2\xBF[\x80-\xAF]|[\xE3-\xEC]{UTF8CONT}{UTF8CONT}|\xED[\x80-\x9F]{UTF8CONT}|\xEF[\xA4-\xB6]{UTF8CONT}|\xEF\xB7[\x80-\x8F]|\xEF[\xB7-\xBE]{UTF8CONT}|\xEF\xBF[\x80-\xBD]
FOURBYTECHAR     \xF0[\x90-\xBF]{UTF8CONT}{UTF8CONT}|[\xF1-\xF2]{UTF8CONT}{UTF8CONT}{UTF8CONT}|\xF3[\x80-\xAF]{UTF8CONT}{UTF8CONT}

UCHAR            "\\u"{HEX}{HEX}{HEX}{HEX}|"\\U"{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}
ECHAR            "\\"[tbnrf"'\\]
WS               \x20|\x09|\x0D|\x0A
PN_CHARS_BASE    [a-zA-Z]|{TWOBYTECHAR}|{THREEBYTECHAR}|{FOURBYTECHAR}
PN_CHARS_U       {PN_CHARS_BASE}|"_"
PN_CHARS         {PN_CHARS_U}|"-"|{DIGIT}|\xC2\xB7|\xCC{UTF8CONT}|\xCD[\x80-\xAF]|\xE2\x80\xBF|\xE2\x81\x80
PN_PREFIX        {PN_CHARS_BASE}(({PN_CHARS}|".")*{PN_CHARS})?
PN_LOCAL         ({PN_CHARS_U}|":"|{DIGIT}|{PLX})(({PN_CHARS}|"."|":"|{PLX})*({PN_CHARS}|":"|{PLX}))?
PLX              {PERCENT}|{PN_LOCAL_ESC}
PERCENT          "%"{HEX}{HEX}
HEX              {DIGIT}|[a-fA-F]
PN_LOCAL_ESC     "\\"[_~.\-!$&'()*+,;=/?#@%]
SIGN             [-+]
EXPONENT         [eE]{SIGN}?{DIGIT}+

%%

@prefix                                                                         { return turtle::Token::Prefix; }
@base                                                                           { return turtle::Token::Base; }
"<"([^\x00-\x20<>"{}|^`\\]|{UCHAR})*">"                                         { return turtle::Token::IriRef; }
{PN_PREFIX}?":"                                                                 { return turtle::Token::PNameNS; }
{PN_PREFIX}?":"{PN_LOCAL}                                                       { return turtle::Token::PNameLN; }
"_:"({PN_CHARS_U}|{DIGIT})(({PN_CHARS}|".")*{PN_CHARS})?                        { return turtle::Token::BlankNodeLabel; }
"@"[a-zA-Z]+("-"[a-zA-Z0-9]+)*                                                  { return turtle::Token::LangTag; }
{SIGN}?{DIGIT}+                                                                 { return turtle::Token::Integer; }
{SIGN}?{DIGIT}*"."{DIGIT}+                                                      { return turtle::Token::Decimal; }
{SIGN}?({DIGIT}+"."{DIGIT}*{EXPONENT}|"."{DIGIT}+{EXPONENT}|{DIGIT}+{EXPONENT}) { return turtle::Token::Double; }
"\""([^\x22\x5C\x0A\x0D]|{ECHAR}|{UCHAR})*"\""                                  { return turtle::Token::StringLiteralQuote; }
"'"([^\x27\x5C\x0A\x0D]|{ECHAR}|{UCHAR})*"'"                                    { return turtle::Token::StringLiteralSingleQuote; }
"'''"(("'"|"''")?([^'\\]|{ECHAR}|{UCHAR}))*"'''"                                { return turtle::Token::StringLiteralLongSingleQuote; }
"\"\"\""(("\""|"\"\"")?([^"\\]|{ECHAR}|{UCHAR}))*"\"\"\""                       { return turtle::Token::StringLiteralLongQuote; }
{WS}+                                                                           
"#".*
false                                                                           { return turtle::Token::False; }
true                                                                            { return turtle::Token::True; }
(?i:prefix)                                                                     { return turtle::Token::SparqlPrefix; }
(?i:base)                                                                       { return turtle::Token::SparqlBase; }
"^^"                                                                            { return turtle::Token::CaretCaret; }
.                                                                               { return yytext[0]; } /* [.;,()[\]a] */

%%

