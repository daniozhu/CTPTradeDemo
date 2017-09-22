#include "stdafx.h"
#include "Util.h"

#include <fstream>
#include <regex>


void Util::FormatJsonFile(const std::string& srcJsonFilePath, const std::string destJsonFilePath)
{
	std::regex r("\\b([a-zA-Z]+):");

	std::ifstream fin(srcJsonFilePath.c_str(), std::ios::in);
	std::ofstream fout(destJsonFilePath.c_str(), std::ios::out);

	char str[256];

	while (fin.getline(str, 256, '}'))
	{
		fout << regex_replace(str, r, "\"$1\":") << (fin.eof() ? "" : "}") << std::endl;
	}

	fout.close();
	fin.close();
}
