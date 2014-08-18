YIDPP
=====

Yacc is Dead Plus Plus, A C++ parser combinator library

Based on the SCALA implementation from parsing with derivatives. The code is a implementation of the paper "YACC is dead" written by Matthew Might and David Darais. Further reading is "Parsing with Derivatives, a functional derivative" by Matthew Might, David Darais and Daniel Spiewak.

This library allows one to build up a parser from within the C++ language without having to resort to additional compiled tools such as YACC. Estimated average complexity is approximately linear with appropriate optimizations and it is guessed that the worst case complexity is O(N^3) but this is yet to be proven for a given implementation.


