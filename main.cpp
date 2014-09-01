#include "parser.h"
#include <iostream>

using namespace yidpp;

int main(void) {
	std::shared_ptr<Parser<char,char>> singleParser = std::make_shared<EqT<char>>('x');
	std::shared_ptr<Parser<char,std::vector<char>>> kleene = std::make_shared<Rep<char,char>>(singleParser);
	std::shared_ptr<Parser<char,std::vector<char>>> recursive = kleene;
	for(size_t i=0;i<10;++i) {
		if(!recursive->isNullable()) {
		 std::cout << "Result is Not Nullable" << std::endl;
		 std::cout << "At index: " << i << std::endl;
		 break;
		}
		recursive = recursive->derive('x');
	}
	std::set<std::vector<char>> result = recursive->parseNull();
	std::cout << "Set Size: " << result.size() << std::endl;
	for(auto j=result.begin();j!=result.end();++j) {
			std::cout << "Result of Length: " << j->size() << std::endl;
			std::cout << "\"";
			for(auto k=j->begin();k!=j->end();++k) {
				std::cout << *k;
			}
			std::cout << "\"" << std::endl;
	}
	return 0;
}
