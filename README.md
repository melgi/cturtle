# Turtle

Turtle is a tool for parsing [RDF 1.1 Turtle](http://www.w3.org/TR/turtle/) files and outputting the resulting triples in "N3P" format.
"N3P" is a RDF serialization format used by the [Eye reasoner](http://eulersharp.sourceforge.net/).

# Usage

`turtle [-b=base-uri] [-o=output-file] [-f=(nt|n3p)] [input-files]`

* `-b=baseUri` the base URI to use when resolving relative URIs.
* `-o=output-file` where the results are written, write to stdout when omitted.
* `-f=nt` output triples in [N-Triples](http://www.w3.org/TR/n-triples/) format.
* `-f=n3p` (default) output triples in N3P format.
* `input-files` the turtle input files to process, read from stdin when omitted.

# Limitations

* The parser only does basic validation of IRIs and literals, e.g. `<http://localhost:abc> :value "abc"^^xsd:integer.` will pass as a valid triple.
