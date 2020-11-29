#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
#include "pch.h"
using namespace std;

shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
	// . . .
	// if AND 
	//istringstream line(s);  // separate the line into words
	//string word;
	//bool first = true;
	//std::vector<std::string> vec;
	//while (line >> word) {
	//	vec->push_back(line);
	//	if (first && word.compare("AND")) {

	//	}
	//	else if (first && word.compare("OR")) {

	//	}
	//	else if (first && word.compare("AD")) {
	//	}
	//	else {}
	//}
		//return std::shared_ptr<QueryBase>(new AndQuery(s1, s2));

  // All Regex:
	regex regex_AND(R"(^\\s*AND\\s+(\\w+)\\s+(\\w+)\\s*$)");
	regex regex_OR(R"(^\\s*OR\\s+(\\w+)\\s+(\\w+)\\s*$)");
	regex regex_AD(R"(^\\s*AD\\s+(\\w+)\\s+(\\w+)\\s*$)");
	regex regex_one_word((R"(^\\s*(\\w+)\\s*$)"));

	//one word
	auto words_begin = sregex_iterator(s.begin(), s.end(), regex_one_word);
	auto words_end = sregex_iterator();
	smatch cit = *words_begin;
	if (distance(words_begin, words_end) > 0)
	{
		return std::shared_ptr<QueryBase>(new WordQuery(cit.str(1)));
	}

	// AND
	auto words_begin = sregex_iterator(s.begin(), s.end(), regex_AND);
	auto words_end = sregex_iterator();
	smatch cit = *words_begin;
	if (distance(words_begin, words_end) > 0)
	{
		return std::shared_ptr<QueryBase>(new AndQuery(cit.str(1), cit.str(2)));
	}

	// OR
	auto words_begin = sregex_iterator(s.begin(), s.end(), regex_OR);
	auto words_end = sregex_iterator();
	smatch cit = *words_begin;
	if (distance(words_begin, words_end) > 0)
	{
		return std::shared_ptr<QueryBase>(new OrQuery(cit.str(1), cit.str(2)));
	}

	// AD
	auto words_begin = sregex_iterator(s.begin(), s.end(), regex_AD);
	auto words_end = sregex_iterator();
	smatch cit = *words_begin;
	if (distance(words_begin, words_end) > 0)
	{
		return std::shared_ptr<QueryBase>(new AdjacentQuery(cit.str(1), cit.str(2)));
	}

	cout << "Unrecognized search" << endl;
}
	QueryResult AndQuery::eval(const TextQuery& text) const
	{
		QueryResult left_result = text.query(left_query);
		QueryResult right_result = text.query(right_query);
		auto ret_lines = make_shared<set<line_no>>();
		set_intersection(left_result.begin(), left_result.end(),
			right_result.begin(), right_result.end(),
			inserter(*ret_lines, ret_lines->begin()));
		return QueryResult(rep(), ret_lines, left_result.get_file());
	}

	QueryResult OrQuery::eval(const TextQuery &text) const
	{
		QueryResult left_result = text.query(left_query);
		QueryResult right_result = text.query(right_query);
		auto ret_lines = make_shared<set<line_no>>(left_result.begin(), left_result.end());
		ret_lines->insert(right_result.begin(), right_result.end());
		return QueryResult(rep(), ret_lines, left_result.get_file());
	}

	QueryResult AdjacentQuery::eval(const TextQuery& text) const
	{
		QueryResult left_result = text.query(left_query);
		QueryResult right_result = text.query(right_query);
		auto ret_lines = make_shared<set<line_no>>();
		for (auto itLeft = left_result.begin(); itLeft != left_result.end(); ++itLeft) {
			for (auto itRight = right_result.begin(); itRight != right_result.end(); ++itRight) {
				if ((*itLeft) + 1 == (*itRight) || (*itLeft) - 1 == (*itRight)) {
					ret_lines->insert((*itLeft));
					ret_lines->insert((*itRight));
				}
			}
		}
		return QueryResult(rep(), ret_lines, left_result.get_file());


	}

	std::ostream &print(std::ostream &os, const QueryResult &qr)
	{
		os << "\"" << qr.sought << "\"" << " occurs " <<
			qr.lines->size() << " times:" << std::endl;
		for (auto num : *qr.lines)
		{
			os << "\t(line " << num + 1 << ") "
				<< *(qr.file->begin() + num) << std::endl;
		}
		return os;
	}
	/////////////////////////////////////////////////////////