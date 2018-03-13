#pragma once

#include <fstream>
#include <string>

unsigned int	CountNumberOfLinesStartingWith(std::ifstream& reader, const std::string seq);
unsigned short	CountSpaces(const std::string& line, const unsigned short start_index);
std::string		ExtractNextWord(const std::string& line, const unsigned short start_index);
void			ExtractPoints(const std::string& line, float* points, const unsigned short num_numbers, const unsigned short start_index);
unsigned short	FindFirstNonSpace(const std::string& line, const unsigned short start_index);
std::string		GetDirectory(const std::string& filename);
bool			LastCharIsSpace(const std::string& line);
bool			NextSequenceIs(const std::string& str, const std::string& seq, unsigned short start_index);
bool			NextWordIs(const std::string& str, const std::string& word, unsigned short start_index);
bool			StringStartsWith(const std::string& str, const std::string& seq);