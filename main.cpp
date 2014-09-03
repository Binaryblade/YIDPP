#include "parser.h"
#include <iostream>

using namespace yidpp;

int main(void) {
	class Tree {
		public:
			Tree() : Left(nullptr), Right(nullptr) {};
			Tree* Left;
			Tree* Right;
	};

	std::shared_ptr<Parser<char,char>> OpenBrace = std::make_shared<EqT<char>>('(');
	std::shared_ptr<Parser<char,char>> CloseBrace = std::make_shared<EqT<char>>(')');
	std::set<Tree*> blankSet;
	blankSet.insert(nullptr);
	std::shared_ptr<Parser<char,Tree*>> EmptyTree = std::make_shared<Eps<char,Tree*>>(blankSet);
	std::shared_ptr<RecursiveParser<char,Tree*>> language = std::make_shared<RecursiveParser<char,Tree*>>();
	
	std::shared_ptr<Parser<char,std::pair<char,Tree*>>> LeftPair = std::make_shared<Con<char,char,Tree*>>(OpenBrace,language);
	std::shared_ptr<Parser<char,std::pair<char,Tree*>>> RightPair = std::make_shared<Con<char,char,Tree*>>(CloseBrace,language);

	std::shared_ptr<Parser<char,Tree*>> LeftReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(LeftPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});
	std::shared_ptr<Parser<char,Tree*>> RightReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(RightPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});

	std::shared_ptr<Parser<char,std::pair<Tree*,Tree*>>> parsePair = std::make_shared<Con<char,Tree*,Tree*>>(LeftReduce,RightReduce);
	std::shared_ptr<Parser<char,Tree*>> ReducedPair = std::make_shared<Red<char,std::pair<Tree*,Tree*>,Tree*>>(parsePair,[](std::pair<Tree*,Tree*> input) -> Tree* {Tree* temp = new Tree(); temp->Left = input.first; temp->Right = input.second; return temp;});
	language->SetRecurse(std::make_shared<Alt<char,Tree*>>(ReducedPair,EmptyTree));

	std::vector<char> inputStream;
	std::set<Tree*> result = language->parseFull(inputStream);
	std::cout << result.size() << std::endl;
	return 0;
}
