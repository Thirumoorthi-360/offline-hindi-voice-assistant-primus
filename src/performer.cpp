/*
 * ============================================================
 *  PRIMUS AI v2.0 — Performer
 *  Vocabulary builder, Levenshtein fuzzy match, auto-correct
 * ============================================================
 */

#include "performer.h"
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cctype>

using namespace std;

/* ================= CONSTRUCTOR ================= */

Performer::Performer(){
    loadSynonyms();
}

/* ================= SYNONYMS ================= */

void Performer::loadSynonyms(){
    synonymMap = {
        {"india",           "भारत"},
        {"bharat",          "भारत"},
        {"states",          "राज्य"},
        {"pm",              "प्रधानमंत्री"},
        {"prime minister",  "प्रधानमंत्री"},
        {"cm",              "मुख्यमंत्री"},
        {"president",       "राष्ट्रपति"},
        {"parliament",      "संसद"},
        {"court",           "न्यायालय"},
        {"law",             "कानून"},
        {"section",         "धारा"},
        {"river",           "नदी"},
        {"mountain",        "पर्वत"},
        {"film",            "फिल्म"},
        {"movie",           "फिल्म"},
        {"actor",           "अभिनेता"},
        {"इंडिया",          "भारत"},
        {"नेहरु",           "नेहरू"},
        {"गाँधी",           "गांधी"}
    };
}

/* ================= BUILD VOCAB ================= */

void Performer::buildVocabulary(sqlite3* db){

    vocabulary.clear();

    sqlite3_stmt *stmt;
    string sql = "SELECT question FROM knowledge;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        string q = (const char*)sqlite3_column_text(stmt, 0);
        stringstream ss(q);
        string word;
        while(ss >> word)
            vocabulary.insert(word);
    }

    sqlite3_finalize(stmt);
}

/* ================= FUZZY SIMILARITY ================= */

double Performer::fuzzySimilarity(const string &a, const string &b){

    int m = a.size(), n = b.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1));

    for(int i = 0; i <= m; i++) dp[i][0] = i;
    for(int j = 0; j <= n; j++) dp[0][j] = j;

    for(int i = 1; i <= m; i++)
        for(int j = 1; j <= n; j++)
            dp[i][j] = min({
                dp[i-1][j] + 1,
                dp[i][j-1] + 1,
                dp[i-1][j-1] + (a[i-1] != b[j-1] ? 1 : 0)
            });

    int lev    = dp[m][n];
    int maxLen = max(m, n);

    if(maxLen == 0) return 1.0;
    return 1.0 - (double)lev / maxLen;
}

/* ================= AUTO CORRECT ================= */

string Performer::autoCorrect(const string &word){

    double best      = 0.0;
    string bestMatch = word;

    for(auto &v : vocabulary){

        if(word.size() < 3) continue;

        double sim = fuzzySimilarity(word, v);

        if(sim > best){
            best      = sim;
            bestMatch = v;
        }
    }

    if(best > 0.80)
        return bestMatch;

    return word;
}

/* ================= NORMALIZE QUERY ================= */

string Performer::normalizeQuery(const string &input){

    string s = input;

    // Lowercase English part
    transform(s.begin(), s.end(), s.begin(), ::tolower);

    // Replace synonyms
    for(auto &p : synonymMap){
        size_t pos = s.find(p.first);
        if(pos != string::npos)
            s.replace(pos, p.first.length(), p.second);
    }

    // Remove punctuation (keep Hindi unicode chars)
    string cleaned;
    for(unsigned char c : s){
        if(isalnum(c) || c >= 128 || c == ' ')
            cleaned += (char)c;
    }

    // Token-level auto correction
    stringstream ss(cleaned);
    string word;
    string result;

    while(ss >> word){
        word = autoCorrect(word);
        result += word + " ";
    }

    return result;
}