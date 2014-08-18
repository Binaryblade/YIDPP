#ifndef SPECIAL_PARSER_H_
#define SPECIAL_PARSER_H_

#include "parser.h"
#include <string>

class StringParser : public yidpp::Parser<char,std::string> {
	public:
		StringParser(const std::string &parseString);
		static std::string reductionFunc(const std::pair<char,std::string>& pairing);
	protected:
		virtual std::shared_ptr<Parser<char,std::string>> internalDerive(char t, std::set<void*> nulls) override;
	private:
		std::string internalString;
};

#endif
