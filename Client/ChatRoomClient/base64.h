#pragma once
#include <iostream>
#include <string>

std::string base64_encode(unsigned char const*, unsigned int len);
std::string base64_decode(std::string const& s);

const char* encode(std::string buf);					//º”√‹
char* decode(char* buf, int n);					//Ω‚√‹