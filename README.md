# CTurtle

CTurtle is a tool for parsing [RDF 1.1 Turtle](http://www.w3.org/TR/turtle/) files and outputting the resulting triples in "N3P" format.
"N3P" is a RDF serialization format used by the [Eye reasoner](http://github.com/josd/eye).

## Build status

[![Build Status](https://travis-ci.org/melgi/cturtle.svg?branch=master)](https://travis-ci.org/melgi/cturtle)

## Usage

`cturtle [-b=base-uri] [-o=output-file] [-f=(nt|n3p|n3p-rdiv)] [input-files]`

* `-b=baseUri` the base URI to use when resolving relative URIs.
* `-o=output-file` where the results are written, write to stdout when omitted.
* `-f=nt` (default) output triples in [N-Triples](http://www.w3.org/TR/n-triples/) format.
* `-f=n3p` output triples in N3P format.
* `-f=n3p-rdiv` output triples in N3P format, use `rdiv` to output decimals.
* `input-files` the Turtle input files to process, read from stdin when omitted.

## Limitations

* The parser only does basic validation of IRIs and literals, e.g. `<http://localhost:abc> :value "abc"^^xsd:integer.` will pass as a valid triple.

## Integration with Eye

The latest version of Eye (since EYE-Winter16) will automatically use CTurtle when using the `--turtle` option to specify Turtle input files.
