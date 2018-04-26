#include <string>
#include <vector>

std::string base64_encode(std::vector<unsigned char> &bytes_to_encode);
std::string base64_encode(const unsigned char* bytes_to_encode, unsigned int len);
std::string base64_decode(std::string const& s);