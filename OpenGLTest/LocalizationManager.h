#pragma once

#include <string>
#include <map>

class LocalizationManager
{
public:
	enum class Language { english, french, maxVal = french, langCount = maxVal + 1 };

	static Language currentLanguage;

	static std::map<std::string, std::u8string> data;

	static void LoadLanguage(Language lang);
	static std::u8string GetLocale(std::string key);
	static void Clear();

private:
	static void ReadFile(std::string path);
	static void DiscardUntil(std::basic_ifstream<char8_t>* ifile, char target);
	static std::u8string ReadValue(std::basic_ifstream<char8_t>* ifile);
};

