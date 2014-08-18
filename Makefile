SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst %.cpp,%.d,$(SOURCES))

override CXXFLAGS := -g -std=c++11 -Wall $(CXXFLAGS)
override LDFLAGS := -std=c++11 $(LDFLAGS)
override LIBS := $(LIBS)

parsertest: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $+ $(LIBS)

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.d:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -MM $^

.PHONY: clean
clean:
	$(RM) *.d
	$(RM) *.o 
	$(RM) parsertest
ifneq ($(MAKECMDGOALS),clean)
include $(DEPENDS) 
endif
