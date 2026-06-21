#include "Mode.hpp"
#include <stdexcept>

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

Mode::Mode() : score(0), prefixCount(0), suffixCount(0), matchingCount(0) {

}

static std::string::size_type hexValueNoException(char c) {
	const std::string hex = "0123456789abcdef";
	const std::string::size_type ret = hex.find(tolower(c));
	return ret;
}

static std::string trimTargetLine(std::string line) {
	if (line.size() >= 3 &&
		static_cast<unsigned char>(line[0]) == 0xEF &&
		static_cast<unsigned char>(line[1]) == 0xBB &&
		static_cast<unsigned char>(line[2]) == 0xBF) {
		line.erase(0, 3);
	}
	line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), line.end());
	return line;
}

static bool isBase58Character(char c) {
	const std::string alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
	return alphabet.find(c) != std::string::npos;
}

static bool isValidTronAddress(const std::string& line) {
	return line.size() == 34 && line[0] == 'T' && std::all_of(line.begin(), line.end(), isBase58Character);
}

Mode Mode::matching(std::string matchingInput) {
	Mode r;
	std::vector<std::string> matchingList;

	matchingInput = trimTargetLine(matchingInput);

	if(matchingInput.size() == 34 && matchingInput[0] == 'T') {
		if(!isValidTronAddress(matchingInput)) {
			std::cout << "error: matching target contains invalid TRON/Base58 characters; 0 is not allowed. :<" << std::endl;
			return r;
		}
		std::stringstream ss;
		matchingInput.erase(10, 14);
		for (const char &item: matchingInput) {
			ss << std::hex << int(item);
		}
		matchingList.push_back(ss.str());
	} else {
		std::ifstream file(matchingInput);
		if (file.is_open()) {
			std::string line;
			while (std::getline(file, line)) {
				line = trimTargetLine(line);
				std::stringstream ss;
				if(line.size() == 20 || isValidTronAddress(line)) {
					if(line.size() == 34) {
						line.erase(10, 14);
					}
					for (const char &item: line) {
						ss << std::hex << int(item);
					}
					matchingList.push_back(ss.str());
				}
			}
		} else {
			std::cout << "error: Failed to open matching file. :<" << std::endl;
		}
	}
	
	if(matchingList.size() > 0) {
		r.matchingCount = matchingList.size();
		for( size_t j = 0; j < matchingList.size(); j += 1) {
			const std::string matchingItem = matchingList[j];
			for( size_t i = 0; i < matchingItem.size(); i += 2 ) {
				const size_t indexHi = hexValueNoException(matchingItem[i]);
				const size_t indexLo = (i + 1) < matchingItem.size() ? hexValueNoException(matchingItem[i+1]) : std::string::npos;
				const unsigned long valHi = (indexHi == std::string::npos) ? 0 : indexHi << 4;
				const unsigned long valLo = (indexLo == std::string::npos) ? 0 : indexLo;
				const int maskHi = (indexHi == std::string::npos) ? 0 : 0xF << 4;
				const int maskLo = (indexLo == std::string::npos) ? 0 : 0xF;
				r.data1.push_back(maskHi | maskLo);
				r.data2.push_back(valHi | valLo);
			}
		}
	}

	return r;
}
