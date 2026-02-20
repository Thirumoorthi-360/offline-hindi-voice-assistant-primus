#pragma once
#include <string>
#include <map>
#include <vector>

class Enhancer {
public:
    Enhancer();
    std::string preprocess(const std::string& input);
    std::string applyContext(const std::string& input);
    std::string expandAnswer(const std::string& answer);

private:
    std::string lastTopic;
    std::map<std::string, std::string> synonymMap;
    std::map<std::string, std::string> shortExpansions;

    void loadSynonyms();
    void loadExpansions();

    std::string removePunctuation(const std::string& text);
    std::string normalizeSynonyms(const std::string& text);
    std::string expandShortForms(const std::string& text);
    std::string toLower(const std::string& text);
};
