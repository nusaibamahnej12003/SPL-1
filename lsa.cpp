#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <random>
#include <cmath>
#include <iomanip>

using namespace std;

class LDA {
private:
    int K; // number of topics
    int V; // vocabulary size
    int D; // number of documents
    int iterations;
    double alpha, beta;
    
    vector<vector<int>> docs;
    vector<int> z;
    vector<vector<int>> ndk;
    vector<vector<int>> nkw;
    vector<int> nk;
    vector<int> nd;
    
    map<string, int> word2id;
    map<int, string> id2word;
    vector<string> originalDocs;
    
    set<string> stopWords = {"and", "the", "in", "of", "to", "a", "is", "for", "on", "with", "at", "by"};
    
    mt19937 rng;
    
public:
    LDA(int topics, int iter = 1000, double a = 10, double b = 0.01) 
        : K(topics), iterations(iter), alpha(a), beta(b), rng(random_device{}()) {}
    
    void preprocess(const vector<string>& documents) {
        D = documents.size();
        docs.resize(D);
        originalDocs = documents;
        nd.resize(D, 0);
        
        int wordId = 0;
        for (int d = 0; d < D; d++) {
            istringstream iss(documents[d]);
            string word;
            while (iss >> word) {
                transform(word.begin(), word.end(), word.begin(), ::tolower);
                word.erase(remove_if(word.begin(), word.end(), [](char c) {
                    return !isalnum(c);
                }), word.end());
                
                if (word.length() < 3 || stopWords.count(word)) continue;
                
                if (word2id.find(word) == word2id.end()) {
                    word2id[word] = wordId;
                    id2word[wordId] = word;
                    wordId++;
                }
                docs[d].push_back(word2id[word]);
                nd[d]++;
            }
        }
        V = wordId;
    }
    
    void initialize() {
        ndk.assign(D, vector<int>(K, 0));
        nkw.assign(K, vector<int>(V, 0));
        nk.assign(K, 0);
        
        uniform_int_distribution<> dis(0, K - 1);
        
        for (int d = 0; d < D; d++) {
            for (int w : docs[d]) {
                int topic = dis(rng);
                z.push_back(topic);
                ndk[d][topic]++;
                nkw[topic][w]++;
                nk[topic]++;
            }
        }
    }
    
    void gibbsSampling() {
        for (int iter = 0; iter < iterations; iter++) {
            int idx = 0;
            for (int d = 0; d < D; d++) {
                for (int w : docs[d]) {
                    int oldTopic = z[idx];
                    
                    ndk[d][oldTopic]--;
                    nkw[oldTopic][w]--;
                    nk[oldTopic]--;
                    
                    vector<double> p(K);
                    double psum = 0;
                    for (int k = 0; k < K; k++) {
                        p[k] = (nkw[k][w] + beta) / (nk[k] + V * beta) * 
                               (ndk[d][k] + alpha) / (nd[d] - 1 + K * alpha);
                        psum += p[k];
                    }
                    
                    // Sample new topic
                    uniform_real_distribution<> udist(0, psum);
                    double r = udist(rng);
                    double cumsum = 0;
                    int newTopic = 0;
                    for (int k = 0; k < K; k++) {
                        cumsum += p[k];
                        if (r <= cumsum) {
                            newTopic = k;
                            break;
                        }
                    }
                    
                    z[idx] = newTopic;
                    ndk[d][newTopic]++;
                    nkw[newTopic][w]++;
                    nk[newTopic]++;
                    
                    idx++;
                }
            }
            
            // Print progress every 200 iterations
            if ((iter + 1) % 200 == 0) {
                cout << "Iteration " << (iter + 1) << "/" << iterations << " completed\n";
            }
        }
    }
    
    vector<vector<double>> getDocTopicDist() {
        vector<vector<double>> theta(D, vector<double>(K));
        for (int d = 0; d < D; d++) {
            for (int k = 0; k < K; k++) {
                theta[d][k] = (ndk[d][k] + alpha) / (nd[d] + K * alpha);
            }
        }
        return theta;
    }
    
    void displayResults() {
        cout << "\n============================================================\n";
        cout << "LDA TOPIC MODELING RESULTS\n";
        cout << "============================================================\n\n";
        
        auto theta = getDocTopicDist();
        
        // Get dominant topic for each document
        vector<int> docTopics(D);
        for (int d = 0; d < D; d++) {
            int maxTopic = 0;
            double maxProb = theta[d][0];
            for (int k = 1; k < K; k++) {
                if (theta[d][k] > maxProb) {
                    maxProb = theta[d][k];
                    maxTopic = k;
                }
            }
            docTopics[d] = maxTopic;
        }
        
        // Display documents grouped by topic
        for (int k = 0; k < K; k++) {
            cout << "Topic " << k << ":\n";
            
            bool hasDoc = false;
            for (int d = 0; d < D; d++) {
                if (docTopics[d] == k) {
                    cout << "  - Doc " << d << ": " << originalDocs[d] << "\n";
                    hasDoc = true;
                }
            }
            if (!hasDoc) {
                cout << "  (No documents strongly assigned)\n";
            }
            cout << "\n";
        }
        
        // Display top words per topic
        cout << "============================================================\n";
        cout << "TOP WORDS PER TOPIC\n";
        cout << "============================================================\n\n";
        
        for (int k = 0; k < K; k++) {
            vector<pair<double, string>> wordProbs;
            for (int w = 0; w < V; w++) {
                double prob = (nkw[k][w] + beta) / (nk[k] + V * beta);
                wordProbs.push_back({prob, id2word[w]});
            }
            sort(wordProbs.rbegin(), wordProbs.rend());
            
            cout << "Topic " << k << ": ";
            int topN = min(10, (int)wordProbs.size());
            for (int i = 0; i < topN; i++) {
                cout << wordProbs[i].second;
                if (i < topN - 1) cout << ", ";
            }
            cout << "\n\n";
        }
        
        // Display document-topic distribution
        cout << "============================================================\n";
        cout << "DOCUMENT-TOPIC PROBABILITIES\n";
        cout << "============================================================\n\n";
        
        for (int d = 0; d < D; d++) {
            cout << "Doc " << d << ": ";
            for (int k = 0; k < K; k++) {
                cout << "Topic " << k << "=" << fixed << setprecision(3) << theta[d][k];
                if (k < K - 1) cout << ", ";
            }
            cout << "\n";
        }
        cout << "\n";
    }
    
    void fit(const vector<string>& documents) {
        cout << "Preprocessing documents...\n";
        preprocess(documents);
        cout << "Vocabulary size: " << V << " unique words\n";
        cout << "Running LDA with " << K << " topics and " << iterations << " iterations...\n\n";
        initialize();
        gibbsSampling();
        displayResults();
    }
};

int main() {
    vector<string> documents = {
        "football soccer tournament championship match game players team sports competition",
        "doctors nurses medical treatment hospitals healthcare patients clinical medicine therapy",
        "students teachers study universities classrooms education learning academic school courses",
        "software programming development computer technology coding algorithms systems tech",
        "stock market business investment finance trading growth economy profit revenue",
        "basketball baseball athletic sports competition games team players training victory",
        "surgery healthcare physicians hospitals medical care patient treatment diagnosis",
        "college university academic students education professors teaching research learning",
        "artificial intelligence machine learning data science computer technology innovation",
        "financial markets corporate business economics investment banking profit analysis"
    };
    
    LDA lda(5, 1000); // 5 topics, 1000 iterations
    lda.fit(documents);
    
    return 0;
}