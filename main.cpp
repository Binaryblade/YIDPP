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

	//parser for A = () -> Tree*
	auto ClosedPair = std::make_shared<Red<char,std::pair<char,char>,Tree*>>(std::make_shared<Con<char,char,char>>(OpenBrace, CloseBrace),
		[](std::pair<char,char> in) -> Tree* {return new Tree();}
		);
	
	//parser for L, needs its child to be set later
	auto language = std::make_shared<RecursiveParser<char,Tree*>>();
	
	//parser for Lp = (L -> Tree*
	auto LeftPair = std::make_shared<Red<char,std::pair<char,Tree*>,Tree*>>(std::make_shared<Con<char,char,Tree*>>(OpenBrace,language),
		[](std::pair<char,Tree*> in) -> Tree* { return new Tree(in.second,nullptr);}
	);

	//parser for B = Lp) -> Tree*
	auto RightPair = std::make_shared<Red<char,std::pair<Tree*,char>,Tree*>>(std::make_shared<Con<char,Tree*,char>>(LeftPair,CloseBrace),
	 [](std::pair<Tree*,char> in) -> Tree* { return in.first; }
	 );

	//parser for pair of languages
	// C = LL -> Tree*
	auto CompletePair = std::make_shared<Red<char,std::pair<Tree*,Tree*>,Tree*>>(std::make_shared<Con<char,Tree*,Tree*>>(language,language),
		[](std::pair<Tree*,Tree*> in) -> Tree* { return new Tree(in.first,in.second);} 
	);
	

	//Parser for union
	// Fl = A U B U C
	std::set<std::shared_ptr<Parser<char,Tree*>>> unionSet;
	unionSet.insert(ClosedPair);
	unionSet.insert(RightPair);
	unionSet.insert(CompletePair);
	auto FullLanguage = std::make_shared<Alt<char,Tree*>>(unionSet);

	//set L to be Fl so that recursion can happen 
	language->SetRecurse( FullLanguage );


	std::shared_ptr<Parser<char,Tree*>> recurse = FullLanguage;
	for(size_t i=0;i<25;++i) {
		std::stringstream sstream;
		sstream << i;
		std::cout << getGraph<char,Tree*>(sstream.str(),recurse) << std::endl;
		recurse = recurse->derive('(');
	}

//	recurse = recurse->derive(')');
//	std::cout << getGraph<char,Tree*>("second",recurse) << std::endl;
//	recurse = recurse->derive(')');
//	std::cout << getGraph<char,Tree*>("third",recurse) << std::endl;
	return 0;
}
