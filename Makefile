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


all: cturtle


cturtle: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)


obj/%.o: src/%.cc $(INCLUDES)
	@mkdir -p $(@D)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -std=c++11 -o $@ $<


src/$(LEXER_CC): src/Turtle.l
	$(LEX) $(LFLAGS) -o $@ $<


install: cturtle installdirs
	$(INSTALL_PROGRAM) cturtle $(DESTDIR)$(bindir)


uninstall:
	rm -f $(DESTDIR)$(bindir)/cturtle


installdirs:
	mkdir -p $(DESTDIR)$(bindir)


test: cturtle
	$(MAKE) -C test
	test/test-cturtle


clean:
	rm -f obj/*.o
	rm -f cturtle
	rm -f cturtle.tar.gz
	rm -f cturtle.zip
	$(MAKE) -C test clean


maintainer-clean: clean
	rm -f src/$(LEXER_CC)


distclean: clean


dist: tar


tar: cturtle.tar.gz


cturtle.tar.gz: $(SOURCES) src/Turtle.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d cturtle.XXXXXXXX -t))
	mkdir -p $(TMP)/cturtle/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/cturtle
	cp $(SOURCES) src/Turtle.l $(INCLUDES) $(TMP)/cturtle/src
	mkdir $(TMP)/cturtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/cturtle/test
	tar -C $(TMP) -czf $@ cturtle
	rm -rf $(TMP)


zip: cturtle.zip


cturtle.zip: $(SOURCES) src/Turtle.l $(INCLUDES) LICENSE README.md
	$(eval TMP := $(shell mktemp -d cturtle.XXXXXXXX -t))
	mkdir -p $(TMP)/cturtle/src
	cp $(MAKEFILE_LIST) LICENSE README.md $(TMP)/cturtle
	cp $(SOURCES) src/Turtle.l $(INCLUDES) $(TMP)/cturtle/src
	mkdir $(TMP)/cturtle/test
	cp test/*.cc test/*.hpp test/Makefile $(TMP)/cturtle/test
	cd $(TMP) && zip -r $@ cturtle
	cp $(TMP)/$@ .
	rm -rf $(TMP)
