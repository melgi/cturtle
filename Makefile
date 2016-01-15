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

.PHONY: all install uninstall installdirs test clean maintainer-clean distclean dist tar zip 

SHELL=/bin/sh

prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

CXXFLAGS=-O2 -Wall -march=native
LFLAGS=--warn

INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644
LEXER_CC=TurtleLexer.cc
SOURCES:=src/$(LEXER_CC) $(filter-out src/$(LEXER_CC), $(wildcard src/*.cc))
OBJECTS:=$(patsubst src/%.cc, obj/%.o, $(SOURCES))
INCLUDES:=$(wildcard src/*.hh)


all: turtle


turtle: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)


obj/%.o: src/%.cc $(INCLUDES)
	@mkdir -p $(@D)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -std=c++11 -o $@ $<


src/$(LEXER_CC): src/Turtle.l
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
	rm -f obj/*.o
	rm -f turtle
	rm -f turtle.tar.gz
	rm -f turtle.zip
	$(MAKE) -C test clean


maintainer-clean: clean
	rm -f src/$(LEXER_CC)


distclean: clean


dist: tar


tar: turtle.tar.gz


turtle.tar.gz: $(SOURCES) src/Turtle.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d turtle.XXXXXXXX -t))
	mkdir -p $(TMP)/turtle/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/turtle
	cp $(SOURCES) src/Turtle.l $(INCLUDES) $(TMP)/turtle/src
	mkdir $(TMP)/turtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/turtle/test
	tar -C $(TMP) -czf $@ turtle
	rm -rf $(TMP)


zip: turtle.zip


turtle.zip: $(SOURCES) src/Turtle.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d turtle.XXXXXXXX -t))
	mkdir -p $(TMP)/turtle/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/turtle
	cp $(SOURCES) src/Turtle.l $(INCLUDES) $(TMP)/turtle/src
	mkdir $(TMP)/turtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/turtle/test
	cd $(TMP) && zip -r $@ turtle
	cp $(TMP)/$@ .
	rm -rf $(TMP)
