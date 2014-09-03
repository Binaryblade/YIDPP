#ifndef PARSER2_H_
#define PARSER2_H_
#include <vector>
#include <memory>
#include <utility>
#include <set>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace yidpp {
		template<class T,class A>
		class Parser;


		//Data object for fixed point computation
		class ChangeCell {
			public:
				ChangeCell() : change(false) {};
				bool change;
				std::set<void*> seen;
				bool orWith(bool change) {
					this->change = this->change || change;
					return this->change;
				}
		};

	//The abstract base class for all parsers
	template<class T, class A>
	class Parser : public std::enable_shared_from_this<Parser<T,A>> {
		public:
			Parser() : initialized(false) {};
		
			//retreive the parse forest because the stream has terminated
			std::set<A> parseNull() {
				if(isEmpty()) {
					return std::set<A>();
				} else {
					init();
					return parseNullLocal;
				}
			}

			//getter for Nullable that performs a lazy fixedpoint
			bool isNullable() {
				if(isEmpty()) {
					return false;
				} else {
					init();
					return isNullableLocal;
				}
			}

			//getter for Empty that performs a lazy fixed point
			bool isEmpty() {
				init();
				return isEmptyLocal;
			}

			//take the derivative with respect to a terminal pretty much the main algorithm
			std::shared_ptr<Parser<T,A>> derive (T t) {
				
				//should do an is empty check here 
				if (cache.count(t)) {
					return cache.find(t)->second; //if seen before return previous result
				} else {
						std::shared_ptr<Parser<T,A>> temp_derive = internalDerive(t);  //new get the internal derivative
						cache.insert(std::make_pair(t,temp_derive));
						return temp_derive;
				}
			}

			//parse the entire input stream and return the forest
			virtual std::set<A> parseFull(const std::vector<T>& input) { 
				if (input.empty()) {
					return parseNull();
				} else{  
					std::vector<T> remainder(++input.begin(), input.end());
					return derive(input.front())->parseFull(remainder);
				}
			}
		
			//parse the available input and get the interim state
			virtual std::set<std::pair<A,std::vector<T>>> parse(const std::vector<T>& input) {
				if (input.empty()) { 
					std::set<std::pair<A,std::vector<T>>> parseSet;
					std::set<A> parseNullResult = parseNull();
					for(typename std::set<A>::iterator i=parseNullResult.begin(); i != parseNullResult.end(); ++i) {
						parseSet.insert(std::make_pair(*i,std::vector<T>()));
					}
					return parseSet;
				} else { 
					std::set<std::pair<A,std::vector<T>>> nonEmpty = derive(input.front())->parse(std::vector<T>(++input.begin(),input.end()));
					std::set<A> parseFullResult = parseFull(std::vector<T>());
					for(typename std::set<A>::iterator i=parseFullResult.begin();i!=parseFullResult.end();++i) {
						nonEmpty.insert(std::make_pair(*i,input));
					}
					return nonEmpty;
				}
			}
		
			//virtual method for fixed point computation
			void updateChildBasedAttributes(ChangeCell& change) {
				void* localPtr = Parser<T,A>::shared_from_this().get();
				if(!change.seen.count(localPtr)) {
					change.seen.insert(localPtr);
					oneShotUpdate(change);
					initialized=true;
				}
				allUpdate(change);
			};

			virtual std::string getDescription() {
				std::string returntemp;
				returntemp += "digraph parseTree {\n";
				std::set<void*> emptySet;
				returntemp += treeRecurse(emptySet);
				returntemp += "}\n";
				return returntemp;
			}
			
			virtual std::string getNodeName() {
				std::stringstream ss;
				ss << std::hex << "\"" << static_cast<void*>(this) << "\"";
				return ss.str();
			}

			virtual std::string getNodeLabel() {
				 std::string nodelabel;
				 nodelabel = this->getNodeName() +" [label="+this->getLabel()+":"+this->getNodeName()+"];\n";
				return std::move(nodelabel);
			}

			virtual std::string treeRecurse(std::set<void*>& valueSet) {
				if(!valueSet.count(this)) {
					valueSet.insert(this);
					return this->getNodeLabel()+";\n"+getChildren(valueSet);
				}
				return "";
			}

			virtual std::string getChildren(std::set<void*>& valueSet) {
				return "";
			}

			virtual std::string getLabel() {
				return "UNKNOWN";
			}

		protected:
			//has the fixed point been done
			bool initialized;
			
			//virtual method for popping the derivative
			virtual std::shared_ptr<Parser<T,A>> internalDerive (T t) = 0;

			//setter for parse forest and checks if changed
			bool parseNullSet(std::set<A> set) {
				if(parseNullLocal != set) {
					parseNullLocal = set;
					return true;
				} else {
					return false;
				}
			}

			//setter for local is empty and checks if changed
			bool isEmptySet(bool v) {
				if(isEmptyLocal != v) {
					isEmptyLocal = v;
					return true;
				} else {
					return false;
				}
			}

			//setter for local nullable and checks if changed
			bool isNullableSet(bool v) {
				if(isNullableLocal != v) {
					isNullableLocal = v;
					return true;
				} else {
					return false;
				}
			}


			virtual void oneShotUpdate(ChangeCell&) {};
			virtual void allUpdate(ChangeCell&) {};
		
		private:
			//contains the current parse forest for it
			std::set<A> parseNullLocal;
			
			//cache of derivative results
			std::unordered_map<T,std::shared_ptr<Parser<T,A>>> cache;

			//Nullable and Empty status of the class
			bool isEmptyLocal = false;
			bool isNullableLocal = false;
			
			//performs the fixed point update of the properties
			void init() {
				if(initialized)
					return;
				ChangeCell change;
				do {
					change = ChangeCell();
					updateChildBasedAttributes(change);
				} while(change.change);
			}
};


template<class T, class A>
class DFut : public Parser<T,A> {
		std::shared_ptr<Parser<T,A>> to_derive;
		T terminal;

	public:
		DFut(std::shared_ptr<Parser<T,A>> input, T terminal) : to_derive(input) , terminal(terminal) {};
	
		virtual std::set<std::pair<A,std::vector<T>>> parse(const std::vector<T>& input) override {
			return to_derive->derive(terminal)->parse(input);
		}

		std::string getChildren(std::set<void*>& valueSet) override {
			std::string childrenString;
			childrenString = Parser<T,A>::getNodeLabel() + "->" + to_derive->derive(terminal)->getNodeName() + ";\n";
			childrenString += to_derive->derive(terminal)->treeRecurse(valueSet);
			return childrenString;
		}

		std::string getLabel() override {
			return "DerivativeFuture";
		}
	protected:

		std::shared_ptr<Parser<T,A>> internalDerive(T t) override {
			return std::make_shared<DFut<T,A>>(to_derive->derive(terminal),t);
		}

		virtual void oneShotUpdate(ChangeCell &change) override {
      to_derive->derive(terminal)->updateChildBasedAttributes(change);
		}

		virtual void allUpdate(ChangeCell& change) override {
			change.orWith (Parser<T,A>::parseNullSet(to_derive->derive(terminal)->parseNull()));
			change.orWith (Parser<T,A>::isEmptySet(to_derive->derive(terminal)->isEmpty()));
			change.orWith (Parser<T,A>::isNullableSet(to_derive->derive(terminal)->isNullable()));
		}

};


//class for the empty set
template<class T, class A>
class Emp : public Parser<T,A> {
	public:
		Emp() {
			//always empty
			Parser<T,A>::isEmptySet(true);
			//never nullable
			Parser<T,A>::isNullableSet(false);
		}
		
		//you get nothing out of parsing it
		virtual std::set<std::pair<A,std::vector<T>>> parse(const std::vector<T>& input) override {
			return std::set<std::pair<A,std::vector<T>>>();
		} //you get nothing out of parsing it
  
		std::string getLabel() override {
			return "Empty_Set";
		}
  protected:
		
		std::shared_ptr<Parser<T,A>> internalDerive (T t) override {
			return Parser<T,A>::shared_from_this();
		}//derivative of the empty set is the empty set
};


//class for the empty string or the Nulll reduction parser
template<class T, class A>
class Eps : public Parser<T,A> {
	public:
		Eps(std::set<A> generator) {
			//The empty string is not the Null Set
			//it contains only the empty string and is
			//thus non empty
			Parser<T,A>::isEmptySet(false);
			
			//It by definition contains the empty string
			//and is therefore nullable
			Parser<T,A>::isNullableSet(true);
			
			//The Character consumed (or possible parse trees consumed) to produce this is
			//what should be returned on a null parse
			Parser<T,A>::parseNullSet(generator);
		}


		std::string getLabel() override {
			return "Empty_String";
		}
	protected:

		//if you take the derivative of it you get the null set
		//which is no parser at all
		virtual std::shared_ptr<Parser<T,A>> internalDerive(T t) override {
			return std::make_shared<Emp<T,A>>();
		}

	public:
	virtual	std::set<std::pair<A,std::vector<T>>> parse(const std::vector<T>& input) override {
			std::set<std::pair<A,std::vector<T>>> retval;
			auto parseNullResult = Parser<T,A>::parseNull();
			for(auto i = parseNullResult.begin(); i!= parseNullResult.end(); ++i) {
				retval.insert(std::make_pair(*i,input));
			}
			return retval;
		}
};


//parser for a single terminal
template<class T>
class EqT : public Parser<T,T> {
	private:
		T t;
	public:
		EqT(T t) : t(t) {
			//A single character (terminal) is neither empty nor nullable as it contains
			//and item (the single terminal) and does not contain the empty string
			Parser<T,T>::isEmptySet(false);
			Parser<T,T>::isNullableSet(false);
		}
	
		virtual std::set<std::pair<T,std::vector<T>>> parse(const std::vector<T>& input) override {
				std::set<std::pair<T,std::vector<T>>> retval;
				if(!input.empty() && input.front() == t) {
					retval.insert(std::make_pair(input.front(), std::vector<T>(++input.begin(), input.end())));
				}
				return retval;
			}
  
		std::string getLabel() override {
			return "TerminalParser_"+t;
		}
  protected:

		virtual std::shared_ptr<Parser<T,T>> internalDerive(T t_) override {
			if(t == t_) {
				//derivative of the single terminal is the null reduction parser
				//formed with t as the construction option
				std::set<T> generator;
				generator.insert(t);
				return std::make_shared<Eps<T,T>>(generator);
			} else {
				//if not equal cannot be part of language
				//therefore null set or empty parser
				return std::make_shared<Emp<T,T>>();
			}
		}
};

//Union
template<class T,class A>
class Alt : public Parser<T,A> {
	private:
		std::shared_ptr<Parser<T,A>> choice1;
		std::shared_ptr<Parser<T,A>> choice2;

	public:
		Alt(std::shared_ptr<Parser<T,A>> choice1, std::shared_ptr<Parser<T,A>> choice2) : choice1(choice1), choice2(choice2) { }


		std::string getChildren(std::set<void*>& valueSet) override {
			std::string childrenString;
			childrenString = Parser<T,A>::getNodeName() + "->" + choice1->getNodeName() + ";\n";
			childrenString += Parser<T,A>::getNodeName() + "->" + choice2->getNodeName() + ";\n";
			childrenString += choice1->treeRecurse(valueSet);
			childrenString += choice2->treeRecurse(valueSet);
			return childrenString;
		}

		std::string getLabel() override {
			return "Union";
		}
	protected:

		virtual std::shared_ptr<Parser<T,A>> internalDerive(T t) override {
			//Quick optimization
			//if either choice is the empty parser
			//then the result is only the opposite derivative
			if(choice1->isEmpty()) 
				return std::make_shared<DFut<T,A>>(choice2,t);

			if(choice2->isEmpty())
				return std::make_shared<DFut<T,A>>(choice1,t);

			//if both have stuff then apply alternation rule, derivative of alternation is
			//alternation of derivative (it commutes)
			return std::make_shared<Alt<T,A>>(std::make_shared<DFut<T,A>>(choice1,t), std::make_shared<DFut<T,A>>(choice2,t));
		}

	protected:
		virtual void oneShotUpdate(ChangeCell &change) override {
      choice1->updateChildBasedAttributes(change);
      choice2->updateChildBasedAttributes(change);
		}

		virtual void allUpdate(ChangeCell& change) override {
			std::set<A> nullSet;
			auto first = choice1->parseNull();
			auto second = choice2->parseNull();
			nullSet.insert(first.begin(),first.end());
			nullSet.insert(second.begin(), second.end());
			change.orWith (Parser<T,A>::parseNullSet(nullSet));
			change.orWith (Parser<T,A>::isEmptySet(choice1->isEmpty() && choice2->isEmpty()));
			change.orWith (Parser<T,A>::isNullableSet(!Parser<T,A>::isEmpty() && (choice1->isNullable() || choice2->isNullable())));
		}
};


template<class T, class A, class B>
class Con : public Parser<T,std::pair<A,B>> {
 	private:
		std::shared_ptr<Parser<T,A>> first;
		std::shared_ptr<Parser<T,B>> second;
	public:
		Con(std::shared_ptr<Parser<T,A>> first, std::shared_ptr<Parser<T,B>> second) : first(first), second(second) {};


		std::string getChildren(std::set<void*>& valueSet) override {
			std::string childrenString;
			childrenString = this->getNodeName() + "->" + first->getNodeName() + ";\n";
			childrenString = this->getNodeName() + "->" + second->getNodeName() + ";\n";
			childrenString += first->treeRecurse(valueSet);
			childrenString += second->treeRecurse(valueSet);
			return childrenString;
		}

		std::string getLabel() override {
			return "Concatenation";
		}
	protected:
		virtual std::shared_ptr<Parser<T,std::pair<A,B>>> internalDerive(T t) override {

			//Concatenation has some funny rules and is also a big target of compaction
			//so apply it here
			std::shared_ptr<Parser<T,A>> leftDerive = std::make_shared<DFut<T,A>>(first,t);
			std::shared_ptr<Parser<T,std::pair<A,B>>> primaryRet = std::make_shared<Emp<T,std::pair<A,B>>>();
			if(!leftDerive->isEmpty()) { 
				//only if the first expressions derivative is non-empty do we care about the first
				//concatenation in the union
				primaryRet = std::make_shared<Con<T,A,B>>(leftDerive, second);
			}

			if(first->isNullable()) {
				//If the first language is nullable, then the second term in the rule appears
				//and we need to generate the null reduction (empty string parser) for the first
				//term
				std::shared_ptr<Parser<T,A>> nullability = std::make_shared<Eps<T,A>>(first->parseNull());
				//we also want the second derivate to concat with the reduction parser
				std::shared_ptr<Parser<T,B>> rightDerive = std::make_shared<DFut<T,B>>(second,t);
				if(leftDerive->isEmpty()) {
					//if the left is empty then only worry about nullability portion of the concat
					if(rightDerive->isEmpty()) {
						//if both right and left are empty, then return the Null parser because its screwed anyway
						return primaryRet;
					} else {
						//just the right side of the alternation rule 
						return std::make_shared<Con<T,A,B>>(nullability,rightDerive);
					}
				} else {
					//There is nothing we can do to optimize, so just take the full rule
					return std::make_shared<Alt<T,std::pair<A,B>>>(primaryRet,std::make_shared<Con<T,A,B>>(nullability,rightDerive));
				}
			} else {
				//The second side is not nullable (there for the nullability operation nulls out the right half)
				//so just take the first term of the rule
				return primaryRet;
			}
		}

	protected:
		virtual void oneShotUpdate(ChangeCell& change) override {
				first->updateChildBasedAttributes(change);
				second->updateChildBasedAttributes(change);
		}

		virtual void allUpdate(ChangeCell& change) override {
			std::set<std::pair<A,B>> options;
			auto firstNull = first->parseNull();
			auto secondNull = second->parseNull();
			for(auto i=firstNull.begin();i!=firstNull.end(); ++i) {
				for(auto j=secondNull.begin();j!=secondNull.end(); ++j) {
					options.insert(std::make_pair(*i,*j));
				}
			}
			change.orWith(Parser<T,std::pair<A,B>>::parseNullSet(options));
			change.orWith(Parser<T,std::pair<A,B>>::isEmptySet(first->isEmpty() || second->isEmpty()));
			change.orWith(Parser<T,std::pair<A,B>>::isNullableSet(!Parser<T,std::pair<A,B>>::isEmpty() &&(first->isNullable() && second->isNullable())));
		}
};


//Reducton Operator
template<class T, class A, class B>
class Red : public Parser<T,B> {
  private:
		std::shared_ptr<Parser<T,A>> localParser;
		std::function<B(A)> reductionFunction;
	public:
		Red(std::shared_ptr<Parser<T,A>> parser, std::function<B(A)> redfunc): localParser(parser), reductionFunction(redfunc) {};	

		std::string getChildren(std::set<void*>& valueSet) override {
			std::string childrenString;
			childrenString = Parser<T,B>::getNodeName() + "->" + localParser->getNodeName() + ";\n";
			childrenString += localParser->treeRecurse(valueSet);
			return childrenString;
		}

		std::string getLabel() override {
			return "ReductionOperation";
		}
	
	protected:
		virtual std::shared_ptr<Parser<T,B>> internalDerive(T t) override {
			
			auto temp = std::make_shared<DFut<T,A>>(localParser,t);
			//If the derivitive of the internal parser which you are reducing is the Null Parser
			//Then the result is simply the Null Parser of the correct type
			if(temp->isEmpty()) {
				return std::make_shared<Emp<T,B>>();
			}
			
			//derivative of the reduction is the reduction of the derivative
			return std::make_shared<Red<T,A,B>>(temp,reductionFunction);
		}

	public:
		virtual std::set<B> parseFull(const std::vector<T> &input) override {
			auto lower = localParser->parseFull(input);
			std::set<B> retval;
			//Just take the internal Parse Forest and reduce it.
			for(auto i=lower.begin();i!=lower.end();++i) {
				retval.insert(reductionFunction(*i));
			}
			return retval;
		}

		virtual std::set<std::pair<B,std::vector<T>>> parse(const std::vector<T>& input) override {
			std::set<std::pair<A,std::vector<T>>> lower = localParser->parse(input);
			std::set<std::pair<B,std::vector<T>>> retval;
			for(auto i=lower.begin();i!=lower.end();++i) {
				retval.insert(std::make_pair(reductionFunction(i->first),i->second));
			}
			return retval;
		}

	protected:
		virtual void oneShotUpdate(ChangeCell& change) override {
				localParser->updateChildBasedAttributes(change);
		}

		virtual void allUpdate(ChangeCell& change) override {
			auto localParseNull = localParser->parseNull();
			std::set<B> changedParseNull;
			for(auto i=localParseNull.begin();i!=localParseNull.end();++i) {
				changedParseNull.insert(reductionFunction(*i));
			}
			change.orWith(Parser<T,B>::parseNullSet(changedParseNull));
			change.orWith(Parser<T,B>::isEmptySet(localParser->isEmpty()));
			change.orWith(Parser<T,B>::isNullableSet(localParser->isNullable()));
		}
};

//Kleene Star
template<class T, class A>
class Rep: public Parser<T,std::vector<A>> {
 	private:
		std::shared_ptr<Parser<T,A>> internal;
	public:
		Rep(std::shared_ptr<Parser<T,A>> p) : internal(p) {
			Parser<T,std::vector<A>>::isEmptySet(false);
			Parser<T,std::vector<A>>::isNullableSet(true);
			std::set<std::vector<A>> retset;
			retset.insert(std::vector<A>());
			Parser<T,std::vector<A>>::parseNullSet(retset);
		}
  
		std::string getChildren(std::set<void*>& valueSet) override {
			std::string childrenString;
			childrenString = this->getNodeName() + "->" + internal->getNodeName() + ";\n";
			childrenString += internal->treeRecurse(valueSet);
			return childrenString;
		}

		std::string getLabel() override {
			return "Kleene";
		}
	
	protected:
	
	static std::vector<A> reductionOperation(std::pair<A,std::vector<A>> input) {
					std::vector<A> retval; 
					retval.push_back(input.first);
					retval.insert(retval.begin(),input.second.begin(),input.second.end());
					return retval;
	}
	
	virtual std::shared_ptr<Parser<T,std::vector<A>>> internalDerive(T t) override {
			auto localDerive = std::make_shared<Con<T,A,std::vector<A>>>(std::make_shared<DFut<T,A>>(internal,t), Parser<T,std::vector<A>>::shared_from_this());
			return std::make_shared<Red<T,std::pair<A,std::vector<A>>,std::vector<A>>>(localDerive,Rep<T,A>::reductionOperation);
		}

	protected:
		virtual void oneShotUpdate(ChangeCell& change) override {
				internal->updateChildBasedAttributes(change);
				Parser<T,std::vector<A>>::initialized=true;
		}
};


};

#endif
