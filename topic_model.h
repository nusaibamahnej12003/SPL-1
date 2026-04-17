#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <random>
#include <iomanip>
#include <numeric>
#include <chrono>
using namespace std;

// ═══════════════════════════════════════════════════════════════════
//  SECTION 1: PORTER STEMMER
// ═══════════════════════════════════════════════════════════════════
class PorterStemmer
{
    string b;
    int k, j;

    bool cons(int i)
    {
        switch (b[i]) {
            case 'a': case 'e': case 'i': case 'o': case 'u': return false;
            case 'y': return (i == 0) ? true : !cons(i - 1);
            default:  return true;
        }
    }

    int m()
    {
        int n = 0, i = 0;
        while (true) {
            if (i > j) return n;
            if (!cons(i)) break;
            i++;
        }
        i++;
        while (true) {
            while (true) { if (i > j) return n; if (cons(i)) break; i++; }
            n++; i++;
            while (true) { if (i > j) return n; if (!cons(i)) break; i++; }
            i++;
        }
    }

    bool vowelinstem()
    {
        for (int i = 0; i <= j; i++) if (!cons(i)) return true;
        return false;
    }

    bool doublec(int i)
    {
        if (i < 1) return false;
        if (b[i] != b[i - 1]) return false;
        return cons(i);
    }

    bool cvc(int i)
    {
        if (i < 2 || !cons(i) || cons(i-1) || !cons(i-2)) return false;
        char ch = b[i];
        if (ch == 'w' || ch == 'x' || ch == 'y') return false;
        return true;
    }

    bool ends(const string& s)
    {
        int l = s.length();
        if (l > k + 1) return false;
        if (b.substr(k - l + 1, l) != s) return false;
        j = k - l;
        return true;
    }

    void setto(const string& s) { b.replace(j+1, k-j, s); k = j + s.length(); }
    void r(const string& s)     { if (m() > 0) setto(s); }

    void step1ab()
    {
        if (b[k] == 's') {
            if      (ends("sses")) k -= 2;
            else if (ends("ies"))  setto("i");
            else if (b[k-1] != 's') k--;
        }
        if (ends("eed")) { if (m() > 0) k--; }
        else if ((ends("ed") || ends("ing")) && vowelinstem()) {
            k = j;
            if      (ends("at")) setto("ate");
            else if (ends("bl")) setto("ble");
            else if (ends("iz")) setto("ize");
            else if (doublec(k)) {
                k--;
                char ch = b[k];
                if (ch == 'l' || ch == 's' || ch == 'z') k++;
            }
            else if (m() == 1 && cvc(k)) setto("e");
        }
    }

    void step1c() { if (ends("y") && vowelinstem()) b[k] = 'i'; }

    void step2()
    {
        switch (b[k-1]) {
            case 'a': if (ends("ational")) r("ate"); else if (ends("tional")) r("tion"); break;
            case 'c': if (ends("enci")) r("ence"); else if (ends("anci")) r("ance"); break;
            case 'e': if (ends("izer")) r("ize"); break;
            case 'l': if (ends("bli")) r("ble");
                      else if (ends("alli"))  r("al");
                      else if (ends("entli")) r("ent");
                      else if (ends("eli"))   r("e");
                      else if (ends("ousli")) r("ous"); break;
            case 'o': if (ends("ization")) r("ize");
                      else if (ends("ation")) r("ate");
                      else if (ends("ator"))  r("ate"); break;
            case 's': if (ends("alism"))   r("al");
                      else if (ends("iveness")) r("ive");
                      else if (ends("fulness")) r("ful");
                      else if (ends("ousness")) r("ous"); break;
            case 't': if (ends("aliti"))  r("al");
                      else if (ends("iviti"))  r("ive");
                      else if (ends("biliti")) r("ble"); break;
        }
    }

    void step3()
    {
        switch (b[k]) {
            case 'e': if (ends("icate")) r("ic");
                      else if (ends("ative")) r("");
                      else if (ends("alize")) r("al"); break;
            case 'i': if (ends("iciti")) r("ic"); break;
            case 'l': if (ends("ical")) r("ic"); else if (ends("ful")) r(""); break;
            case 's': if (ends("ness")) r(""); break;
        }
    }

    void step4()
    {
        switch (b[k-1]) {
            case 'a': if (ends("al"))    break; else return;
            case 'c': if (ends("ance") || ends("ence")) break; else return;
            case 'e': if (ends("er"))    break; else return;
            case 'i': if (ends("ic"))    break; else return;
            case 'l': if (ends("able") || ends("ible")) break; else return;
            case 'n': if (ends("ant") || ends("ement") || ends("ment") || ends("ent")) break; else return;
            case 'o': if (ends("ion") && j >= 0 && (b[j]=='s'||b[j]=='t')) break;
                      else if (ends("ou")) break; else return;
            case 's': if (ends("ism"))   break; else return;
            case 't': if (ends("ate") || ends("iti")) break; else return;
            case 'u': if (ends("ous"))   break; else return;
            case 'v': if (ends("ive"))   break; else return;
            case 'z': if (ends("ize"))   break; else return;
            default: return;
        }
        if (m() > 1) k = j;
    }

    void step5a() { if (b[k]=='e') { int a=m(); if (a>1||(a==1&&!cvc(k-1))) k--; } }
    void step5b() { if (b[k]=='l' && doublec(k) && m()>1) k--; }

public:
    string stem(const string& word)
    {
        if (word.length() <= 2) return word;
        b = word; k = word.length()-1; j = 0;
        step1ab(); step1c(); step2(); step3(); step4(); step5a(); step5b();
        return b.substr(0, k+1);
    }
};

// ═══════════════════════════════════════════════════════════════════
//  SECTION 2: TEXT PREPROCESSOR
// ═══════════════════════════════════════════════════════════════════
class TextPreprocessor
{
private:
    PorterStemmer stemmer;
    set<string>   stopWords;

public:
    TextPreprocessor()
    {
        string sw[] = {
            "the","and","this","that","with","from","your","have","were",
            "they","will","would","about","been","also","more","when","which",
            "their","there","what","into","than","then","its","but","not",
            "are","was","for","has","his","her","she","him","you","can",
            "all","one","had","our","they","just","very","much","some",
            "such","only","even","most","after","before","those","these",
            "is","in","to","a","an","of","it","he","i","at","on","by"
        };
        for (const string& s : sw) stopWords.insert(s);
    }

    string cleanWord(const string& raw)
    {
        string res = "";
        for (char c : raw) if (isalpha(c)) res += tolower(c);
        return res;
    }

    string stemWord(const string& word) { return stemmer.stem(word); }
    bool isStopWord(const string& word) { return stopWords.count(word) > 0; }

    vector<string> tokenize(const string& text)
    {
        vector<string> tokens;
        stringstream ss(text);
        string word;
        while (ss >> word) {
            string cleaned = cleanWord(word);
            if (cleaned.length() >= 2 && !isStopWord(cleaned))
                tokens.push_back(stemWord(cleaned));
        }
        return tokens;
    }

    vector<string> tokenizeRaw(const string& text)
    {
        vector<string> tokens;
        stringstream ss(text);
        string word;
        while (ss >> word) {
            string cleaned = cleanWord(word);
            if (!cleaned.empty()) tokens.push_back(cleaned);
        }
        return tokens;
    }
};

// ═══════════════════════════════════════════════════════════════════
//  SECTION 3: TF-IDF VECTORIZER
// ═══════════════════════════════════════════════════════════════════
class TFIDFVectorizer
{
public:
    static double tf(int termCount, int totalTerms)
    {
        if (totalTerms == 0) return 0.0;
        return (double)termCount / totalTerms;
    }

    static double idf(int numTopics, int topicsWithTerm)
    {
        if (topicsWithTerm == 0) return 0.0;
        return log((double)numTopics / (topicsWithTerm + 1.0)) + 1.0;
    }

    static double tfidf(int termCount, int totalTerms, int numTopics, int topicsWithTerm)
    {
        return tf(termCount, totalTerms) * idf(numTopics, topicsWithTerm);
    }
};

// ═══════════════════════════════════════════════════════════════════
//  SECTION 4: SUPERVISED LDA — TOPIC MODELING
// ═══════════════════════════════════════════════════════════════════

const double LDA_ALPHA = 0.1;
const double LDA_BETA  = 0.01;
const double LDA_ETA   = 150.0;
const int    LDA_ITER  = 1000;
const int    BURN_IN   = 200;
const int    THINNING  = 5;

struct Document
{
    string      label;
    int         labelId;
    vector<int> wordIndices;
    vector<int> topicAssignments;
};

class SupervisedLDA
{
private:
    int K, V, D;
    vector<Document>       docs;
    vector<string>         vocab;
    map<string, int>       wordToId;
    map<string, int>       labelToId;
    map<int, string>       idToLabel;
    vector<vector<int>>    nw;
    vector<vector<int>>    nd;
    vector<int>            nwsum;
    vector<int>            ndsum;
    vector<vector<double>> nw_acc;
    int                    acc_count = 0;
    mt19937                rng;
    TextPreprocessor       preprocessor;

    void initCounts()
    {
        D = docs.size(); V = vocab.size(); K = labelToId.size();
        nw.assign(V, vector<int>(K, 0));
        nd.assign(D, vector<int>(K, 0));
        nwsum.assign(K, 0);
        ndsum.assign(D, 0);
        nw_acc.assign(V, vector<double>(K, 0.0));
        for (int d = 0; d < D; ++d) {
            docs[d].topicAssignments.resize(docs[d].wordIndices.size());
            for (int i = 0; i < (int)docs[d].wordIndices.size(); ++i) {
                int t = docs[d].labelId;
                docs[d].topicAssignments[i] = t;
                nw[docs[d].wordIndices[i]][t]++;
                nd[d][t]++; nwsum[t]++; ndsum[d]++;
            }
        }
    }

    double logLikelihood()
    {
        double ll = 0;
        for (int k = 0; k < K; k++) {
            ll += lgamma(V * LDA_BETA) - V * lgamma(LDA_BETA);
            for (int v = 0; v < V; v++) ll += lgamma(nw[v][k] + LDA_BETA);
            ll -= lgamma(nwsum[k] + V * LDA_BETA);
        }
        return ll;
    }

public:
    SupervisedLDA() { random_device rd; rng.seed(rd()); }

    void loadData(const string& filename)
    {
        ifstream file(filename);
        if (!file.is_open()) { cerr << "[ERROR] input.txt not found!" << endl; exit(1); }

        string line;
        while (getline(file, line)) {
            size_t pos = line.find('|');
            if (pos == string::npos) continue;
            string labelStr = line.substr(0, pos);
            string text     = line.substr(pos + 1);

            if (!labelToId.count(labelStr)) {
                int id = labelToId.size();
                labelToId[labelStr] = id;
                idToLabel[id] = labelStr;
            }

            Document doc;
            doc.label   = labelStr;
            doc.labelId = labelToId[labelStr];

            vector<string> tokens = preprocessor.tokenize(text);
            for (const string& tok : tokens) {
                if (!wordToId.count(tok)) {
                    wordToId[tok] = vocab.size();
                    vocab.push_back(tok);
                }
                doc.wordIndices.push_back(wordToId[tok]);
            }
            if (!doc.wordIndices.empty()) docs.push_back(doc);
        }
        initCounts();
        cout << "[Topic Model] Loaded " << D << " docs | "
             << K << " topics | " << V << " vocab words" << endl;
    }

    void train()
    {
        auto t0 = chrono::steady_clock::now();
        cout << "[Topic Model] Gibbs Sampling — Burn-in: " << BURN_IN
             << " | Thinning: " << THINNING
             << " | Iterations: " << LDA_ITER << endl;

        for (int iter = 1; iter <= LDA_ITER; ++iter) {
            for (int d = 0; d < D; ++d) {
                for (int i = 0; i < (int)docs[d].wordIndices.size(); ++i) {
                    int wId = docs[d].wordIndices[i];
                    int old = docs[d].topicAssignments[i];
                    nw[wId][old]--; nd[d][old]--; nwsum[old]--;

                    vector<double> p(K); double pSum = 0;
                    for (int k = 0; k < K; ++k) {
                        double prob = (nw[wId][k] + LDA_BETA) / (nwsum[k] + V * LDA_BETA)
                                    * (nd[d][k] + LDA_ALPHA);
                        if (k == docs[d].labelId) prob *= LDA_ETA;
                        p[k] = prob; pSum += prob;
                    }

                    uniform_real_distribution<double> u(0, pSum);
                    double r = u(rng); double cur = 0; int nt = K-1;
                    for (int k = 0; k < K; ++k) { cur += p[k]; if (r < cur) { nt=k; break; } }

                    docs[d].topicAssignments[i] = nt;
                    nw[wId][nt]++; nd[d][nt]++; nwsum[nt]++;
                }
            }

            if (iter > BURN_IN && iter % THINNING == 0) {
                for (int v = 0; v < V; v++)
                    for (int k = 0; k < K; k++)
                        nw_acc[v][k] += nw[v][k];
                acc_count++;
            }

            if (iter % 200 == 0) {
                auto elapsed = chrono::duration_cast<chrono::seconds>(
                    chrono::steady_clock::now() - t0).count();
                cout << "  Iter " << setw(4) << iter
                     << " | LL: " << fixed << setprecision(1) << logLikelihood()
                     << " | " << elapsed << "s" << endl;
            }
        }

        if (acc_count > 0)
            for (int v = 0; v < V; v++)
                for (int k = 0; k < K; k++)
                    nw_acc[v][k] /= acc_count;

        cout << "[Topic Model] Training complete! Samples: " << acc_count << endl;
    }

    string predict(const string& input)
    {
        vector<string> tokens = preprocessor.tokenize(input);
        vector<int> testWords;
        for (const string& tok : tokens)
            if (wordToId.count(tok)) testWords.push_back(wordToId[tok]);

        if (testWords.empty()) return "UNKNOWN";

        int bestK = 0; double maxScore = -1e18;
        for (int k = 0; k < K; ++k) {
            double score = 0;
            for (int wId : testWords)
                score += log((nw[wId][k] + LDA_BETA) / (nwsum[k] + V * LDA_BETA));
            if (score > maxScore) { maxScore = score; bestK = k; }
        }
        return idToLabel[bestK];
    }
};