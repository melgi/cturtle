#
# Copyright 2016 Giovanni Mels
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

.PHONY: all install uninstall installdirs test clean tar zip 

SHELL=/bin/sh

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

CXXFLAGS=-O2 -Wall -march=native
#-march=x86-64
#-Wpedantic
LFLAGS=--warn

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644
LEXER_CC=TurtleLexer.cc
SOURCES:=$(filter-out $(LEXER_CC), $(wildcard *.cc))
OBJECTS:=$(patsubst %.cc, %.o, $(LEXER_CC) $(SOURCES))
INCLUDES:=$(wildcard *.hh)
#LIBS=-luriparser


all: turtle


turtle: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)


%.o: %.cc $(INCLUDES)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -std=c++11 -o $@ $<


$(LEXER_CC): Turtle.l
	$(LEX) $(LFLAGS) -o $@ $<


install: turtle installdirs
	$(INSTALL_PROGRAM) turtle $(DESTDIR)$(bindir)


uninstall:
	rm -f $(DESTDIR)$(bindir)/turtle


installdirs:
	mkdir -p $(DESTDIR)$(bindir)


test: turtle
	$(MAKE) -C test
	test/test-turtle


clean:
	rm -f *.o
	rm -f $(LEXER_CC)
	rm -f turtle
	rm -f turtle-src.tar.gz
	rm -f turtle-src.zip
	$(MAKE) -C test clean


tar: turtle-src.tar.gz


turtle-src.tar.gz: $(SOURCES) Turtle.l $(INCLUDES) LICENSE
	$(eval TMP := $(shell mktemp -d turtle.XXXXXXXX -t))
	mkdir $(TMP)/turtle
	cp $(SOURCES) Turtle.l $(INCLUDES) $(MAKEFILE_LIST) LICENSE $(TMP)/turtle
	mkdir $(TMP)/turtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/turtle/test
	tar -C $(TMP) -czf $@ turtle
	rm -rf $(TMP)


zip: turtle-src.zip


turtle-src.zip: $(SOURCES) Turtle.l $(INCLUDES) LICENSE
	$(eval TMP := $(shell mktemp -d turtle.XXXXXXXX -t))
	mkdir $(TMP)/turtle
	cp $(SOURCES) Turtle.l $(INCLUDES) $(MAKEFILE_LIST) LICENSE $(TMP)/turtle
	mkdir $(TMP)/turtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/turtle/test
	cd $(TMP) && zip -r $@ turtle
	cp $(TMP)/$@ .
	rm -rf $(TMP)
