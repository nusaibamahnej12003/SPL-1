#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <iomanip>
using namespace std;

// ═══════════════════════════════════════════════════════════════════
//  SENTIMENT ANALYSIS
//  VADER-inspired lexicon + rules + emotion detection
// ═══════════════════════════════════════════════════════════════════

struct SentimentResult
{
    double score;
    double positive;
    double negative;
    double neutral;
    string label;
    string intensity;
    string emotion;
};

class SentimentAnalyzer
{
private:
    map<string, double> lexicon;
    set<string>         negWords;
    map<string, double> intensifiers;
    map<string, string> emotionMap;

    void buildLexicon()
    {
        // Positive words
        lexicon["excellent"]=3.5; lexicon["outstanding"]=3.5; lexicon["amazing"]=3.4;
        lexicon["fantastic"]=3.4; lexicon["wonderful"]=3.3;   lexicon["brilliant"]=3.3;
        lexicon["superb"]=3.2;    lexicon["perfect"]=3.1;     lexicon["incredible"]=3.1;
        lexicon["awesome"]=2.8;   lexicon["great"]=2.5;       lexicon["love"]=2.5;
        lexicon["loved"]=2.5;     lexicon["good"]=2.0;        lexicon["happy"]=2.1;
        lexicon["happiness"]=2.2; lexicon["joy"]=2.3;         lexicon["joyful"]=2.3;
        lexicon["enjoy"]=2.0;     lexicon["enjoyed"]=2.0;     lexicon["fun"]=2.0;
        lexicon["funny"]=2.0;     lexicon["nice"]=1.8;        lexicon["like"]=1.5;
        lexicon["liked"]=1.5;     lexicon["best"]=2.6;        lexicon["better"]=1.8;
        lexicon["helpful"]=1.8;   lexicon["pleased"]=2.0;     lexicon["glad"]=1.9;
        lexicon["excited"]=2.2;   lexicon["impressive"]=2.3;  lexicon["recommend"]=1.9;
        lexicon["useful"]=1.7;    lexicon["smart"]=1.8;       lexicon["efficient"]=1.8;
        lexicon["reliable"]=1.8;  lexicon["success"]=2.1;     lexicon["successful"]=2.1;
        lexicon["winning"]=2.0;   lexicon["win"]=2.0;         lexicon["delight"]=2.2;
        lexicon["delightful"]=2.2;lexicon["charming"]=2.0;    lexicon["cheerful"]=2.0;
        lexicon["ok"]=0.9;        lexicon["okay"]=0.9;        lexicon["alright"]=0.8;
        lexicon["fair"]=0.5;      lexicon["positive"]=1.9;    lexicon["beautiful"]=2.1;
        lexicon["safe"]=1.5;      lexicon["creative"]=1.8;    lexicon["honest"]=1.6;
        lexicon["hope"]=1.7;      lexicon["hopeful"]=1.8;     lexicon["strong"]=1.5;
        lexicon["healthy"]=1.7;   lexicon["comfortable"]=1.8; lexicon["friendly"]=1.9;
        lexicon["clean"]=1.5;     lexicon["smooth"]=1.7;      lexicon["worthy"]=1.6;
        lexicon["marvelous"]=2.9; lexicon["spectacular"]=2.9; lexicon["phenomenal"]=3.0;
        lexicon["magnificent"]=3.0;lexicon["splendid"]=2.8;

        // Negative words
        lexicon["terrible"]=-3.5; lexicon["horrible"]=-3.5;   lexicon["awful"]=-3.4;
        lexicon["disgusting"]=-3.3;lexicon["abysmal"]=-3.2;   lexicon["appalling"]=-3.2;
        lexicon["bad"]=-2.0;      lexicon["poor"]=-2.0;       lexicon["hate"]=-2.5;
        lexicon["hated"]=-2.5;    lexicon["sad"]=-2.0;        lexicon["unhappy"]=-2.1;
        lexicon["angry"]=-2.2;    lexicon["anger"]=-2.2;      lexicon["worst"]=-2.7;
        lexicon["worse"]=-1.9;    lexicon["disappointed"]=-2.1;lexicon["disappointing"]=-2.1;
        lexicon["disappointment"]=-2.1; lexicon["frustrated"]=-2.0;
        lexicon["frustrating"]=-2.0;    lexicon["frustration"]=-2.0;
        lexicon["annoying"]=-1.8; lexicon["annoyed"]=-1.8;    lexicon["boring"]=-1.7;
        lexicon["waste"]=-2.0;    lexicon["broken"]=-2.0;     lexicon["fail"]=-2.1;
        lexicon["failed"]=-2.1;   lexicon["failure"]=-2.1;    lexicon["ugly"]=-2.0;
        lexicon["dirty"]=-1.8;    lexicon["slow"]=-1.5;       lexicon["rude"]=-2.1;
        lexicon["useless"]=-2.2;  lexicon["worthless"]=-2.3;  lexicon["painful"]=-2.0;
        lexicon["pain"]=-1.9;     lexicon["sick"]=-1.8;       lexicon["problem"]=-1.5;
        lexicon["crash"]=-2.0;    lexicon["wrong"]=-1.7;      lexicon["confusing"]=-1.5;
        lexicon["regret"]=-2.1;   lexicon["scam"]=-3.0;       lexicon["fraud"]=-3.0;
        lexicon["mediocre"]=-1.2; lexicon["unreliable"]=-1.5; lexicon["expensive"]=-1.2;
        lexicon["lie"]=-2.5;      lexicon["cheat"]=-2.7;      lexicon["fear"]=-2.0;
        lexicon["scared"]=-1.9;   lexicon["worried"]=-1.8;    lexicon["stress"]=-1.8;
        lexicon["shame"]=-2.0;    lexicon["guilt"]=-1.9;      lexicon["hopeless"]=-2.3;
        lexicon["lonely"]=-2.0;   lexicon["depressed"]=-2.4;  lexicon["miserable"]=-2.5;
        lexicon["dreadful"]=-2.9; lexicon["catastrophic"]=-3.1;lexicon["disastrous"]=-3.1;
    }

    void buildNegWords()
    {
        negWords = {"not","no","never","neither","nor","nothing","nobody",
                    "nowhere","without","dont","doesnt","didnt","wasnt","werent",
                    "isnt","arent","cant","cannot","wont","wouldnt","shouldnt",
                    "couldnt","hardly","barely","scarcely"};
    }

    void buildIntensifiers()
    {
        intensifiers["very"]=1.3;       intensifiers["extremely"]=1.5;
        intensifiers["incredibly"]=1.5; intensifiers["absolutely"]=1.4;
        intensifiers["totally"]=1.3;    intensifiers["completely"]=1.3;
        intensifiers["highly"]=1.3;     intensifiers["deeply"]=1.3;
        intensifiers["really"]=1.2;     intensifiers["so"]=1.2;
        intensifiers["quite"]=1.1;      intensifiers["pretty"]=1.1;
        intensifiers["fairly"]=0.9;     intensifiers["somewhat"]=0.8;
        intensifiers["slightly"]=0.7;   intensifiers["rather"]=0.9;
    }

    void buildEmotionMap()
    {
        emotionMap["happy"]="Joy";    emotionMap["joy"]="Joy";
        emotionMap["excited"]="Joy";  emotionMap["love"]="Joy";
        emotionMap["delight"]="Joy";  emotionMap["cheerful"]="Joy";
        emotionMap["great"]="Joy";    emotionMap["wonderful"]="Joy";
        emotionMap["fun"]="Joy";      emotionMap["funny"]="Joy";
        emotionMap["angry"]="Anger";  emotionMap["hate"]="Anger";
        emotionMap["frustrated"]="Anger"; emotionMap["annoyed"]="Anger";
        emotionMap["rage"]="Anger";   emotionMap["furious"]="Anger";
        emotionMap["rude"]="Anger";
        emotionMap["sad"]="Sadness";  emotionMap["unhappy"]="Sadness";
        emotionMap["depressed"]="Sadness"; emotionMap["lonely"]="Sadness";
        emotionMap["miserable"]="Sadness"; emotionMap["hopeless"]="Sadness";
        emotionMap["disappointed"]="Sadness"; emotionMap["regret"]="Sadness";
        emotionMap["scared"]="Fear";  emotionMap["fear"]="Fear";
        emotionMap["worried"]="Fear"; emotionMap["anxious"]="Fear";
        emotionMap["terrified"]="Fear"; emotionMap["nervous"]="Fear";
        emotionMap["amazing"]="Surprise"; emotionMap["incredible"]="Surprise";
        emotionMap["unexpected"]="Surprise"; emotionMap["shocking"]="Surprise";
        emotionMap["awesome"]="Surprise";
        emotionMap["disgusting"]="Disgust"; emotionMap["horrible"]="Disgust";
        emotionMap["awful"]="Disgust";  emotionMap["terrible"]="Disgust";
        emotionMap["nasty"]="Disgust";  emotionMap["ugly"]="Disgust";
    }

    string norm(const string& w)
    {
        string r = "";
        for (char c : w) if (isalpha(c)) r += tolower(c);
        return r;
    }

    string detectEmotion(const vector<string>& tokens)
    {
        map<string, int> emotionCount;
        for (const string& t : tokens) {
            string w = norm(t);
            if (emotionMap.count(w))
                emotionCount[emotionMap[w]]++;
        }
        if (emotionCount.empty()) return "Neutral";
        return max_element(emotionCount.begin(), emotionCount.end(),
            [](const pair<string,int>& a, const pair<string,int>& b){
                return a.second < b.second;
            })->first;
    }

public:
    SentimentAnalyzer()
    {
        buildLexicon();
        buildNegWords();
        buildIntensifiers();
        buildEmotionMap();
    }

    SentimentResult analyze(const string& text)
    {
        stringstream ss(text);
        vector<string> tokens;
        string w;
        while (ss >> w) tokens.push_back(w);

        double raw = 0.0, posSum = 0.0, negSum = 0.0;
        int cnt = 0;

        for (int i = 0; i < (int)tokens.size(); i++) {
            string word = norm(tokens[i]);
            if (word.empty() || !lexicon.count(word)) continue;

            double ws = lexicon[word];
            cnt++;

            bool allCaps = true;
            for (char c : tokens[i]) if (isalpha(c) && !isupper(c)) { allCaps=false; break; }
            if (allCaps && tokens[i].length() > 1)
                ws *= (ws > 0) ? 1.2 : 0.8;

            if (i > 0 && intensifiers.count(norm(tokens[i-1])))
                ws *= intensifiers[norm(tokens[i-1])];

            for (int j = max(0,i-3); j < i; j++)
                if (negWords.count(norm(tokens[j]))) { ws *= -0.74; break; }

            int exc = 0;
            for (char c : text) if (c == '!') exc++;
            if (exc > 0) ws += (ws>0?1:-1) * min(exc,3) * 0.292;

            raw += ws;
            if (ws > 0) posSum += ws;
            if (ws < 0) negSum += fabs(ws);
        }

        double score = (cnt==0) ? 0.0 : raw / sqrt(raw*raw + 15.0);
        score = max(-1.0, min(1.0, score));

        double total = posSum + negSum + 1e-9;
        double posR  = posSum/total, negR = negSum/total;
        double neuR  = max(0.0, 1.0 - posR - negR);
        double pct   = posR + negR + neuR;
        posR/=pct; negR/=pct; neuR/=pct;

        string label, intensity;
        double a = fabs(score);
        if      (score >=  0.05) label = "POSITIVE";
        else if (score <= -0.05) label = "NEGATIVE";
        else                      label = "NEUTRAL";
        if      (a >= 0.75) intensity = "Strongly";
        else if (a >= 0.50) intensity = "Moderately";
        else if (a >= 0.25) intensity = "Mildly";
        else if (a >= 0.05) intensity = "Slightly";
        else                 intensity = "";

        string emotion = detectEmotion(tokens);
        return {score, posR*100, negR*100, neuR*100, label, intensity, emotion};
    }
};