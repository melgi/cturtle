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

.PHONY: all clean

SHELL=/bin/sh

CXXFLAGS=-Wall -march=native
SOURCES:=$(wildcard *.cc)
INCLUDES:=$(wildcard ../src/*.hh)
OBJECTS:= $(patsubst %.cc, %.o, $(SOURCES)) $(filter-out ../obj/Main.o, $(wildcard ../obj/*.o))

all: test-turtle

test-turtle: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cc $(INCLUDES)
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -std=c++11 -o $@ $<

clean:
	rm -f *.o
	rm -f test-turtle
