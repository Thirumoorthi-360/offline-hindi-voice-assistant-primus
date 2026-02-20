/*
 * ============================================================
 *  PRIMUS AI - Enhanced Main v2.0
 *  Time, Date, Math, Follow-up, Smart routing
 * ============================================================
 */

#include "hindi_ai.h"
#include "tts.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cmath>

using namespace std;

/* ===== HELPERS ===== */

string toLower(string text){
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c){ return tolower(c); });
    return text;
}

bool contains(const string& text, const string& word){
    return text.find(word) != string::npos;
}

/* ===== TIME ===== */

string getCurrentTime(){
    time_t now = time(0);
    tm *ltm = localtime(&now);
    stringstream ss;
    ss << "वर्तमान समय है "
       << ltm->tm_hour << " बजकर "
       << ltm->tm_min  << " मिनट ";
    if(ltm->tm_min == 0) ss << "पूरे।";
    else ss << "।";
    return ss.str();
}

/* ===== DATE ===== */

string getCurrentDate(){
    time_t now = time(0);
    tm *ltm = localtime(&now);

    static const string months[] = {
        "जनवरी","फरवरी","मार्च","अप्रैल","मई","जून",
        "जुलाई","अगस्त","सितंबर","अक्टूबर","नवंबर","दिसंबर"
    };
    static const string days[] = {
        "रविवार","सोमवार","मंगलवार","बुधवार","गुरुवार","शुक्रवार","शनिवार"
    };

    stringstream ss;
    ss << "आज " << days[ltm->tm_wday] << " है। "
       << "तारीख है " << ltm->tm_mday
       << " " << months[ltm->tm_mon]
       << " " << (1900 + ltm->tm_year) << "।";
    return ss.str();
}

/* ===== SIMPLE MATH ===== */

string evaluateMath(const string& input){
    // Detect basic arithmetic in Hindi
    // e.g. "25 और 30 का जोड़" / "100 में से 45 घटाओ"
    double a = 0, b = 0;
    char op = 0;

    if(contains(input, "जोड़") || contains(input, "plus") || contains(input, "+")){
        op = '+';
    } else if(contains(input, "घटाओ") || contains(input, "minus") || contains(input, "-")){
        op = '-';
    } else if(contains(input, "गुणा") || contains(input, "multiply") || contains(input, "*")){
        op = '*';
    } else if(contains(input, "भाग") || contains(input, "divide") || contains(input, "/")){
        op = '/';
    }

    if(op == 0) return "";

    // Extract numbers
    istringstream ss(input);
    string token;
    vector<double> nums;
    while(ss >> token){
        try {
            double n = stod(token);
            nums.push_back(n);
        } catch(...) {}
    }

    if(nums.size() < 2) return "";

    a = nums[0]; b = nums[1];
    double result = 0;
    string opName;

    if(op == '+') { result = a+b; opName = "जोड़"; }
    if(op == '-') { result = a-b; opName = "अंतर"; }
    if(op == '*') { result = a*b; opName = "गुणनफल"; }
    if(op == '/' && b != 0) { result = a/b; opName = "भागफल"; }
    else if(op == '/' && b == 0) return "शून्य से भाग संभव नहीं है।";

    stringstream ans;
    ans << a << " और " << b << " का " << opName << " है: " << result;
    return ans.str();
}

/* ===== MAIN ===== */

int main(){

    const string dbPath = "/home/pi/primus/AI/knowledge.db";

    HindiAI ai(dbPath);

    // Deep Male Hindi Voice
    TTS tts(1.0, 130, 22);

    cerr << "╔══════════════════════════════════════╗\n";
    cerr << "║   PRIMUS AI v2.0 — Enhanced Hindi   ║\n";
    cerr << "║   2000+ Facts: Politics, Cinema,    ║\n";
    cerr << "║   Geography, Law, Technology        ║\n";
    cerr << "╚══════════════════════════════════════╝\n";
    cerr << "Type 'exit' to quit.\n\n";

    string input;

    while(true){

        cout << "\nYou: ";
        cout.flush();

        if(!getline(cin, input)) break;
        if(input == "exit" || input == "बंद") break;
        if(input.empty()) continue;

        string processed = toLower(input);
        string response;

        /* --- GREETING --- */
        if(contains(processed, "hello") ||
           contains(processed, "hey")   ||
           contains(processed, "नमस्ते")||
           contains(processed, "हाय"))
        {
            response = "नमस्ते बॉस! मैं PRIMUS हूँ। भारतीय राजनीति, सिनेमा, "
                       "भूगोल, कानून या तकनीक — किसी भी विषय पर पूछें।";
        }

        /* --- TIME --- */
        else if(contains(processed, "समय") ||
                contains(processed, "टाइम")||
                contains(processed, "time"))
        {
            response = getCurrentTime();
        }

        /* --- DATE --- */
        else if(contains(processed, "तारीख")  ||
                contains(processed, "दिनांक") ||
                contains(processed, "डेट")    ||
                contains(processed, "date"))
        {
            response = getCurrentDate();
        }

        /* --- MATH --- */
        else if(contains(processed, "जोड़")   ||
                contains(processed, "घटाओ")   ||
                contains(processed, "गुणा")   ||
                contains(processed, "भाग")    ||
                contains(processed, "calculate"))
        {
            response = evaluateMath(processed);
            if(response.empty())
                response = ai.generateResponse(input);
        }

        /* --- HELP --- */
        else if(contains(processed, "मदद") ||
                contains(processed, "help"))
        {
            response = "मैं इन विषयों में मदद कर सकता हूँ:\n"
                       "• भारतीय राजनीति — नेता, दल, चुनाव, संसद\n"
                       "• भारतीय सिनेमा — फिल्में, कलाकार, पुरस्कार\n"
                       "• भारतीय भूगोल — राज्य, नदियाँ, पर्वत, राजधानियाँ\n"
                       "• भारतीय कानून — धाराएँ, संविधान, न्यायालय\n"
                       "• तकनीक — इंटरनेट, एआई, कंप्यूटर, मोबाइल\n"
                       "• गणित — जोड़, घटाव, गुणा, भाग\n"
                       "• समय और तारीख\n\n"
                       "बस पूछिए!";
        }

        /* --- AI KNOWLEDGE DB --- */
        else{
            response = ai.generateResponse(input);
        }

        cout << "AI: " << response << "\n";
        cout.flush();

        tts.speak(response);
    }

    cerr << "\nPRIMUS AI बंद हो रहा है। अलविदा!\n";
    return 0;
}
