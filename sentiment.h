/*#pragma once
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

};*/

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



struct SentimentResult
{
    double score;        // -1.0 to +1.0
    double positive;     // % positive
    double negative;     // % negative
    double neutral;      // % neutral
    double confidence;   // ratio of sentiment words found
    string label;        // POSITIVE / NEGATIVE / NEUTRAL
    string intensity;    // Strongly / Moderately / Mildly / Slightly
    string emotion;      // Joy/Anger/Sadness/Fear/Surprise/Disgust/Trust/Anticipation
};

class SentimentAnalyzer
{
private:
    map<string, double> lexicon;
    map<string, double> intensifiers;
    map<string, double> diminishers;
    map<string, string> emotionMap;
    set<string>         negWords;
    set<string>         contrastWords;

    void buildLexicon()
    {
        // ── Strong Positive (3.0 ~ 3.5) ───────────────────────────
        lexicon["excellent"]=3.5;    lexicon["outstanding"]=3.5;
        lexicon["exceptional"]=3.5;  lexicon["extraordinary"]=3.4;
        lexicon["amazing"]=3.4;      lexicon["fantastic"]=3.4;
        lexicon["wonderful"]=3.3;    lexicon["brilliant"]=3.3;
        lexicon["superb"]=3.2;       lexicon["perfect"]=3.1;
        lexicon["incredible"]=3.1;   lexicon["phenomenal"]=3.0;
        lexicon["magnificent"]=3.0;  lexicon["marvelous"]=2.9;
        lexicon["spectacular"]=2.9;  lexicon["splendid"]=2.8;
        lexicon["glorious"]=2.8;     lexicon["sublime"]=2.8;
        lexicon["flawless"]=3.0;     lexicon["masterpiece"]=3.2;
        lexicon["legendary"]=3.0;    lexicon["breathtaking"]=3.1;
        lexicon["astounding"]=3.0;   lexicon["remarkable"]=2.8;

        // ── Moderate Positive (2.0 ~ 2.9) ─────────────────────────
        lexicon["awesome"]=2.8;      lexicon["great"]=2.5;
        lexicon["love"]=2.5;         lexicon["loved"]=2.5;
        lexicon["loving"]=2.4;       lexicon["best"]=2.6;
        lexicon["happy"]=2.1;        lexicon["happiness"]=2.2;
        lexicon["joy"]=2.3;          lexicon["joyful"]=2.3;
        lexicon["joyous"]=2.3;       lexicon["excited"]=2.2;
        lexicon["exciting"]=2.2;     lexicon["delight"]=2.2;
        lexicon["delightful"]=2.2;   lexicon["cheerful"]=2.0;
        lexicon["enjoy"]=2.0;        lexicon["enjoyed"]=2.0;
        lexicon["enjoyable"]=2.0;    lexicon["fun"]=2.0;
        lexicon["funny"]=2.0;        lexicon["impressive"]=2.3;
        lexicon["pleased"]=2.0;      lexicon["glad"]=1.9;
        lexicon["recommend"]=1.9;    lexicon["winning"]=2.0;
        lexicon["win"]=2.0;          lexicon["won"]=2.0;
        lexicon["charming"]=2.0;     lexicon["elegant"]=2.1;
        lexicon["graceful"]=2.0;     lexicon["vibrant"]=2.0;
        lexicon["thrilling"]=2.3;    lexicon["inspiring"]=2.2;
        lexicon["motivated"]=2.0;    lexicon["energetic"]=2.0;
        lexicon["enthusiastic"]=2.1; lexicon["passionate"]=2.1;
        lexicon["proud"]=2.0;        lexicon["grateful"]=2.1;
        lexicon["thankful"]=2.0;     lexicon["blessed"]=2.2;
        lexicon["fortunate"]=2.0;    lexicon["lucky"]=1.9;
        lexicon["celebrate"]=2.1;    lexicon["celebration"]=2.1;
        lexicon["triumph"]=2.3;      lexicon["victorious"]=2.4;
        lexicon["achievement"]=2.2;  lexicon["accomplished"]=2.1;
        lexicon["succeed"]=2.1;      lexicon["success"]=2.1;
        lexicon["successful"]=2.1;   lexicon["innovative"]=2.0;
        lexicon["creative"]=1.8;

        // ── Mild Positive (0.5 ~ 1.9) ─────────────────────────────
        lexicon["good"]=2.0;         lexicon["nice"]=1.8;
        lexicon["like"]=1.5;         lexicon["liked"]=1.5;
        lexicon["better"]=1.8;       lexicon["helpful"]=1.8;
        lexicon["useful"]=1.7;       lexicon["smart"]=1.8;
        lexicon["efficient"]=1.8;    lexicon["reliable"]=1.8;
        lexicon["honest"]=1.6;       lexicon["hope"]=1.7;
        lexicon["hopeful"]=1.8;      lexicon["strong"]=1.5;
        lexicon["healthy"]=1.7;      lexicon["comfortable"]=1.8;
        lexicon["friendly"]=1.9;     lexicon["clean"]=1.5;
        lexicon["smooth"]=1.7;       lexicon["worthy"]=1.6;
        lexicon["safe"]=1.5;         lexicon["positive"]=1.9;
        lexicon["beautiful"]=2.1;    lexicon["ok"]=0.9;
        lexicon["okay"]=0.9;         lexicon["alright"]=0.8;
        lexicon["fair"]=0.5;         lexicon["decent"]=1.2;
        lexicon["reasonable"]=1.1;   lexicon["adequate"]=0.8;
        lexicon["satisfactory"]=1.0; lexicon["acceptable"]=0.9;
        lexicon["pleasant"]=1.7;     lexicon["pleasing"]=1.7;
        lexicon["polite"]=1.6;       lexicon["kind"]=1.7;
        lexicon["caring"]=1.8;       lexicon["warm"]=1.6;
        lexicon["gentle"]=1.6;       lexicon["calm"]=1.4;
        lexicon["peaceful"]=1.7;     lexicon["stable"]=1.3;
        lexicon["consistent"]=1.3;   lexicon["trust"]=1.8;
        lexicon["trusted"]=1.8;      lexicon["loyal"]=1.8;
        lexicon["dedicated"]=1.7;    lexicon["confident"]=1.7;
        lexicon["capable"]=1.5;      lexicon["skilled"]=1.6;
        lexicon["talented"]=1.8;     lexicon["clever"]=1.7;
        lexicon["wise"]=1.8;         lexicon["experienced"]=1.5;
        lexicon["professional"]=1.6; lexicon["improve"]=1.5;
        lexicon["improved"]=1.5;     lexicon["progress"]=1.6;
        lexicon["recover"]=1.5;      lexicon["recovered"]=1.5;
        lexicon["heal"]=1.6;         lexicon["cure"]=1.7;
        lexicon["save"]=1.5;         lexicon["protect"]=1.5;
        lexicon["support"]=1.4;      lexicon["help"]=1.5;
        lexicon["helped"]=1.5;       lexicon["cooperate"]=1.5;
        lexicon["unite"]=1.5;        lexicon["share"]=1.3;
        lexicon["donate"]=1.6;       lexicon["volunteer"]=1.7;

        // ── Strong Negative (-3.0 ~ -3.5) ─────────────────────────
        lexicon["terrible"]=-3.5;    lexicon["horrible"]=-3.5;
        lexicon["awful"]=-3.4;       lexicon["disgusting"]=-3.3;
        lexicon["abysmal"]=-3.2;     lexicon["appalling"]=-3.2;
        lexicon["catastrophic"]=-3.1;lexicon["disastrous"]=-3.1;
        lexicon["dreadful"]=-2.9;    lexicon["atrocious"]=-3.2;
        lexicon["despicable"]=-3.0;  lexicon["horrendous"]=-3.3;
        lexicon["hideous"]=-2.9;     lexicon["vile"]=-3.0;
        lexicon["wretched"]=-2.9;    lexicon["deplorable"]=-3.0;
        lexicon["outrageous"]=-2.8;  lexicon["intolerable"]=-2.9;
        lexicon["unbearable"]=-2.8;  lexicon["insufferable"]=-2.8;

        // ── Moderate Negative (-2.0 ~ -2.9) ───────────────────────
        lexicon["bad"]=-2.0;         lexicon["poor"]=-2.0;
        lexicon["hate"]=-2.5;        lexicon["hated"]=-2.5;
        lexicon["worst"]=-2.7;       lexicon["worse"]=-1.9;
        lexicon["sad"]=-2.0;         lexicon["unhappy"]=-2.1;
        lexicon["angry"]=-2.2;       lexicon["anger"]=-2.2;
        lexicon["furious"]=-2.6;     lexicon["rage"]=-2.6;
        lexicon["enraged"]=-2.7;     lexicon["livid"]=-2.5;
        lexicon["disappointed"]=-2.1;lexicon["disappointing"]=-2.1;
        lexicon["disappointment"]=-2.1;
        lexicon["frustrated"]=-2.0;  lexicon["frustrating"]=-2.0;
        lexicon["frustration"]=-2.0; lexicon["annoying"]=-1.8;
        lexicon["annoyed"]=-1.8;     lexicon["irritated"]=-1.9;
        lexicon["aggravating"]=-2.0; lexicon["waste"]=-2.0;
        lexicon["broken"]=-2.0;      lexicon["fail"]=-2.1;
        lexicon["failed"]=-2.1;      lexicon["failure"]=-2.1;
        lexicon["ugly"]=-2.0;        lexicon["rude"]=-2.1;
        lexicon["useless"]=-2.2;     lexicon["worthless"]=-2.3;
        lexicon["painful"]=-2.0;     lexicon["pain"]=-1.9;
        lexicon["scam"]=-3.0;        lexicon["fraud"]=-3.0;
        lexicon["lie"]=-2.5;         lexicon["lied"]=-2.5;
        lexicon["cheat"]=-2.7;       lexicon["cheated"]=-2.7;
        lexicon["steal"]=-2.6;       lexicon["corrupt"]=-2.8;
        lexicon["corruption"]=-2.8;  lexicon["depressed"]=-2.4;
        lexicon["miserable"]=-2.5;   lexicon["hopeless"]=-2.3;
        lexicon["desperate"]=-2.3;   lexicon["devastated"]=-2.6;
        lexicon["heartbroken"]=-2.5; lexicon["crushed"]=-2.4;
        lexicon["destroyed"]=-2.5;   lexicon["ruined"]=-2.4;

        // ── Mild Negative (-0.5 ~ -1.9) ───────────────────────────
        lexicon["boring"]=-1.7;      lexicon["bored"]=-1.6;
        lexicon["dirty"]=-1.8;       lexicon["slow"]=-1.5;
        lexicon["sick"]=-1.8;        lexicon["problem"]=-1.5;
        lexicon["crash"]=-2.0;       lexicon["wrong"]=-1.7;
        lexicon["confusing"]=-1.5;   lexicon["regret"]=-2.1;
        lexicon["mediocre"]=-1.2;    lexicon["unreliable"]=-1.5;
        lexicon["expensive"]=-1.2;   lexicon["fear"]=-2.0;
        lexicon["scared"]=-1.9;      lexicon["worried"]=-1.8;
        lexicon["stress"]=-1.8;      lexicon["stressed"]=-1.8;
        lexicon["shame"]=-2.0;       lexicon["guilt"]=-1.9;
        lexicon["lonely"]=-2.0;      lexicon["alone"]=-1.5;
        lexicon["tired"]=-1.4;       lexicon["exhausted"]=-1.7;
        lexicon["weak"]=-1.5;        lexicon["helpless"]=-2.0;
        lexicon["inferior"]=-1.8;    lexicon["inadequate"]=-1.7;
        lexicon["incompetent"]=-1.9; lexicon["lazy"]=-1.5;
        lexicon["careless"]=-1.6;    lexicon["irresponsible"]=-1.9;
        lexicon["unfair"]=-1.8;      lexicon["unjust"]=-1.9;
        lexicon["selfish"]=-1.8;     lexicon["greedy"]=-1.9;
        lexicon["cruel"]=-2.3;       lexicon["harsh"]=-1.7;
        lexicon["toxic"]=-2.2;       lexicon["harmful"]=-2.0;
        lexicon["dangerous"]=-1.9;   lexicon["risky"]=-1.5;
        lexicon["uncertain"]=-1.2;   lexicon["unstable"]=-1.5;
        lexicon["difficult"]=-1.2;   lexicon["complicated"]=-1.1;
        lexicon["doubt"]=-1.3;       lexicon["suspicious"]=-1.5;
        lexicon["nervous"]=-1.6;     lexicon["anxious"]=-1.8;
        lexicon["panic"]=-2.0;       lexicon["terror"]=-2.5;
        lexicon["horror"]=-2.4;      lexicon["nightmare"]=-2.3;
        lexicon["dread"]=-2.1;       lexicon["paranoid"]=-1.9;
        lexicon["insecure"]=-1.7;    lexicon["grief"]=-2.2;
        lexicon["sorrow"]=-2.1;      lexicon["mourn"]=-2.0;
        lexicon["cry"]=-1.6;         lexicon["weep"]=-1.8;
        lexicon["suffer"]=-2.2;      lexicon["suffering"]=-2.2;
        lexicon["struggle"]=-1.6;    lexicon["burden"]=-1.7;
        lexicon["reject"]=-1.9;      lexicon["rejected"]=-2.0;
        lexicon["ignore"]=-1.5;      lexicon["ignored"]=-1.6;
        lexicon["abandon"]=-2.1;     lexicon["abandoned"]=-2.2;
        lexicon["betray"]=-2.5;      lexicon["betrayed"]=-2.5;
        lexicon["neglect"]=-1.8;
    }

    void buildNegWords()
    {
        negWords = {
            "not","no","never","neither","nor","nothing","nobody",
            "nowhere","without","dont","doesnt","didnt","wasnt","werent",
            "isnt","arent","cant","cannot","wont","wouldnt","shouldnt",
            "couldnt","hardly","barely","scarcely","rarely","seldom",
            "few","little","lack","lacking","absence","absent",
            "deny","denied","refuse","refused"
        };
    }

    void buildIntensifiers()
    {
        intensifiers["very"]=1.3;         intensifiers["extremely"]=1.5;
        intensifiers["incredibly"]=1.5;   intensifiers["absolutely"]=1.4;
        intensifiers["totally"]=1.3;      intensifiers["completely"]=1.3;
        intensifiers["highly"]=1.3;       intensifiers["deeply"]=1.3;
        intensifiers["really"]=1.2;       intensifiers["so"]=1.2;
        intensifiers["quite"]=1.1;        intensifiers["pretty"]=1.1;
        intensifiers["exceptionally"]=1.5;intensifiers["remarkably"]=1.4;
        intensifiers["terribly"]=1.4;     intensifiers["awfully"]=1.4;
        intensifiers["insanely"]=1.4;     intensifiers["unbelievably"]=1.4;
        intensifiers["truly"]=1.2;        intensifiers["genuinely"]=1.2;
        intensifiers["definitely"]=1.2;   intensifiers["especially"]=1.2;
        intensifiers["particularly"]=1.2;
    }

    // diminisher: sentiment কে দুর্বল করে
    // "kind of bad" → bad × 0.6
    void buildDiminishers()
    {
        diminishers["somewhat"]=0.7;  diminishers["slightly"]=0.6;
        diminishers["fairly"]=0.8;    diminishers["rather"]=0.8;
        diminishers["mildly"]=0.6;    diminishers["moderately"]=0.7;
        diminishers["partially"]=0.6; diminishers["almost"]=0.8;
        diminishers["nearly"]=0.8;    diminishers["barely"]=0.4;
        diminishers["just"]=0.7;      diminishers["only"]=0.7;
        diminishers["average"]=0.6;   diminishers["kinda"]=0.6;
    }

    // contrast conjunction: "but" এর পরের sentiment এর weight বাড়ে
    void buildContrastWords()
    {
        contrastWords = {
            "but","however","although","though","despite","yet",
            "nevertheless","nonetheless","still","except",
            "whereas","while","alternatively","conversely"
        };
    }

    void buildEmotionMap()
    {
        // Joy
        emotionMap["happy"]="Joy";        emotionMap["joy"]="Joy";
        emotionMap["excited"]="Joy";      emotionMap["love"]="Joy";
        emotionMap["delight"]="Joy";      emotionMap["cheerful"]="Joy";
        emotionMap["great"]="Joy";        emotionMap["wonderful"]="Joy";
        emotionMap["fun"]="Joy";          emotionMap["funny"]="Joy";
        emotionMap["joyful"]="Joy";       emotionMap["celebrate"]="Joy";
        emotionMap["glad"]="Joy";         emotionMap["pleased"]="Joy";
        emotionMap["blessed"]="Joy";      emotionMap["grateful"]="Joy";
        emotionMap["thankful"]="Joy";     emotionMap["thrilled"]="Joy";
        emotionMap["ecstatic"]="Joy";     emotionMap["elated"]="Joy";
        emotionMap["bliss"]="Joy";

        // Anger
        emotionMap["angry"]="Anger";      emotionMap["hate"]="Anger";
        emotionMap["frustrated"]="Anger"; emotionMap["annoyed"]="Anger";
        emotionMap["rage"]="Anger";       emotionMap["furious"]="Anger";
        emotionMap["rude"]="Anger";       emotionMap["enraged"]="Anger";
        emotionMap["livid"]="Anger";      emotionMap["outraged"]="Anger";
        emotionMap["hostile"]="Anger";    emotionMap["bitter"]="Anger";
        emotionMap["resentful"]="Anger";  emotionMap["scam"]="Anger";
        emotionMap["cheat"]="Anger";      emotionMap["corrupt"]="Anger";

        // Sadness
        emotionMap["sad"]="Sadness";      emotionMap["unhappy"]="Sadness";
        emotionMap["depressed"]="Sadness";emotionMap["lonely"]="Sadness";
        emotionMap["miserable"]="Sadness";emotionMap["hopeless"]="Sadness";
        emotionMap["disappointed"]="Sadness";emotionMap["regret"]="Sadness";
        emotionMap["grief"]="Sadness";    emotionMap["sorrow"]="Sadness";
        emotionMap["mourn"]="Sadness";    emotionMap["cry"]="Sadness";
        emotionMap["weep"]="Sadness";     emotionMap["heartbroken"]="Sadness";
        emotionMap["devastated"]="Sadness";emotionMap["crushed"]="Sadness";
        emotionMap["abandoned"]="Sadness";emotionMap["betrayed"]="Sadness";

        // Fear
        emotionMap["scared"]="Fear";      emotionMap["fear"]="Fear";
        emotionMap["worried"]="Fear";     emotionMap["anxious"]="Fear";
        emotionMap["terrified"]="Fear";   emotionMap["nervous"]="Fear";
        emotionMap["panic"]="Fear";       emotionMap["terror"]="Fear";
        emotionMap["horror"]="Fear";      emotionMap["dread"]="Fear";
        emotionMap["paranoid"]="Fear";    emotionMap["insecure"]="Fear";
        emotionMap["nightmare"]="Fear";   emotionMap["helpless"]="Fear";

        // Surprise
        emotionMap["amazing"]="Surprise"; emotionMap["incredible"]="Surprise";
        emotionMap["unexpected"]="Surprise";emotionMap["shocking"]="Surprise";
        emotionMap["awesome"]="Surprise"; emotionMap["astonishing"]="Surprise";
        emotionMap["astounding"]="Surprise";emotionMap["unbelievable"]="Surprise";
        emotionMap["stunning"]="Surprise";emotionMap["wow"]="Surprise";

        // Disgust
        emotionMap["disgusting"]="Disgust";emotionMap["horrible"]="Disgust";
        emotionMap["awful"]="Disgust";    emotionMap["terrible"]="Disgust";
        emotionMap["nasty"]="Disgust";    emotionMap["ugly"]="Disgust";
        emotionMap["vile"]="Disgust";     emotionMap["revolting"]="Disgust";
        emotionMap["repulsive"]="Disgust";emotionMap["gross"]="Disgust";
        emotionMap["filthy"]="Disgust";   emotionMap["toxic"]="Disgust";

        // Trust (NEW)
        emotionMap["trust"]="Trust";      emotionMap["trusted"]="Trust";
        emotionMap["reliable"]="Trust";   emotionMap["honest"]="Trust";
        emotionMap["loyal"]="Trust";      emotionMap["faithful"]="Trust";
        emotionMap["dependable"]="Trust"; emotionMap["responsible"]="Trust";
        emotionMap["credible"]="Trust";   emotionMap["integrity"]="Trust";

        // Anticipation (NEW)
        emotionMap["hope"]="Anticipation";emotionMap["hopeful"]="Anticipation";
        emotionMap["eager"]="Anticipation";emotionMap["await"]="Anticipation";
        emotionMap["anticipate"]="Anticipation";emotionMap["expect"]="Anticipation";
        emotionMap["plan"]="Anticipation";emotionMap["goal"]="Anticipation";
        emotionMap["forward"]="Anticipation";
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

    bool hasContrastBefore(const vector<string>& tokens, int i)
    {
        for (int j = max(0, i-5); j < i; j++)
            if (contrastWords.count(norm(tokens[j]))) return true;
        return false;
    }

public:
    SentimentAnalyzer()
    {
        buildLexicon();
        buildNegWords();
        buildIntensifiers();
        buildDiminishers();
        buildEmotionMap();
        buildContrastWords();
    }

    SentimentResult analyze(const string& text)
    {
        stringstream ss(text);
        vector<string> tokens;
        string w;
        while (ss >> w) tokens.push_back(w);

        double raw = 0.0, posSum = 0.0, negSum = 0.0;
        int cnt = 0;

        int qmarks   = 0; for (char c : text) if (c=='?') qmarks++;
        int excmarks = 0; for (char c : text) if (c=='!') excmarks++;

        for (int i = 0; i < (int)tokens.size(); i++) {
            string word = norm(tokens[i]);
            if (word.empty() || !lexicon.count(word)) continue;

            double ws = lexicon[word];
            cnt++;

            // Rule 1: ALL CAPS boost
            bool allCaps = true;
            for (char c : tokens[i])
                if (isalpha(c) && !isupper(c)) { allCaps = false; break; }
            if (allCaps && tokens[i].length() > 1)
                ws *= (ws > 0) ? 1.25 : 0.8;

            // Rule 2: Intensifier (1 word before)
            if (i > 0 && intensifiers.count(norm(tokens[i-1])))
                ws *= intensifiers[norm(tokens[i-1])];

            // Rule 3: Diminisher (1-2 words before)
            for (int j = max(0, i-2); j < i; j++)
                if (diminishers.count(norm(tokens[j]))) {
                    ws *= diminishers[norm(tokens[j])];
                    break;
                }

            // Rule 4: Negation window — 5 words
            for (int j = max(0, i-5); j < i; j++)
                if (negWords.count(norm(tokens[j]))) { ws *= -0.74; break; }

            // Rule 5: Contrast conjunction — "but" এর পরে 1.5x boost
            if (hasContrastBefore(tokens, i))
                ws *= 1.5;

            // Rule 6: Exclamation amplify
            if (excmarks > 0)
                ws += (ws > 0 ? 1 : -1) * min(excmarks, 3) * 0.292;

            // Rule 7: Question mark dampening — uncertain statement
            if (qmarks > 0)
                ws *= 0.85;

            raw += ws;
            if (ws > 0) posSum += ws;
            if (ws < 0) negSum += fabs(ws);
        }

        double score = (cnt == 0) ? 0.0 : raw / sqrt(raw * raw + 15.0);
        score = max(-1.0, min(1.0, score));

        double total = posSum + negSum + 1e-9;
        double posR  = posSum / total;
        double negR  = negSum / total;
        double neuR  = max(0.0, 1.0 - posR - negR);
        double pct   = posR + negR + neuR;
        posR /= pct; negR /= pct; neuR /= pct;

        double confidence = min(1.0, (double)cnt / max(1,(int)tokens.size()));

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
        return {score, posR*100, negR*100, neuR*100,
                confidence, label, intensity, emotion};
    }
};