/*
 * ============================================================
 *  PRIMUS AI - Enhanced Hindi AI Core v2.0
 *  TF-IDF style scoring, category search, smart wrappers
 * ============================================================
 */

#include "hindi_ai.h"
#include "enhancer.h"
#include "intelligence.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>
#include <cstdlib>
#include <ctime>
#include <cctype>

using namespace std;

static Enhancer    enhancer;
static Intelligence brain;

/* ===== STOP WORDS (Hindi + English) ===== */

static const set<string> STOP_WORDS = {
    "क्या","है","हैं","था","थे","की","के","का","में","और","या",
    "से","पर","को","ने","यह","वह","एक","कौन","कब","कहाँ","कैसे",
    "the","is","of","in","a","an","what","who","when","where","how"
};

/* ===== RANDOM TEMPLATE ===== */

static string randomFrom(const vector<string>& v){
    static bool seeded = false;
    if(!seeded){ srand(time(0)); seeded = true; }
    return v[rand() % v.size()];
}

/* ================================================================
   CONSTRUCTOR / DESTRUCTOR
================================================================ */

HindiAI::HindiAI(const string& dbFile){
    if(sqlite3_open(dbFile.c_str(), &db) != SQLITE_OK){
        cerr << "Database open failed: " << sqlite3_errmsg(db) << "\n";
        db = nullptr;
    } else {
        // Enable WAL mode for faster reads
        sqlite3_exec(db, "PRAGMA journal_mode=WAL;", 0,0,0);
        sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", 0,0,0);
        // Create FTS table if not exists for full text search
        sqlite3_exec(db,
            "CREATE VIRTUAL TABLE IF NOT EXISTS knowledge_fts "
            "USING fts5(question, answer, category, content='knowledge', "
            "content_rowid='id');",
            0,0,0);
    }
}

HindiAI::~HindiAI(){
    if(db) sqlite3_close(db);
}

/* ================================================================
   TOKENIZE
================================================================ */

vector<string> HindiAI::tokenize(const string& text){
    stringstream ss(text);
    string word;
    vector<string> tokens;
    while(ss >> word){
        if(!isStopWord(word))
            tokens.push_back(word);
    }
    return tokens;
}

bool HindiAI::isStopWord(const string& w){
    return STOP_WORDS.count(w) > 0;
}

/* ================================================================
   SCORE MATCH (TF-IDF style)
   - Each matching token = +2
   - Exact phrase match bonus = +10
   - Partial (substr) match = +1
================================================================ */

int HindiAI::scoreMatch(const string& query, const string& dbQuestion){
    int score = 0;

    // Exact phrase match bonus
    if(dbQuestion.find(query) != string::npos)
        score += 10;

    auto tokens = tokenize(query);
    for(auto& tok : tokens){
        if(tok.size() < 2) continue;
        if(dbQuestion.find(tok) != string::npos)
            score += 2;
    }

    return score;
}

/* ================================================================
   SEARCH DB — Primary method
================================================================ */

string HindiAI::searchDB(const string& query){
    if(!db) return "";

    sqlite3_stmt* stmt;
    // Also try FTS first (much faster on large DB)
    string fts_sql =
        "SELECT knowledge.answer FROM knowledge "
        "JOIN knowledge_fts ON knowledge.id = knowledge_fts.rowid "
        "WHERE knowledge_fts MATCH ? "
        "ORDER BY rank LIMIT 1;";

    if(sqlite3_prepare_v2(db, fts_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK){
        sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_TRANSIENT);
        if(sqlite3_step(stmt) == SQLITE_ROW){
            string ans = (const char*)sqlite3_column_text(stmt, 0);
            sqlite3_finalize(stmt);
            if(!ans.empty()) return ans;
        }
        sqlite3_finalize(stmt);
    }

    // Fallback: full scan with scoring
    return searchByKeyword(query);
}

/* ================================================================
   SEARCH BY KEYWORD (scored full scan)
================================================================ */

string HindiAI::searchByKeyword(const string& query){
    if(!db) return "";

    sqlite3_stmt* stmt;
    string sql = "SELECT question, answer, category FROM knowledge;";

    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return "";

    string bestAnswer   = "";
    int    bestScore    = 0;
    const int MIN_SCORE = 1;  // require at least 1 token match

    while(sqlite3_step(stmt) == SQLITE_ROW){
        string dbQ = (const char*)sqlite3_column_text(stmt, 0);
        string dbA = (const char*)sqlite3_column_text(stmt, 1);

        int score = scoreMatch(query, dbQ);

        if(score > bestScore){
            bestScore  = score;
            bestAnswer = dbA;
        }
    }

    sqlite3_finalize(stmt);

    if(bestScore >= MIN_SCORE)
        return bestAnswer;

    return "";
}

/* ================================================================
   SEARCH BY CATEGORY
================================================================ */

string HindiAI::searchByCategory(const string& category, const string& query){
    if(!db || category.empty()) return "";

    sqlite3_stmt* stmt;
    string sql = "SELECT question, answer FROM knowledge WHERE category=?;";

    if(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return "";

    sqlite3_bind_text(stmt, 1, category.c_str(), -1, SQLITE_TRANSIENT);

    string bestAnswer = "";
    int    bestScore  = 0;

    while(sqlite3_step(stmt) == SQLITE_ROW){
        string dbQ = (const char*)sqlite3_column_text(stmt, 0);
        string dbA = (const char*)sqlite3_column_text(stmt, 1);
        int score  = scoreMatch(query, dbQ);
        if(score > bestScore){
            bestScore  = score;
            bestAnswer = dbA;
        }
    }

    sqlite3_finalize(stmt);
    return bestAnswer;
}

/* ================================================================
   WRAP RESPONSE with emotion-aware suffix
================================================================ */

string HindiAI::wrapResponse(const string& answer, const string& emotion){
    vector<string> neutral = {
        answer + "। यदि आप चाहें तो मैं और विस्तार से समझा सकता हूँ।",
        answer + "। क्या आप इस विषय पर और जानकारी चाहते हैं?",
        answer + "। इस विषय में कोई और जिज्ञासा हो तो पूछें।"
    };
    vector<string> warm = {
        answer + "। आपकी जिज्ञासा अच्छी है, और जानकारी के लिए पूछें।",
        answer + "। यह जानकारी आपके काम आए, यही मेरी कोशिश है।"
    };
    vector<string> energetic = {
        answer + "। बढ़िया सवाल! और पूछें।",
        answer + "। शानदार! इस विषय पर और बात करें।"
    };
    vector<string> calm = {
        answer + "। आशा है यह जानकारी सहायक होगी।",
        answer + "। शांत मन से इसे समझें, और प्रश्न हो तो पूछें।"
    };

    if(emotion == "warm")      return randomFrom(warm);
    if(emotion == "energetic") return randomFrom(energetic);
    if(emotion == "calm")      return randomFrom(calm);
    return randomFrom(neutral);
}

/* ================================================================
   GENERATE RESPONSE — Main entry point
================================================================ */

string HindiAI::generateResponse(const string& input){

    // 1. Preprocess
    string processed = enhancer.preprocess(input);

    // 2. Detect emotion
    brain.detectEmotion(processed);
    string emotion = brain.getEmotion();

    // 3. Apply context (pronoun resolution)
    processed = brain.applyContext(processed);

    // 4. Detect topic for category search
    string topic = brain.detectTopic(processed);

    /* --- Greetings --- */
    if(processed.find("नमस्ते") != string::npos ||
       processed.find("hello")  != string::npos ||
       processed.find("हाय")    != string::npos ||
       processed.find("hey")    != string::npos)
    {
        vector<string> greets = {
            "नमस्ते बॉस! मैं पूरी तरह सक्रिय हूँ। भारतीय राजनीति, सिनेमा, भूगोल, कानून या तकनीक — किसी भी विषय पर पूछें।",
            "हेलो! मैं आपकी सेवा में हूँ। क्या जानना चाहते हैं?",
            "नमस्कार! आज किस विषय में जानकारी चाहिए?"
        };
        return randomFrom(greets);
    }

    /* --- Help --- */
    if(processed.find("मदद") != string::npos ||
       processed.find("help") != string::npos)
    {
        return "मैं इन विषयों में मदद कर सकता हूँ:\n"
               "1. भारतीय राजनीति (नेता, दल, चुनाव, संसद)\n"
               "2. भारतीय सिनेमा (फिल्में, कलाकार, पुरस्कार)\n"
               "3. भारतीय भूगोल (राज्य, नदियाँ, पर्वत)\n"
               "4. भारतीय कानून (धाराएँ, संविधान, न्यायालय)\n"
               "5. तकनीक (इंटरनेट, एआई, प्रोग्रामिंग)\n"
               "बस पूछिए!";
    }

    /* --- "और बताओ" (follow-up) --- */
    if(processed.find("और बताओ") != string::npos ||
       processed.find("विस्तार") != string::npos)
    {
        string lastSubj = brain.getLastSubject();
        if(!lastSubj.empty()){
            string followUp = searchDB(lastSubj + " विस्तार " + processed);
            if(followUp.empty()) followUp = searchDB(lastSubj);
            if(!followUp.empty()){
                brain.updateContext(input, followUp);
                return wrapResponse(followUp, emotion);
            }
        }
    }

    // 5. Primary search
    string answer = searchDB(processed);

    // 6. Category fallback
    if(answer.empty() && !topic.empty() && topic != "सामान्य"){
        answer = searchByCategory(topic, processed);
    }

    // 7. Success
    if(!answer.empty()){
        brain.updateContext(input, answer);
        return wrapResponse(answer, emotion);
    }

    // 8. Not found
    return "क्षमा कीजिए बॉस, इस विषय पर मेरे पास अभी जानकारी नहीं है। "
           "कृपया अलग शब्दों में पूछें या किसी और विषय पर प्रश्न करें।";
}
