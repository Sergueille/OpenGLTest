#include "LocalizationManager.h"

#include <iostream>
#include <fstream>

LocalizationManager::Language LocalizationManager::currentLanguage = Language::english;
std::map<std::string, std::string> LocalizationManager::data = std::map<std::string, std::string>();

void LocalizationManager::LoadLanguage(Language lang)
{
	std::cout << "Loading localizations files" << std::endl;

	currentLanguage = lang;
	data = std::map<std::string, std::string>(); // Discard old data
	ReadFile("instructions.csv");
	ReadFile("ui.csv");
	ReadFile("terminal.csv");
}

std::string LocalizationManager::GetLocale(std::string key)
{
	if (!data.contains(key))
	{
		std::cerr << "Localization key (" << key << ") not found" << std::endl;
		return "???";
	}

	return data[key];
}

void LocalizationManager::Clear()
{
	
}

void LocalizationManager::ReadFile(std::string path)
{
	std::string realPath = "Localization\\" + path;

	std::cout << "Loading localization file " << path << std::endl;

	std::ifstream ifile = std::ifstream();
	ifile.open(realPath, std::ios::in);

	// Can't open
	if (!ifile.is_open())
	{
		std::cerr << "Couldn't open localization file!" << std::endl;
		return;
	}

	try
	{
		int i = 0;
		char ch = 0;
		std::string lastKey = "";
		do
		{
			if (ch != '"')
				DiscardUntil(&ifile, '"');

			if (ifile.eof()) break;

			std::string value = ReadValue(&ifile);

			if (i > (int)Language::langCount) // If not in header
			{
				if (i % ((int)Language::langCount + 1) == 0) // Value is key
				{
					lastKey = std::string(value.begin(), value.end());
				}
				else if (i % ((int)Language::langCount + 1) == (int)currentLanguage + 1) // Value is traduction
				{
					data.insert(std::pair<std::string, std::string>(lastKey, value));
				}
			}

			i++;
		} 
		while (ifile >> ch);

		ifile.close();
	}
	catch (std::exception e)
	{
		ifile.close();
		std::cout << "Got an error while loading localization file, here's what it says:" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

void LocalizationManager::DiscardUntil(std::ifstream* ifile, char target)
{
	char ch = 0;
	while (ch != target && !ifile->eof())
	{
		ifile->get(ch);
	}
}

std::string LocalizationManager::ReadValue(std::ifstream* ifile)
{
	char ch;
	ifile->get(ch);; // Read firest char

	std::string str;
	while (ch != '"' && !ifile->eof())
	{
		str.push_back(ch); // Maybe a bit slow
		ifile->get(ch);
	}

	return str;
}
