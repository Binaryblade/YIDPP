#include "parser.h"
#include <iostream>

using namespace yidpp;

int main(void) {
	class Tree {
		public:
			Tree() : Left(nullptr), Right(nullptr) {};
			Tree(Tree* Left,Tree* Right) : Left(Left), Right(Right) {};
			Tree* Left;
			Tree* Right;
	};


	//Constructing the Language 
	//L = (L)L U e
	//parser for (
	auto OpenBrace = std::make_shared<EqT<char>>('(');
	//parser for )
	auto CloseBrace = std::make_shared<EqT<char>>(')');

	//parser for empty string (returns null tree)
	std::set<Tree*> blankSet;
	blankSet.insert(nullptr);
	auto EmptyTree = std::make_shared<Eps<char,Tree*>>(blankSet);
	
	//parser for L, needs its child to be set later
	auto language = std::make_shared<RecursiveParser<char,Tree*>>();
	
	//parser for (L
	auto LeftPair = std::make_shared<Con<char,char,Tree*>>(OpenBrace,language);

	//parser for )L
	auto RightPair = std::make_shared<Con<char,char,Tree*>>(CloseBrace,language);

	
	//parser for A = (L -> Tree
	auto LeftReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(LeftPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});

	//parser for B = )L -> Tree
	auto RightReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(RightPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});

	
	//parser for AB
	auto parsePair = std::make_shared<Con<char,Tree*,Tree*>>(LeftReduce,RightReduce);
	
	
	//parser for AB -> C
	auto ReducedPair = std::make_shared<Red<char,std::pair<Tree*,Tree*>,Tree*>>(parsePair,[](std::pair<Tree*,Tree*> input) -> Tree* {return new Tree(input.first,input.second);});
	
	//set L to be C U e
	language->SetRecurse(std::make_shared<Alt<char,Tree*>>(ReducedPair,EmptyTree));

	std::shared_ptr<Parser<char,Tree*>> torecurse = language;
	for(int i=0; i< 25; i++) {
		torecurse = torecurse->derive('(');
		std::stringstream s;	
		s << "numderives_"<<i;
		std::cout << getGraph<char,Tree*>(s.str(),torecurse) << std::endl;
	}
	

		torecurse = torecurse->derive(')');
		std::cout << getGraph<char,Tree*>("FirstClosure",torecurse) << std::endl;
	return 0;
}
