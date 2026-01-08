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

using namespace std;


const double ALPHA = 0.1;      
const double BETA = 0.01;      
const int ITERATIONS = 3000;   
const int NUM_TOPICS = 3;      

struct Document {
    string originalText;
    vector<int> words;
};

class TopicModeler {
private:
    int K, V, D;
    vector<Document> docs;
    vector<string> vocab;
    map<string, int> word2id;
    set<string> stopwords;

    vector<vector<int>> nw, nd;    
    vector<int> nwsum, ndsum;         
    vector<vector<int>> z;     

    string clean(string s) {
        string res = "";
        for (char c : s) if (isalpha(c)) res += tolower(c);
        return res;
    }

    void loadStopwords() {
        string stp[] = {"the", "and", "was", "for", "with", "from", "their", "many", "has", "are", "have", 
                        "they", "this", "that", "use", "used", "using", "get", "well", "must", "can", "help", 
                        "also", "very", "often", "good", "great", "some", "made", "would", "could", "should",
                        "like", "these", "those", "into", "been", "which", "than", "more", "most", "about"};
        for(string s : stp) stopwords.insert(s);
    }

public:
    TopicModeler(int num_topics) : K(num_topics) {}

    void loadData(string filename) {
        loadStopwords();
        ifstream file(filename);
        if (!file.is_open()) { cout << "Error: input.txt not found!" << endl; exit(1); }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            Document doc;
            doc.originalText = line;
            stringstream ss(line);
            string word;
            while (ss >> word) {
                word = clean(word);
                if (word.length() <= 3 || stopwords.count(word)) continue;
                if (word2id.find(word) == word2id.end()) {
                    word2id[word] = vocab.size();
                    vocab.push_back(word);
                }
                doc.words.push_back(word2id[word]);
            }
            if (!doc.words.empty()) docs.push_back(doc);
        }

        V = vocab.size(); D = docs.size();
        nw.assign(V, vector<int>(K, 0));
        nd.assign(D, vector<int>(K, 0));
        nwsum.assign(K, 0); ndsum.assign(D, 0);
        z.resize(D);

        random_device rd; mt19937 gen(rd());
        uniform_int_distribution<> dis(0, K - 1);

        for (int d = 0; d < D; d++) {
            z[d].resize(docs[d].words.size());
            for (int i = 0; i < docs[d].words.size(); i++) {
                int topic = dis(gen);
                int word_id = docs[d].words[i];
                z[d][i] = topic;
                nw[word_id][topic]++; nd[d][topic]++;
                nwsum[topic]++; ndsum[d]++;
            }
        }
    }

    void train() {
        random_device rd; mt19937 gen(rd());
        cout << "Training LDA Model on Large Dataset..." << endl;

        for (int iter = 0; iter < ITERATIONS; iter++) {
            for (int d = 0; d < D; d++) {
                for (int i = 0; i < docs[d].words.size(); i++) {
                    int word_id = docs[d].words[i];
                    int topic = z[d][i];

                    nw[word_id][topic]--; nd[d][topic]--;
                    nwsum[topic]--; ndsum[d]--;

                    vector<double> p(K);
                    for (int k = 0; k < K; k++) {
                        p[k] = (nw[word_id][k] + BETA) / (nwsum[k] + V * BETA) *
                               (nd[d][k] + ALPHA) / (ndsum[d] + K * ALPHA);
                    }

                    discrete_distribution<> dist(p.begin(), p.end());
                    int new_topic = dist(gen);

                    z[d][i] = new_topic;
                    nw[word_id][new_topic]++; nd[d][new_topic]++;
                    nwsum[new_topic]++; ndsum[d]++;
                }
            }
            if(iter % 500 == 0) cout << "Iteration " << iter << " done..." << endl;
        }
    }

    void clusterAndDisplay() {
      
        cout << "   BIG DATA TOPIC CLUSTERING RESULT" << endl;
        

        map<int, vector<string>> clusters;
        for (int d = 0; d < D; d++) {
            int bestTopic = 0;
            int maxCount = -1;
            for (int k = 0; k < K; k++) {
                if (nd[d][k] > maxCount) {
                    maxCount = nd[d][k];
                    bestTopic = k;
                }
            }
            clusters[bestTopic].push_back(docs[d].originalText);
        }

        for (int k = 0; k < K; k++) {
            vector<pair<int, int>> topWords;
            for (int v = 0; v < V; v++) topWords.push_back({nw[v][k], v});
            sort(topWords.rbegin(), topWords.rend());

            cout << ">>> TOPIC GROUP #" << k + 1 << endl;
            cout << "Core Words: ";
            for (int i = 0; i < 6 && i < V; i++) cout << vocab[topWords[i].second] << " ";
            cout << "\n-------------------------------------------" << endl;
            
            for (const string& line : clusters[k]) {
                cout << "  " << line << endl;
            }
            cout << endl;
        }
    }
};

int main() {
    TopicModeler lda(3); 
    lda.loadData("input.txt");
    lda.train();
    lda.clusterAndDisplay();
    return 0;
}