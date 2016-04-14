/*
 * Copyright (C) 2016, Kevin Brubeck Unhammer <unhammer@fsfe.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mwesplit.hpp"

namespace gtd {


const std::basic_regex<char> CG_LINE ("^"
				      "(\"<(.*)>\".*" // wordform, group 2
				      "|([\t ]+)(\"[^\"]*\"\\S*)(\\s+\\S+)*" // reading, group 3, 4, 5
				      ")");

const std::basic_regex<char> CG_WFTAG (".*(\\s+\"<(.*)>\")");

struct Reading {
		std::string ana;
		std::string wftag;
};

struct Cohort {
		std::string form;
		std::vector<std::vector<Reading> > readings;
		std::string postblank;
};

const std::pair<std::string, std::string> extr_wftag(const std::string line)
{
	std::match_results<const char*> result;
	std::regex_match(line.c_str(), result, CG_WFTAG);
	if(!result.empty() && result[2].length() != 0) {
		std::string rest = line.substr(0, result.position(1)) + line.substr(result.position(1)+result.length(1));
		return std::make_pair(rest, result[2]);
	}
	else {
		return std::make_pair(line, "");	// TODO: what if there's an empty wftag? call it a bug?
	}
}

const void print_reading(std::ostream& os, const Reading& r) {
	os << r.ana <<std::endl;
}

const void print_cohort(std::ostream& os, const Cohort& c) {
	os << "\"<" << c.form << ">\"" << std::endl;
	for(auto &r : c.readings) {
		for(auto &s : r) {
			print_reading(os, s);
		}
	}
	if(!c.postblank.empty()) {
		os << ":" << c.postblank << std::endl;
	}
}

const std::string reindent(const std::string ana, const size_t level) {
	std::string indent = std::string(level+1, '\t');
	return indent + ana.substr(ana.find("\""));
}


const Cohort cohort_from_wftag(const std::string form) {
	// Treats any trailing blanks as if they should be in between
	// words (TODO: only do this for non-final words?)
	size_t i = form.find_last_not_of(" \n\r\t")+1;
	return { form.substr(0, i), {}, form.substr(i) };
}

const std::vector<Cohort> split_cohort(const Cohort& mwe, const unsigned int lno) {
	size_t n_wftags = 0;
	for(const auto& r : mwe.readings) {
		if(!r.empty() && !r.front().wftag.empty()) {
			++n_wftags;
		}
	}
	if(n_wftags < mwe.readings.size()) {
		if(n_wftags > 0) {
			std::cerr << "WARNING: Line " << lno << ": Some but not all main-readings had wordform tags, not splitting."<< std::endl;
		}
		return { mwe };
	}

	std::vector<Cohort> cos;
	for(const auto& r : mwe.readings) {
		size_t pos = -1;
		for(const auto& s : r) {
			if(!s.wftag.empty()) {
				++pos;
				Cohort c = cohort_from_wftag(s.wftag);
				while(cos.size() < pos+1) {
					cos.push_back(c);
				}
				if(cos[pos].form != c.form) {
					std::cerr << "WARNING: Line " << lno << ": Ambiguous word form tags for same cohort, '" << cos[pos].form << "' vs '" << s.wftag << "'"<< std::endl;
				}
				cos[pos].readings.push_back({});
			}
			size_t level = cos[pos].readings.back().size();
			Reading n = { reindent(s.ana, level), "" };
			cos[pos].readings.back().push_back(n);
		}
	}
	// The last word forms are the top readings:
	std::reverse(cos.begin(), cos.end());
	return cos;
}

const void split_and_print(std::ostream& os, const Cohort& c, const unsigned int lno) {
	if(!c.form.empty()) {
		const std::vector<Cohort> cos = split_cohort(c, lno);
		for(auto& c : cos) {
			print_cohort(os, c);
		}
	}
}


void run(std::istream& is, std::ostream& os)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
	std::ostringstream ss;
	Cohort cohort = { "", {}, "" };
	size_t indentation = 0;
	unsigned int lno = 0;
	for (std::string line; std::getline(is, line);) {
		++lno;
		std::match_results<const char*> result;
		std::regex_match(line.c_str(), result, CG_LINE);
		if(!result.empty() && result[2].length() != 0) {
			split_and_print(os, cohort, lno);
			cohort = { result[2], {}, "" };
			indentation = 0;
		}
		else if(!result.empty() && result[3].length() != 0) {
			auto ana_wf = extr_wftag(line);
			Reading r = { ana_wf.first, ana_wf.second };
			if(cohort.readings.empty()) {
				cohort.readings.push_back({r});
			}
			else if((unsigned)result[3].length() > indentation) {
				// we know readings non-empty because above if:
				cohort.readings.back().push_back(r);
			}
			else {
				// indentation same or decreased (CG doesn't allow "ambiguous" subreadings
				// of a reading, so we always go all the way back up to main cohort here)
				cohort.readings.push_back({r});
			}
			indentation = result[3].length();
		}
		else {
			split_and_print(os, cohort, lno);
			cohort = { "", {} };
			indentation = 0;
			os << line << std::endl;
		}
	}

	split_and_print(os, cohort, lno);
}
}
