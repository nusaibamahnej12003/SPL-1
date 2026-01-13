
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

struct Document 
{
    string rawText;
    string label;
    int labelId;
    vector<int> wordIndices;
    vector<int> topicAssignments;
};

class SupervisedLDA
 {
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

    void initStopWords() 
    {
        string sw[] = {"the", "and", "this", "that", "with", "from", "your", "have", "were", "they", "will", "would", "about"};
        for (const string& s : sw) stopWords.insert(s);
    }

    string basicStemmer(string s) 
    {
        if (s.length() <= 4) return s;
        if (s.substr(s.length() - 3) == "ing") return s.substr(0, s.length() - 3);
        if (s.substr(s.length() - 2) == "ed") return s.substr(0, s.length() - 2);
        if (s.substr(s.length() - 1) == "s") return s.substr(0, s.length() - 1);
        return s;
    }

      string preprocess(string s) 
    {
        string res = "";
        for (char c : s) {
            if (isalpha(c)) res += tolower(c);
        }
        return basicStemmer(res);
    }

    double calculateLogLikelihood()
     {
        double ll = 0;
        for (int k = 0; k < K; k++)
         {
            ll += lgamma(V * BETA) - V * lgamma(BETA);
            for (int v = 0; v < V; v++) {
                ll += lgamma(nw[v][k] + BETA);
            }
            ll -= lgamma(nwsum[k] + V * BETA);
        }
        return ll;
    }

    void initializeAfterLoad()
     {
        D = docs.size();
        V = vocab.size();
        K = labelToId.size();

        nw.assign(V, vector<int>(K, 0));
        nd.assign(D, vector<int>(K, 0));
        nwsum.assign(K, 0);
        ndsum.assign(D, 0);

        for (int d = 0; d < D; ++d)
         {
            docs[d].topicAssignments.resize(docs[d].wordIndices.size());
            for (int i = 0; i < docs[d].wordIndices.size(); ++i)
             {
                int topic = docs[d].labelId; 
                docs[d].topicAssignments[i] = topic;
                nw[docs[d].wordIndices[i]][topic]++;
                nd[d][topic]++;
                nwsum[topic]++;
                ndsum[d]++;
            }
        }
    }

public:

    SupervisedLDA()
     {
        random_device rd;
        rng.seed(rd());
        initStopWords();
     }

    void loadData(string filename) 
    {
        ifstream file(filename);
        if (!file.is_open())
         {
            cerr << "Error: " << filename << " not found!" << endl;
            exit(1);
        }

        string line;
        while (getline(file, line)) 
        {
            size_t delimiterPos = line.find('|');
            if (delimiterPos == string::npos) continue;

            string labelStr = line.substr(0, delimiterPos);
            string text = line.substr(delimiterPos + 1);

            if (labelToId.find(labelStr) == labelToId.end())
             {
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
                if (word.length() < 2 || stopWords.count(word)) continue;

                if (wordToId.find(word) == wordToId.end())
                 {
                    wordToId[word] = vocab.size();
                    vocab.push_back(word);
                 }
                doc.wordIndices.push_back(wordToId[word]);
            }
            if (!doc.wordIndices.empty()) docs.push_back(doc);
        }
        initializeAfterLoad();

        cout << "[SYSTEM] Loaded " << D << " documents from Text file." << endl;
    }

    void saveToBinary(string filename) 
    {
        ofstream out(filename, ios::binary);
        if (!out) return;

        int vocabSize = vocab.size();
        out.write((char*)&vocabSize, sizeof(int));
        for (const string& s : vocab)
         {
            size_t len = s.length();
            out.write((char*)&len, sizeof(size_t));
            out.write(s.c_str(), len);
         }

        int labelSize = labelToId.size();
        out.write((char*)&labelSize, sizeof(int));
        for (int i = 0; i < labelSize; i++) 
        {
            string s = idToLabel[i];
            size_t len = s.length();
            out.write((char*)&len, sizeof(size_t));
            out.write(s.c_str(), len);
        }

        int docCount = docs.size();
        out.write((char*)&docCount, sizeof(int));
        for (const auto& doc : docs) 
        {
            out.write((char*)&doc.labelId, sizeof(int));
            int wCount = doc.wordIndices.size();
            out.write((char*)&wCount, sizeof(int));
            out.write((char*)doc.wordIndices.data(), wCount * sizeof(int));
        }
        out.close();
        cout << "[OPTIMIZATION] Preprocessed data saved to Binary: " << filename << endl;
    }

    void loadFromBinary(string filename)
     {
        ifstream in(filename, ios::binary);
        if (!in) 
        {
            cout << "[ERROR] Binary file not found. Falling back to Text loading..." << endl;
            return;
        }

        docs.clear(); vocab.clear(); wordToId.clear(); labelToId.clear(); idToLabel.clear();

      
        int vocabSize; in.read((char*)&vocabSize, sizeof(int));

        for (int i = 0; i < vocabSize; i++) 
        {
            size_t len; in.read((char*)&len, sizeof(size_t));
            char* buf = new char[len + 1];
            in.read(buf, len); buf[len] = '\0';
            string s(buf); vocab.push_back(s); wordToId[s] = i;
            delete[] buf;
        }

 
        int labelSize; in.read((char*)&labelSize, sizeof(int));

        for (int i = 0; i < labelSize; i++)
         {
            size_t len; in.read((char*)&len, sizeof(size_t));
            char* buf = new char[len + 1];
            in.read(buf, len); buf[len] = '\0';
            string s(buf); idToLabel[i] = s; labelToId[s] = i;
            delete[] buf;
        }

    
        int docCount; in.read((char*)&docCount, sizeof(int));

        for (int i = 0; i < docCount; i++) 
        {
            Document doc;
            in.read((char*)&doc.labelId, sizeof(int));
            int wCount; in.read((char*)&wCount, sizeof(int));
            doc.wordIndices.resize(wCount);
            in.read((char*)doc.wordIndices.data(), wCount * sizeof(int));
            docs.push_back(doc);
        }
        in.close();
        initializeAfterLoad();

        cout << "[SYSTEM] Loaded " << D << " documents from Binary file (Optimized)." << endl;
    }

    void train()
     {
        cout << "[TRAINING] Starting Gibbs Sampling with Supervision..." << endl;
        for (int iter = 1; iter <= ITERATIONS; ++iter) {
            for (int d = 0; d < D; ++d) 
            {
                for (int i = 0; i < docs[d].wordIndices.size(); ++i) 
                {
                    int wordId = docs[d].wordIndices[i];
                    int oldTopic = docs[d].topicAssignments[i];

                    nw[wordId][oldTopic]--;
                    nd[d][oldTopic]--;
                    nwsum[oldTopic]--;

                    vector<double> p(K);
                    double pSum = 0;
                    for (int k = 0; k < K; ++k)
                     {
                        double prob = (nw[wordId][k] + BETA) / (nwsum[k] + V * BETA) * (nd[d][k] + ALPHA);
                        if (k == docs[d].labelId) prob *= ETA; 
                        p[k] = prob;
                        pSum += p[k];
                    }

                    uniform_real_distribution<double> u_dist(0, pSum);
                    double u = u_dist(rng);
                    int newTopic = 0;
                    double currentP = 0;
                    for (int k = 0; k < K; ++k) 
                    {
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

            if (iter % 100 == 0) 
            {
                cout << "Iteration " << iter << " | LL: " << calculateLogLikelihood() << endl;
            }
        }
        cout << "[SUCCESS] Training Completed." << endl;
    }

  double calculateTopicCoherence(int topicId, int topN = 10) 
  {
    vector<pair<int, int>> topWords;
    for (int v = 0; v < V; v++) topWords.push_back({nw[v][topicId], v});
    sort(topWords.rbegin(), topWords.rend());

    double coherence = 0.0;
    for (int i = 0; i < topN && i < (int)topWords.size(); i++)
     {
        for (int j = 0; j < i; j++) {
            int w1 = topWords[i].second;
            int w2 = topWords[j].second;
            int coOccurrence = 0;
            for (int d = 0; d < D; d++) 
            {
                bool hasW1 = false, hasW2 = false;
                for (int wid : docs[d].wordIndices)
                 {
                    if (wid == w1) hasW1 = true;
                    if (wid == w2) hasW2 = true;
                 }
                if (hasW1 && hasW2) coOccurrence++;
            }
            coherence += log((coOccurrence + 1.0) / nwsum[topicId]);
        }
    }
    return coherence;
}

void exportResultsToCSV(string filename)
 {
    ofstream file(filename);
    file << "Topic_ID,Category_Name,Top_Keywords,Coherence_Score\n";
    for (int k = 0; k < K; k++)
     {
        file << k << "," << idToLabel[k] << ",";
        
        vector<pair<int, string>> words;
        for (int v = 0; v < V; v++) words.push_back({nw[v][k], vocab[v]});
        sort(words.rbegin(), words.rend());
        
        string keywords = "";
        for (int i = 0; i < 5 && i < (int)words.size(); i++) keywords += words[i].second + " ";
        
        file << keywords << "," << calculateTopicCoherence(k) << "\n";
    }

    file.close();
    cout << "\n[SUCCESS] Results exported to " << filename << " (Open with Excel)" << endl;
}

void logMessage(string type, string msg) 
{
    if (type == "INFO") cout << "\033[34m[INFO] " << msg << "\033[0m" << endl;      

    else if (type == "SUCCESS") cout << "\033[32m[SUCCESS] " << msg << "\033[0m" << endl; 
    
    else if (type == "ERROR") cout << "\033[31m[ERROR] " << msg << "\033[0m" << endl;  
}



    void displayEnhancedReport() 
    {
        cout << "\n" << string(80, '=') << endl;
        cout << setw(50) << "SUPERVISED TOPIC MODELING REPORT" << endl;
        cout << string(80, '=') << endl;

        for (int k = 0; k < K; ++k) 
        {
            cout << "\n>>> TOPIC CATEGORY: [" << idToLabel[k] << "] <<<" << endl;
            vector<pair<double, string>> topWords;
            for (int v = 0; v < V; ++v) 
            {
                double prob = (double)(nw[v][k] + BETA) / (nwsum[k] + V * BETA);
                topWords.push_back({prob, vocab[v]});
            }
            sort(topWords.rbegin(), topWords.rend());

            cout << "Top Keywords:" << endl;
            for (int i = 0; i < 8 && i < V; ++i) 
            {
                cout << "  - " << left << setw(12) << topWords[i].second << " Prob: " << fixed << setprecision(4) << topWords[i].first << endl;
            }
        }
    }

    void predictInteractive() 
    {
        cout << "\nENTER TEXT FOR REAL-TIME CLASSIFICATION (Type 'exit' to quit)" << endl;
        string input;
        while (true) {
            cout << "\nInput: ";
            getline(cin, input);
            if (input == "exit" || input == "EXIT") break;

            vector<int> testWords;
            stringstream ss(input); string word;
            while (ss >> word) 
            {
                word = preprocess(word);
                if (wordToId.count(word)) testWords.push_back(wordToId[word]);
            }

            if (testWords.empty()) { cout << "Result: [UNKNOWN]" << endl; continue; }

            int bestK = 0; double maxScore = -1e18;
            for (int k = 0; k < K; ++k) 
            {
                double score = 0;
                for (int wId : testWords) score += log((double)(nw[wId][k] + BETA) / (nwsum[k] + V * BETA));
                if (score > maxScore) { maxScore = score; bestK = k; }
            }
            cout << "Predicted Category: [" << idToLabel[bestK] << "] (Score: " << abs(maxScore) << ")" << endl;
        }
    }
};

int main()
 {
    SupervisedLDA model;

    cout << "--- Advanced sLDA Text Classifier ---" << endl;
    
    model.loadData("input.txt");

    model.saveToBinary("dataset.bin"); 
    
    model.train();

    model.displayEnhancedReport();

    model.predictInteractive();

    return 0;
}