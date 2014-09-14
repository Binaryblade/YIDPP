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
			~Tree() {
				if(Left != nullptr)
					delete Left;

				if(Right != nullptr)
					delete Right;
			}
	};


	//Constructing the Language of matched braces
	//L = () U (L) U LL
		
	//parser for (
	auto OpenBrace = std::make_shared<EqT<char>>('(');
	//parser for )
	auto CloseBrace = std::make_shared<EqT<char>>(')');

	auto ClosedPair = std::make_shared<Con<char,char,char>>();
	ClosedPair->setLeft(OpenBrace);
	ClosedPair->setRight(CloseBrace);

	//parser for A = () -> Tree*
	auto RedClosedPair = std::make_shared<Red<char,std::pair<char,char>,Tree*>>(
		[](std::pair<char,char> in) -> Tree* {return new Tree();}
		);
	RedClosedPair->setParser(ClosedPair);
	
	//Parser for union
	// Fl = A U B U C
	auto FullLanguage = std::make_shared<Alt<char,Tree*>>();
	
	//parser for Lp = (L -> Tree*
	auto LeftPair = std::make_shared<Con<char,char,Tree*>>();
	LeftPair->setLeft(OpenBrace);
	LeftPair->setRight(FullLanguage);

	auto RedLeftPair = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(
		[](std::pair<char,Tree*> in) -> Tree* { return new Tree(in.second,nullptr);}
	);
	RedLeftPair->setParser(LeftPair);

	//parser for B = Lp) -> Tree*
	auto RightPair = std::make_shared<Con<char,Tree*,char>>();
	RightPair->setLeft(RedLeftPair);
	RightPair->setRight(CloseBrace);

	auto RedRightPair = std::make_shared<Red<char,std::pair<Tree*,char>,Tree*>>(
	 [](std::pair<Tree*,char> in) -> Tree* { return in.first; }
	 );
	 RedRightPair->setParser(RightPair);

	//parser for pair of languages
	// C = LL -> Tree*
	auto CompletePair = std::make_shared<Con<char,Tree*,Tree*>>();
	CompletePair->setLeft(FullLanguage);
	CompletePair->setRight(FullLanguage);
	auto RedCompletePair = std::make_shared<Red<char,std::pair<Tree*,Tree*>,Tree*>>(
		[](std::pair<Tree*,Tree*> in) -> Tree* { return new Tree(in.first,in.second);} 
	);
	RedCompletePair->setParser(CompletePair);
	
	FullLanguage->addParser(RedRightPair);
	FullLanguage->addParser(RedCompletePair);
	FullLanguage->addParser(RedClosedPair);



	std::shared_ptr<Parser<char,Tree*>> recurse = FullLanguage;
	std::string parseString = "()()()";
	for(auto i=parseString.begin();i!=parseString.end();++i) {
		std::stringstream sstream;
		sstream << (i-parseString.begin());
		std::cout << getGraph<char,Tree*>(sstream.str(),recurse) << std::endl;
		recurse = recurse->derive(*i);
	}

	std::cerr << "Derivatives Complete: parsing null" << std::endl;
	std::set<Tree*> test = recurse->parseNull();
	std::cerr << test.size() << std::endl;
	return 0;
}
