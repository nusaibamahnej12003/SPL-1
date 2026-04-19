
#include "topic_model.h"
#include "sentiment.h"
#include <fstream>

// ── Recommendation engine ──────────────────────────────────────────
// topic + overall sentiment দেখে recommendation দেয়
string getRecommendation(const string& topic, const string& overallEmotion,

                          double avgScore, int pos, int neg, int neu)
{
    // topic specific recommendation
    if (topic == "Health") {
        if (avgScore < -0.2)
            return "Health concerns detected. Consider consulting a doctor.";
        else if (avgScore > 0.2)
            return "Positive health outlook! Keep maintaining healthy habits.";
        else
            return "Mixed health sentiments. Monitor your wellbeing regularly.";
    }
    if (topic == "Education") {
        if (avgScore > 0.2)
            return "Great learning attitude! Keep up the academic engagement.";
        else if (avgScore < -0.2)
            return "Learning challenges detected. Seek academic support.";
        else
            return "Neutral academic sentiment. Try to stay motivated.";
    }
    if (topic == "Sports") {
        if (avgScore > 0.2)
            return "High sports enthusiasm! Great for physical fitness.";
        else if (avgScore < -0.2)
            return "Sports frustration noted. Take a break and recharge.";
        else
            return "Moderate sports engagement detected.";
    }
    if (topic == "Technology") {
        if (avgScore > 0.2)
            return "Positive tech outlook! Stay updated with latest trends.";
        else if (avgScore < -0.2)
            return "Tech concerns noted. Evaluate tools and solutions carefully.";
        else
            return "Neutral tech sentiment. Explore more tech opportunities .";
    }
    if (topic == "Entertainment") {
        if (avgScore > 0.2)
            return "Great entertainment mood! Enjoy your leisure time.";
        else if (avgScore < -0.2)
            return "Entertainment dissatisfaction. Try different content.";
        else
            return "Mixed entertainment experience.";
    }
    // general fallback
    if (neg > pos && neg > neu)
        return "Overall negative sentiment detected. Consider seeking support.";
    if (pos > neg && pos > neu)
        return "Overall positive sentiment! Keep the positive energy going.";
    return "Balanced sentiment. Continue monitoring for trends.";
}
struct Result {
    string sentence;
    string topic;
    string sentiment;
    string emotion;
    string label;
    double score;
};

int main()
{
    cout << "\n" << string(75, '=') << endl;
    cout << "          SIMPLE NLP ANALYSIS SYSTEM" << endl;
    cout << string(75, '=') << endl;

    // ── Load & Train ───────────────────────────────────────────────
    SupervisedLDA     topicModel;
    SentimentAnalyzer sentAnalyzer;

    topicModel.loadData("input.txt");
    topicModel.train();

    // ── Read test.txt ──────────────────────────────────────────────
    vector<string> inputs;
    ifstream testFile("test.txt");
    if (testFile.is_open()) {
        string line;
        while (getline(testFile, line))
            if (!line.empty()) inputs.push_back(line);
        testFile.close();
        cout << "\n[Test] " << inputs.size()
             << " sentences loaded from test.txt\n";
    } else {
        cerr << "[Warning] test.txt not found! Using default sentences.\n";
        inputs = {
            "she reads in class 5",
            "he is very sick today",
            "I love to play cricket",
            "AI market is increasing very highly",
            "This TV show is funny and entertaining",
            "Wireless charging uses electromagnetic fields",
            "HE is very Happy",
            "The Doctor behaviour is very nice",
            "I am reading books for exam",
            "He is a very nice and helpful teacher",
            "The patient recovered after surgery",
            "Machine learning is revolutionizing industries",
            "The match was exciting and thrilling",
            "I feel sad and lonely today",
            "The new technology is absolutely amazing",
        };
    }

    // ── Per-sentence analysis ──────────────────────────────────────
    cout << "\n" << string(75, '-') << endl;
    cout << left
         << setw(32) << "INPUT"
         << setw(14) << "TOPIC"
         << setw(25) << "SENTIMENT"
         << endl;
    cout << string(75, '-') << endl;

    // store results for later use
    struct SentRow {
        string sentence, topic, sentiment, emotion, label;
        double score;
    };
    vector<Result> results;

    for (const string& sentence : inputs) {
        string topic = topicModel.predict(sentence);
        SentimentResult sr = sentAnalyzer.analyze(sentence);

        string sentiment = sr.intensity.empty()
                           ? sr.label
                           : sr.intensity + " " + sr.label;

        string display = sentence.length() > 30
                         ? sentence.substr(0, 27) + "..."
                         : sentence;

        cout << left
             << setw(32) << display
             << setw(14) << topic
             << setw(25) << sentiment
             << endl;

        results.push_back({sentence, topic, sentiment, sr.emotion, sr.label, sr.score});
    }
    cout << string(75, '=') << endl;

    // ── Topic-wise Cluster ─────────────────────────────────────────
    cout << "\n" << string(75, '=') << endl;
    cout << "  TOPIC-WISE CLUSTERS" << endl;
    cout << string(75, '=') << endl;

    map<string, vector<Result>> clusters;
    for (auto& r : results)
        clusters[r.topic].push_back(r);

    for (auto& [topic, rlist] : clusters) {
        cout << "\n  [" << topic << "]  (" << rlist.size() << " sentences)\n";
        cout << "  " << string(60, '-') << endl;

        int tPos=0, tNeg=0, tNeu=0;
        double tScore=0;
        for (auto& r : rlist) {
            string disp = r.sentence.length() > 45
                          ? r.sentence.substr(0,42)+"..."
                          : r.sentence;
            cout << "    • " << left << setw(46) << disp
                 << r.sentiment << endl;
            tScore += r.score;
            if      (r.label=="POSITIVE") tPos++;
            else if (r.label=="NEGATIVE") tNeg++;
            else                           tNeu++;
        }

        // topic level summary
        double avg = tScore / rlist.size();
        string topicMood;
        if      (avg >  0.15) topicMood = "\033[32mPositive\033[0m";
        else if (avg < -0.15) topicMood = "\033[31mNegative\033[0m";
        else                   topicMood = "\033[33mNeutral\033[0m";

        cout << "  " << string(60, '-') << endl;
        cout << "  Mood: " << topicMood
             << "  |  Pos:" << tPos
             << " Neg:" << tNeg
             << " Neu:" << tNeu << endl;

        // recommendation per topic
        cout << "  \033[36mRec:\033[0m "
             << getRecommendation(topic, "", avg, tPos, tNeg, tNeu) << endl;
    }

    // ── Overall Sentiment Summary ──────────────────────────────────
    cout << "\n" << string(75, '=') << endl;
    cout << "  OVERALL SENTIMENT SUMMARY" << endl;
    cout << string(75, '-') << endl;

    int totalPos=0, totalNeg=0, totalNeu=0;
    double totalScore=0;
    map<string,int> emotionCount;

    for (auto& r : results) {
        totalScore += r.score;
        if      (r.label=="POSITIVE") totalPos++;
        else if (r.label=="NEGATIVE") totalNeg++;
        else                           totalNeu++;
        emotionCount[r.emotion]++;
    }

    double avgScore = totalScore / results.size();

    // dominant emotion
    string domEmotion = "Neutral";
    int maxE = 0;
    for (auto& [e,c] : emotionCount)
        if (c > maxE) { maxE = c; domEmotion = e; }

    // overall mood label
    string overallMood, moodColor;
    if      (totalPos > totalNeg && totalPos > totalNeu)
        { overallMood="JOY";     moodColor="\033[32m"; }
    else if (totalNeg > totalPos && totalNeg > totalNeu)
        { overallMood="SAD";     moodColor="\033[31m"; }
    else
        { overallMood="NEUTRAL"; moodColor="\033[33m"; }

    cout << "  Total Sentences : " << results.size() << endl;
    cout << "  Positive        : " << totalPos
         << " (" << fixed << setprecision(1)
         << (100.0*totalPos/results.size()) << "%)" << endl;
    cout << "  Negative        : " << totalNeg
         << " (" << (100.0*totalNeg/results.size()) << "%)" << endl;
    cout << "  Neutral         : " << totalNeu
         << " (" << (100.0*totalNeu/results.size()) << "%)" << endl;
    cout << "  Avg Score       : " << fixed << setprecision(4) << avgScore << endl;
    cout << "  Dominant Emotion: " << domEmotion << endl;
    cout << "  Overall Mood    : "
         << moodColor << overallMood << "\033[0m" << endl;

    // ── Overall Recommendation ─────────────────────────────────────
    cout << "\n" << string(75, '-') << endl;
    cout << "  RECOMMENDATION" << endl;
    cout << string(75, '-') << endl;

    // dominant topic
    string domTopic = "General";
    int maxT = 0;
    for (auto& [t, rlist] : clusters)
        if ((int)rlist.size() > maxT) { maxT = rlist.size(); domTopic = t; }

    cout << "  " << getRecommendation(domTopic, domEmotion,
                                       avgScore, totalPos, totalNeg, totalNeu)
         << endl;

    // emotion based extra advice
    if (domEmotion == "Sadness" || domEmotion == "Fear")
        cout << "  Consider talking to someone you trust or a professional." << endl;
    else if (domEmotion == "Anger")
        cout << "  Take a moment to breathe and reflect before responding." << endl;
    else if (domEmotion == "Joy" || domEmotion == "Surprise")
        cout << "  Keep embracing positivity and share it with others!" << endl;
    else if (domEmotion == "Trust" || domEmotion == "Anticipation")
        cout << "  Stay focused on your goals and trust the process." << endl;

    cout << string(75, '=') << endl;
    return 0;
}