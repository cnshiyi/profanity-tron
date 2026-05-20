#ifndef HPP_LEXICALCAST
#define HPP_LEXICALCAST

#include <sstream>
#include <stdexcept>

template <typename T>
T fromString(const std::string s) {
	std::istringstream ss(s);
	T t{};
	ss >> t;
	if (ss.fail() || !ss.eof()) {
		throw std::runtime_error("invalid numeric argument: " + s);
	}
	return t;
}

template <typename T>
std::string toString(const T & t) {
	std::ostringstream ss;
	ss << t;
	return ss.str();
}

#endif /* HPP_LEXICALCAST */
