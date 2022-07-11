#pragma once

#include <string>
#include <map>

class LocalizationManager
{
public:
	enum class Language { english, french, maxVal = french, langCount = maxVal + 1 };

	static Language currentLanguage;

	static std::map<std::string, std::string> data;

	static void LoadLanguage(Language lang);
	static std::string GetLocale(std::string key);
	static void Clear();

private:
	static void ReadFile(std::string path);
	static void DiscardUntil(std::ifstream* ifile, char target);
	static std::string ReadValue(std::ifstream* ifile);
};

