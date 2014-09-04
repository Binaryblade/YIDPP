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


	//Constructing the Language 
	//L = (L)L U e
	//parser for (
	std::shared_ptr<Parser<char,char>> OpenBrace = std::make_shared<EqT<char>>('(');
	//parser for )
	std::shared_ptr<Parser<char,char>> CloseBrace = std::make_shared<EqT<char>>(')');

	//parser for empty string (returns null tree)
	std::set<Tree*> blankSet;
	blankSet.insert(nullptr);
	std::shared_ptr<Parser<char,Tree*>> EmptyTree = std::make_shared<Eps<char,Tree*>>(blankSet);
	
	//parser for L, needs its child to be set later
	std::shared_ptr<RecursiveParser<char,Tree*>> language = std::make_shared<RecursiveParser<char,Tree*>>();
	
	//parser for (L
	std::shared_ptr<Parser<char,std::pair<char,Tree*>>> LeftPair = std::make_shared<Con<char,char,Tree*>>(OpenBrace,language);

	//parser for )L
	std::shared_ptr<Parser<char,std::pair<char,Tree*>>> RightPair = std::make_shared<Con<char,char,Tree*>>(CloseBrace,language);

	
	//parser for A = (L -> Tree
	std::shared_ptr<Parser<char,Tree*>> LeftReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(LeftPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});

	//parser for B = )L -> Tree
	std::shared_ptr<Parser<char,Tree*>> RightReduce = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(RightPair, [](std::pair<char,Tree*> in)->Tree* {return in.second;});

	
	//parser for AB
	std::shared_ptr<Parser<char,std::pair<Tree*,Tree*>>> parsePair = std::make_shared<Con<char,Tree*,Tree*>>(LeftReduce,RightReduce);
	
	
	//parser for AB -> C
	std::shared_ptr<Parser<char,Tree*>> ReducedPair = std::make_shared<Red<char,std::pair<Tree*,Tree*>,Tree*>>(parsePair,[](std::pair<Tree*,Tree*> input) -> Tree* {Tree* temp = new Tree(); temp->Left = input.first; temp->Right = input.second; return temp;});
	
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
