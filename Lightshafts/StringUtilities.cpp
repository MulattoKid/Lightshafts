#pragma once

#include "StringUtilities.h"

unsigned int CountNumberOfLinesStartingWith(std::ifstream& reader, const std::string seq)
{
	unsigned int count = 0;
	std::string line;
	while (std::getline(reader, line))
	{
		if (StringStartsWith(line, seq))
		{
			count++;
		}
	}

	//Reset reader
	reader.clear();
	reader.seekg(0, std::ios::beg);

	return count;
}

unsigned short CountSpaces(const std::string& line, const unsigned short start_index)
{
	unsigned short num_spaces = 0;
	for (int i = start_index; i < line.size(); i++)
	{
		if (line[i] == ' ')
		{
			num_spaces++;
		}
	}
	if (LastCharIsSpace(line)) // NOTE(Daniel): Some .obj files have can have a space at the end of the file - this takes care of this
	{
		num_spaces--;
	}
	return num_spaces;
}

std::string ExtractNextWord(const std::string& line, const unsigned short start_index)
{
	unsigned short current_index = start_index;
	while (current_index < line.length() && line[current_index] != ' ') //Space represents a new word
	{
		current_index++;
	}
	return line.substr(start_index, current_index);
}

void ExtractPoints(const std::string& line, float* points, const unsigned short num_numbers, const unsigned short start_index)
{
	unsigned short numbers_found = 0;
	unsigned short start = start_index;
	unsigned short i = start_index;
	while (i < line.size() && numbers_found < num_numbers - 1) //Find all but last one
	{
		if (line[i] == ' ')
		{
			points[numbers_found++] = std::stof(line.substr(start, i).c_str());
			start = i + 1;
		}
		i++;
	}
	points[numbers_found] = std::stof(line.substr(start, i).c_str()); //Get last number
}

unsigned short FindFirstNonSpace(const std::string& line, const unsigned short start_index)
{
	int i = start_index;
	while (i < line.size())
	{
		if (line[i] != ' ')
		{
			return i;
		}
		i++;
	}
	return -1; //Should never happen
}

std::string	GetDirectory(const std::string& filename)
{
	for (int i = filename.length() - 1; i >= 0; i--)
	{
		if (filename[i] == '/')
		{
			return filename.substr(0, i + 1); //+1 to add back '/'
		}
	}
}

bool LastCharIsSpace(const std::string& line)
{
	if (line[line.size() - 1] == ' ')
	{
		return true;
	}
	return false;
}

bool NextSequenceIs(const std::string& str, const std::string& seq, unsigned short start_index)
{
	//Early exit
	if (seq.length() > str.length() - start_index) { return false; }

	int i = 0;
	while (i < seq.length())
	{
		if (seq[i] != str[i + start_index])
		{
			return false;
		}
		i++;
	}
	return true;
}

bool NextWordIs(const std::string& str, const std::string& word, unsigned short start_index)
{
	//Early exit
	if (word.length() > str.length() - start_index) { return false; }

	int i = 0;
	while (i < word.length())
	{
		if (word[i] != str[i + start_index] || str[i + start_index] == ' ')
		{
			return false;
		}
		i++;
	}
	return true;
}

bool StringStartsWith(const std::string& str, const std::string& seq)
{
	//Early exit
	if (seq.length() > str.length()) { return false; }

	for (int i = 0; i < seq.length(); i++)
	{
		if (seq[i] != str[i])
		{
			return false;
		}
	}
	return true;
}