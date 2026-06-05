#include "languages.hpp"

// Static class member
Language LanguagedText::currentLanguage = Language::Default;


LanguagedText::LanguagedText(const std::string engVar, const std::string rusVar)
: eng(engVar), rus(rusVar) {}

bool LanguagedText::setCurrentLanguage(Language newLanguage) {
    if (newLanguage != currentLanguage) {
        currentLanguage = newLanguage;
        return true;
    }
    return false;
}

Language LanguagedText::getLanguage() {
    return currentLanguage;
}

const std::string& LanguagedText::getString() {
    switch (currentLanguage) {
    case Language::English:
        return eng;
    
    case Language::Russian:
        return rus;

    default:
        return eng;
    }
}

sf::String LanguagedText::getUTF8() {
    return sf::String::fromUtf8(getString().begin(), getString().end());
}
