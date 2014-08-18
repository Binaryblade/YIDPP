LDFLAGS := --std=gnu++0x -g $(LDFLAGS)
CXXFLAGS := --std=gnu++0x -g  $(CXXFLAGS)

all: parsertest

parsertest: main.o special_parser.o
	g++ $(LDFLAGS) $(LIBS) $+ -o $@

main.o: main.cpp special_parser.h
	g++ $(CXXFLAGS) -c $< -o $@

special_parser.h: parser.h

special_parser.o: special_parser.cpp special_parser.h
	g++ $(CXXFLAGS) -c $< -o $@

.PHONY clean:
	rm -fr main.o
	rm -fr parsertest
