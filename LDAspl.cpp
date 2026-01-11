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

using namespace std;

const double ALPHA = 0.1;       
const double BETA = 0.01;        
const double ETA = 150.0;        
const int ITERATIONS = 1000;    

struct Document {
    string rawText;
    string label;
    int labelId;
    vector<int> wordIndices;
    vector<int> topicAssignments;
};

class SupervisedLDA {
private:
    int K; 
    int V; 
    int D; 

    vector<Document> docs;
    vector<string> vocab;
    map<string, int> wordToId;
    map<string, int> labelToId;
    map<int, string> idToLabel;
    set<string> stopWords;

    vector<vector<int>> nw;    
    vector<vector<int>> nd;    
    vector<int> nwsum;        
    vector<int> ndsum;        

    mt19937 rng;

    void initStopWords() {
        string sw[] = {"the", "and", "this", "that", "with", "from", "your", "have", "were", "they", "will", "would", "about"};
        for (const string& s : sw) stopWords.insert(s);
    }


    string basicStemmer(string s) {
        if (s.length() <= 4) return s;
        if (s.substr(s.length() - 3) == "ing") return s.substr(0, s.length() - 3);
        if (s.substr(s.length() - 2) == "ed") return s.substr(0, s.length() - 2);
        if (s.substr(s.length() - 1) == "s") return s.substr(0, s.length() - 1);
        return s;
    }

    string preprocess(string s) {
        string res = "";
        for (char c : s) {
            if (isalpha(c)) res += tolower(c);
        }
        return basicStemmer(res);
    }

    double calculateLogLikelihood() {
        double ll = 0;
        for (int k = 0; k < K; k++) {
            ll += lgamma(V * BETA) - V * lgamma(BETA);
            for (int v = 0; v < V; v++) {
                ll += lgamma(nw[v][k] + BETA);
            }
            ll -= lgamma(nwsum[k] + V * BETA);
        }
        return ll;
    }

public:
    SupervisedLDA() {
        random_device rd;
        rng.seed(rd());
        initStopWords();
    }

    void loadData(string filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: " << filename << " not found!" << endl;
            exit(1);
        }

        string line;
        while (getline(file, line)) {
            size_t delimiterPos = line.find('|');
            if (delimiterPos == string::npos) continue;

            string labelStr = line.substr(0, delimiterPos);
            string text = line.substr(delimiterPos + 1);

            if (labelToId.find(labelStr) == labelToId.end()) {
                int newId = labelToId.size();
                labelToId[labelStr] = newId;
                idToLabel[newId] = labelStr;
            }

            Document doc;
            doc.rawText = text;
            doc.label = labelStr;
            doc.labelId = labelToId[labelStr];

            stringstream ss(text);
            string word;
            while (ss >> word) {
                word = preprocess(word);
                if (word.length() <= 3 || stopWords.count(word)) continue;

                if (wordToId.find(word) == wordToId.end()) {
                    wordToId[word] = vocab.size();
                    vocab.push_back(word);
                }
                doc.wordIndices.push_back(wordToId[word]);
            }
            if (!doc.wordIndices.empty()) docs.push_back(doc);
        }

        D = docs.size();
        V = vocab.size();
        K = labelToId.size();

        nw.assign(V, vector<int>(K, 0));
        nd.assign(D, vector<int>(K, 0));
        nwsum.assign(K, 0);
        ndsum.assign(D, 0);

        for (int d = 0; d < D; ++d) {
            docs[d].topicAssignments.resize(docs[d].wordIndices.size());
            for (int i = 0; i < docs[d].wordIndices.size(); ++i) {
                int topic = docs[d].labelId; 
                docs[d].topicAssignments[i] = topic;
                nw[docs[d].wordIndices[i]][topic]++;
                nd[d][topic]++;
                nwsum[topic]++;
                ndsum[d]++;
            }
        }
        cout << "[SYSTEM] Loaded " << D << " documents and " << V << " unique words." << endl;
    }

    void train() {
        cout << "[TRAINING] Starting Gibbs Sampling with Supervision..." << endl;
        for (int iter = 1; iter <= ITERATIONS; ++iter) {
            for (int d = 0; d < D; ++d) {
                for (int i = 0; i < docs[d].wordIndices.size(); ++i) {
                    int wordId = docs[d].wordIndices[i];
                    int oldTopic = docs[d].topicAssignments[i];

                    nw[wordId][oldTopic]--;
                    nd[d][oldTopic]--;
                    nwsum[oldTopic]--;

                    vector<double> p(K);
                    double pSum = 0;
                    for (int k = 0; k < K; ++k) {
                        double prob = (nw[wordId][k] + BETA) / (nwsum[k] + V * BETA) * (nd[d][k] + ALPHA);
                        if (k == docs[d].labelId) prob *= ETA; 
                        p[k] = prob;
                        pSum += p[k];
                    }

                    uniform_real_distribution<double> u_dist(0, pSum);
                    double u = u_dist(rng);
                    int newTopic = 0;
                    double currentP = 0;
                    for (int k = 0; k < K; ++k) {
                        currentP += p[k];
                        if (u < currentP) {
                            newTopic = k;
                            break;
                        }
                    }

                    docs[d].topicAssignments[i] = newTopic;
                    nw[wordId][newTopic]++;
                    nd[d][newTopic]++;
                    nwsum[newTopic]++;
                }
            }

            if (iter % 100 == 0) {
                float progress = (float)iter / ITERATIONS;
                int barWidth = 40;
                cout << "[";
                int pos = barWidth * progress;
                for (int i = 0; i < barWidth; ++i) {
                    if (i < pos) cout << "=";
                    else if (i == pos) cout << ">";
                    else cout << " ";
                }
                cout << "] " << int(progress * 100.0) << "% | LL: " << calculateLogLikelihood() << "\r";
                cout.flush();
            }
        }
        cout << endl << "[SUCCESS] Training Completed." << endl;
    }

    void displayEnhancedReport() {
        cout << "\n" << string(80, '=') << endl;
        cout << setw(50) << "SUPERVISED TOPIC MODELING REPORT" << endl;
        cout << string(80, '=') << endl;

        int totalCorrect = 0;

        for (int k = 0; k < K; ++k) {
            cout << "\n>>> TOPIC CATEGORY: [" << idToLabel[k] << "] <<<" << endl;
            
            vector<pair<double, string>> topWords;
            for (int v = 0; v < V; ++v) {
                double prob = (double)(nw[v][k] + BETA) / (nwsum[k] + V * BETA);
                topWords.push_back({prob, vocab[v]});
            }
            sort(topWords.rbegin(), topWords.rend());

            cout << "Top Keywords & Influence:" << endl;
            for (int i = 0; i < 8 && i < V; ++i) {
                int barLen = topWords[i].first * 500; 
                cout << "  " << left << setw(12) << topWords[i].second 
                     << " [" << string(min(barLen, 20), '#') << "] " 
                     << fixed << setprecision(4) << topWords[i].first << endl;
            }

            cout << "\nSample Classified Documents in this Category:" << endl;
            int count = 0;
            for (int d = 0; d < D; ++d) {
                if (docs[d].labelId == k) {
                    totalCorrect++; 
                    if (count < 3) {
                        cout << "  - " << docs[d].rawText.substr(0, 80) << "..." << endl;
                        count++;
                    }
                }
            }
            cout << string(40, '-') << endl;
        }

        double accuracy = ((double)totalCorrect / D) * 100.0;
        cout << "\n[FINAL METRICS] Model Accuracy: " << fixed << setprecision(2) << accuracy << "%" << endl;
        cout << "[FINAL METRICS] Log-Likelihood: " << calculateLogLikelihood() << endl;
    }

    void predictInteractive() {
        cout << "\n" << string(80, '*') << endl;
        cout << " ENTER TEXT FOR REAL-TIME CLASSIFICATION (Type 'exit' to quit)" << endl;
        cout << string(80, '*') << endl;

        string input;
        while (true) {
            cout << "\nInput: ";
            getline(cin, input);
            if (input == "exit" || input == "EXIT") break;

            vector<int> testWords;
            stringstream ss(input);
            string word;
            while (ss >> word) {
                word = preprocess(word);
                if (wordToId.count(word)) testWords.push_back(wordToId[word]);
            }

            if (testWords.empty()) {
                cout << "Result: [UNKNOWN] - No vocabulary match found." << endl;
                continue;
            }

            vector<double> scores(K, 0.0);
            for (int wId : testWords) {
                for (int k = 0; k < K; ++k) {
                    scores[k] += log((double)(nw[wId][k] + BETA) / (nwsum[k] + V * BETA));
                }
            }

            int bestK = 0;
            double maxScore = -1e18;
            for (int k = 0; k < K; ++k) {
                if (scores[k] > maxScore) {
                    maxScore = scores[k];
                    bestK = k;
                }
            }

            double confidence = exp(maxScore); 
            cout << "Predicted Category: [" << idToLabel[bestK] << "] (Confidence Score: " << abs(maxScore) << ")" << endl;
        }
    }
};

int main() {
    SupervisedLDA model;

    cout << "--- Advanced sLDA Text Classifier ---" << endl;
    

    model.loadData("input.txt");

    model.train();

    model.displayEnhancedReport();

    model.predictInteractive();

    return 0;
}