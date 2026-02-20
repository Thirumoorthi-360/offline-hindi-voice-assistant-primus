#pragma once
#include <string>
#include <vector>
#include <map>
#include <deque>

struct ConversationTurn {
    std::string userInput;
    std::string aiResponse;
    std::string topic;
    std::string subject;
    long long timestamp;
};

class Intelligence {
public:
    Intelligence();

    std::string applyContext(const std::string& input);
    void updateContext(const std::string& input, const std::string& response = "");
    void detectEmotion(const std::string& input);
    std::string getEmotion() const;

    // Enhanced context
    std::string getLastSubject() const { return lastSubject; }
    std::string getLastTopic()   const { return lastTopic; }
    std::vector<ConversationTurn> getHistory() const { return history; }

    // Topic detection
    std::string detectTopic(const std::string& input);

    // Named entity extraction
    std::string extractNamedEntity(const std::string& input);

private:
    std::string lastSubject;
    std::string lastTopic;
    std::string currentEmotion;
    std::deque<ConversationTurn> history;   // last 10 turns
    std::map<std::string, std::string> topicKeywords;

    void loadTopicKeywords();
};
