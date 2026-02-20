/*
 * ============================================================
 *  PRIMUS AI - Enhanced Enhancer v2.0
 *  Synonym expansion, short-form handling, normalization
 * ============================================================
 */

#include "enhancer.h"
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

Enhancer::Enhancer(){
    lastTopic = "";
    loadSynonyms();
    loadExpansions();
}

/* ===== SYNONYMS ===== */

void Enhancer::loadSynonyms(){
    synonymMap = {
        // English → Hindi
        {"india",           "भारत"},
        {"bharat",          "भारत"},
        {"pm",              "प्रधानमंत्री"},
        {"prime minister",  "प्रधानमंत्री"},
        {"cm",              "मुख्यमंत्री"},
        {"chief minister",  "मुख्यमंत्री"},
        {"president",       "राष्ट्रपति"},
        {"parliament",      "संसद"},
        {"constitution",    "संविधान"},
        {"court",           "न्यायालय"},
        {"supreme court",   "सर्वोच्च न्यायालय"},
        {"high court",      "उच्च न्यायालय"},
        {"state",           "राज्य"},
        {"capital",         "राजधानी"},
        {"river",           "नदी"},
        {"mountain",        "पर्वत"},
        {"film",            "फिल्म"},
        {"movie",           "फिल्म"},
        {"actor",           "अभिनेता"},
        {"actress",         "अभिनेत्री"},
        {"director",        "निर्देशक"},
        {"award",           "पुरस्कार"},
        {"oscar",           "ऑस्कर"},
        {"law",             "कानून"},
        {"section",         "धारा"},
        {"ipc",             "आईपीसी"},
        {"technology",      "तकनीक"},
        {"computer",        "कंप्यूटर"},
        {"internet",        "इंटरनेट"},
        {"mobile",          "मोबाइल"},
        {"artificial intelligence", "कृत्रिम बुद्धिमत्ता"},
        {"ai",              "कृत्रिम बुद्धिमत्ता"},
        // Alternate spellings / variants
        {"इंडिया",          "भारत"},
        {"नेहरु",           "नेहरू"},
        {"गाँधी",           "गांधी"},
    };
}

/* ===== SHORT FORM EXPANSIONS ===== */

void Enhancer::loadExpansions(){
    shortExpansions = {
        {"बीजेपी",   "भारतीय जनता पार्टी"},
        {"bjp",      "भारतीय जनता पार्टी"},
        {"कांग्रेस", "भारतीय राष्ट्रीय कांग्रेस"},
        {"inc",      "भारतीय राष्ट्रीय कांग्रेस"},
        {"आप",       "आम आदमी पार्टी"},
        {"aap",      "आम आदमी पार्टी"},
        {"आईपीसी",   "भारतीय दंड संहिता"},
        {"सीआरपीसी", "दंड प्रक्रिया संहिता"},
        {"crpc",     "दंड प्रक्रिया संहिता"},
        {"यूपी",     "उत्तर प्रदेश"},
        {"up",       "उत्तर प्रदेश"},
        {"एमपी",     "मध्य प्रदेश"},
        {"mp",       "मध्य प्रदेश"},
        {"जीडीपी",   "सकल घरेलू उत्पाद"},
        {"gdp",      "सकल घरेलू उत्पाद"},
        {"isro",     "भारतीय अंतरिक्ष अनुसंधान संगठन"},
        {"इसरो",     "भारतीय अंतरिक्ष अनुसंधान संगठन"},
        {"nato",     "उत्तरी अटलांटिक संधि संगठन"},
        {"un",       "संयुक्त राष्ट्र"},
        {"cbi",      "केंद्रीय जांच ब्यूरो"},
        {"सीबीआई",   "केंद्रीय जांच ब्यूरो"},
        {"rti",      "सूचना का अधिकार"},
        {"आरटीआई",   "सूचना का अधिकार"},
    };
}

/* ===== LOWERCASE (ASCII only) ===== */

string Enhancer::toLower(const string& text){
    string result = text;
    transform(result.begin(), result.end(), result.begin(),
              [](unsigned char c){ return tolower(c); });
    return result;
}

/* ===== REMOVE PUNCTUATION ===== */

string Enhancer::removePunctuation(const string& text){
    string result;
    for(unsigned char c : text){
        if(isalnum(c) || c >= 128 || c == ' ')
            result += (char)c;
    }
    return result;
}

/* ===== NORMALIZE SYNONYMS ===== */

string Enhancer::normalizeSynonyms(const string& text){
    string s = text;
    for(auto& [from, to] : synonymMap){
        size_t pos;
        while((pos = s.find(from)) != string::npos)
            s.replace(pos, from.length(), to);
    }
    return s;
}

/* ===== EXPAND SHORT FORMS ===== */

string Enhancer::expandShortForms(const string& text){
    string s = text;
    for(auto& [abbr, full] : shortExpansions){
        size_t pos;
        while((pos = s.find(abbr)) != string::npos)
            s.replace(pos, abbr.length(), full);
    }
    return s;
}

/* ===== PREPROCESS (main pipeline) ===== */

string Enhancer::preprocess(const string& input){
    string s = toLower(input);
    s = normalizeSynonyms(s);
    s = expandShortForms(s);
    s = removePunctuation(s);
    return s;
}

/* ===== CONTEXT APPLY ===== */

string Enhancer::applyContext(const string& input){
    if(input.find("उसकी") != string::npos ||
       input.find("उसका") != string::npos ||
       input.find("वह")   != string::npos)
    {
        if(!lastTopic.empty())
            return lastTopic + " " + input;
    }
    lastTopic = input;
    return input;
}

/* ===== EXPAND ANSWER ===== */

string Enhancer::expandAnswer(const string& answer){
    if(answer.find("उपलब्ध नहीं") != string::npos)
        return answer;
    return answer + " यह एक महत्वपूर्ण तथ्य है।";
}
