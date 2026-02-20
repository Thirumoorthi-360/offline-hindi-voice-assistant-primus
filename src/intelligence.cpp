/*
 * ============================================================
 *  PRIMUS AI - Enhanced Intelligence Module v2.0
 *  Deep context memory, topic tracking, emotion detection
 * ============================================================
 */

#include "intelligence.h"
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cctype>

using namespace std;

/* ===== KNOWN PERSONS / ENTITIES (for subject memory) ===== */

static const vector<string> KNOWN_PERSONS = {
    "नरेंद्र मोदी", "राहुल गांधी", "अमित शाह", "अरविंद केजरीवाल",
    "ममता बनर्जी", "योगी आदित्यनाथ", "सोनिया गांधी", "मनमोहन सिंह",
    "अटल बिहारी वाजपेयी", "इंदिरा गांधी", "जवाहरलाल नेहरू",
    "अमिताभ बच्चन", "शाहरुख खान", "सलमान खान", "आमिर खान",
    "दीपिका पादुकोण", "प्रियंका चोपड़ा", "रणवीर सिंह",
    "एपीजे अब्दुल कलाम", "सुभाष चंद्र बोस", "भगत सिंह",
    "विराट कोहली", "सचिन तेंदुलकर", "महेंद्र सिंह धोनी",
    "रतन टाटा", "मुकेश अंबानी", "गौतम अडानी"
};

/* ================================================================ */

Intelligence::Intelligence(){
    lastSubject  = "";
    lastTopic    = "";
    currentEmotion = "neutral";
    loadTopicKeywords();
}

/* ===== TOPIC KEYWORDS MAP ===== */

void Intelligence::loadTopicKeywords(){
    topicKeywords = {
        {"राजनीति",    "चुनाव मोदी गांधी भाजपा कांग्रेस संसद सरकार मंत्री विधायक सांसद"},
        {"सिनेमा",     "फिल्म बॉलीवुड अभिनेता अभिनेत्री निर्देशक ऑस्कर फिल्मफेयर"},
        {"भूगोल",      "राज्य नदी पर्वत राजधानी जिला क्षेत्रफल जनसंख्या"},
        {"कानून",      "धारा आईपीसी संविधान अदालत न्यायालय अपराध सजा"},
        {"तकनीक",      "इंटरनेट कंप्यूटर सॉफ्टवेयर एआई मोबाइल प्रोग्रामिंग"},
        {"इतिहास",     "स्वतंत्रता आंदोलन युद्ध साम्राज्य मुगल ब्रिटिश"},
        {"खेल",        "क्रिकेट फुटबॉल ओलंपिक विश्वकप टूर्नामेंट"},
        {"अर्थव्यवस्था","जीडीपी बजट रुपया बैंक व्यापार निर्यात"}
    };
}

/* ===== TOPIC DETECTION ===== */

string Intelligence::detectTopic(const string& input){
    int bestScore = 0;
    string bestTopic = "सामान्य";

    for(auto& [topic, keywords] : topicKeywords){
        int score = 0;
        stringstream ss(keywords);
        string kw;
        while(ss >> kw){
            if(input.find(kw) != string::npos)
                score++;
        }
        if(score > bestScore){
            bestScore = score;
            bestTopic = topic;
        }
    }
    return bestTopic;
}

/* ===== NAMED ENTITY EXTRACTION ===== */

string Intelligence::extractNamedEntity(const string& input){
    for(auto& person : KNOWN_PERSONS){
        if(input.find(person) != string::npos)
            return person;
    }
    // fallback: first 3 words
    stringstream ss(input);
    string w1, w2, w3;
    ss >> w1 >> w2 >> w3;
    if(!w1.empty()){
        string entity = w1;
        if(!w2.empty()) entity += " " + w2;
        if(!w3.empty()) entity += " " + w3;
        return entity;
    }
    return "";
}

/* ===== CONTEXT APPLY (pronouns → real subject) ===== */

string Intelligence::applyContext(const string& input){

    // Hindi pronoun references
    static const vector<string> PRONOUNS = {
        "उसकी", "उसका", "उसके", "उनकी", "उनका", "उनके",
        "वह", "वे", "इनकी", "इनका", "यह", "इसकी", "इसका"
    };

    bool hasPronoun = false;
    for(auto& p : PRONOUNS){
        if(input.find(p) != string::npos){
            hasPronoun = true;
            break;
        }
    }

    if(hasPronoun && !lastSubject.empty()){
        return lastSubject + " की बात करें तो — " + input;
    }

    // "और बताओ" / "आगे बताओ"
    if((input.find("और बताओ") != string::npos ||
        input.find("आगे बताओ") != string::npos ||
        input.find("विस्तार") != string::npos) &&
        !lastTopic.empty())
    {
        return lastTopic + " " + input;
    }

    return input;
}

/* ===== CONTEXT UPDATE (called after successful answer) ===== */

void Intelligence::updateContext(const string& input, const string& response){

    string entity = extractNamedEntity(input);
    if(!entity.empty())
        lastSubject = entity;

    lastTopic = detectTopic(input);

    // Store in rolling history (max 10)
    ConversationTurn turn;
    turn.userInput  = input;
    turn.aiResponse = response;
    turn.topic      = lastTopic;
    turn.subject    = lastSubject;

    auto now = chrono::system_clock::now();
    turn.timestamp  = chrono::duration_cast<chrono::seconds>(
                          now.time_since_epoch()).count();

    history.push_back(turn);
    if(history.size() > 10)
        history.pop_front();
}

/* ===== EMOTION DETECTION ===== */

void Intelligence::detectEmotion(const string& input){

    if(input.find("धन्यवाद") != string::npos ||
       input.find("शुक्रिया")  != string::npos ||
       input.find("thanks")   != string::npos)
        currentEmotion = "warm";

    else if(input.find("नमस्ते") != string::npos ||
            input.find("hello")  != string::npos ||
            input.find("हाय")    != string::npos)
        currentEmotion = "energetic";

    else if(input.find("गुस्सा")  != string::npos ||
            input.find("बेकार")   != string::npos ||
            input.find("गलत")    != string::npos)
        currentEmotion = "calm";

    else if(input.find("दुखी")   != string::npos ||
            input.find("उदास")   != string::npos)
        currentEmotion = "empathetic";

    else if(input.find("शाबाश")  != string::npos ||
            input.find("वाह")    != string::npos)
        currentEmotion = "excited";

    else
        currentEmotion = "neutral";
}

string Intelligence::getEmotion() const{
    return currentEmotion;
}
