#include "special_parser.h"

StringParser::StringParser(const std::string &parseString) : internalString(parseString) {
			Parser<char,std::string>::isNullableSet(parseString.size() == 0);
			Parser<char,std::string>::isEmptySet(false);
			if(parseString.size() == 0) {
				std::string temp;
				std::set<std::string> setTemp;
				setTemp.insert(temp);
				Parser<char,std::string>::parseNullSet(setTemp);	
			}
}

std::string StringParser::reductionFunc(const std::pair<char,std::string>& pairing) {
	return pairing.first+pairing.second;
}

std::shared_ptr<yidpp::Parser<char,std::string>> StringParser::internalDerive(char t, std::set<void*> nulls) {
			if(internalString.size() == 0) {
				return std::make_shared<yidpp::Emp<char,std::string>>();
			} else {
				if(t == internalString[0]) {
					std::set<char> possibilities;
					possibilities.insert(t);
					auto nullability = std::make_shared<yidpp::Eps<char,char>>(possibilities);
					auto stringRemainder = std::make_shared<StringParser>(std::string(++internalString.begin(),internalString.end()));
					std::shared_ptr<yidpp::Parser<char,std::pair<char,std::string>>> stringcharpair = std::make_shared<yidpp::Con<char,char,std::string>>(nullability,stringRemainder);
					return std::make_shared<yidpp::Red<char,std::pair<char,std::string>,std::string>>(stringcharpair,StringParser::reductionFunc);
				} else {
					return std::make_shared<yidpp::Emp<char,std::string>>();
				}
			}
}
