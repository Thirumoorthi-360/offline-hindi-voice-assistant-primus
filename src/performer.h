#pragma once
#include <string>
#include <set>
#include <map>
#include <sqlite3.h>

class Performer {
public:
    Performer();

    void   buildVocabulary(sqlite3* db);
    double fuzzySimilarity(const std::string& a, const std::string& b);
    std::string autoCorrect(const std::string& word);
    std::string normalizeQuery(const std::string& input);

private:
    std::set<std::string>        vocabulary;
    std::map<std::string,std::string> synonymMap;

    void loadSynonyms();
};
