#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cmath>
#include <set>
#include <iomanip>
#include <cctype>

using namespace std;

// ========== TEXT PREPROCESSOR ==========
class TextPreprocessor {
private:
    set<string> stopwords;
    
public:
    TextPreprocessor() {
        // Common stopwords
        stopwords = {"the", "a", "an", "and", "or", "but", "in", "on", "at", "to", 
                    "for", "of", "with", "by", "is", "are", "was", "were", "be", 
                    "been", "have", "has", "had", "do", "does", "did", "will", 
                    "would", "should", "can", "could", "may", "might", "must"};
    }
    
    string cleanWord(const string& word) {
        string result;
        for (char c : word) {
            if (isalpha(c)) {
                result += tolower(c);
            }
        }
        return result;
    }
    
    vector<string> process(const string& text) {
        vector<string> tokens;
        string word;
        
        for (char c : text) {
            if (isspace(c) || ispunct(c)) {
                if (!word.empty()) {
                    word = cleanWord(word);
                    if (!word.empty() && word.length() > 2 && 
                        stopwords.find(word) == stopwords.end()) {
                        tokens.push_back(word);
                    }
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        
        if (!word.empty()) {
            word = cleanWord(word);
            if (!word.empty() && word.length() > 2 && 
                stopwords.find(word) == stopwords.end()) {
                tokens.push_back(word);
            }
        }
        
        return tokens;
    }
};

// ========== DOCUMENT CLASS ==========
class Document {
public:
    int id;
    string originalText;
    vector<string> tokens;
    string trueLabel;      // For evaluation
    string predictedLabel;
    double confidence;
    
    Document(int docId, const string& text, const string& label = "") 
        : id(docId), originalText(text), trueLabel(label), confidence(0.0) {}
};

// ========== LDA MODEL ==========
class LDAModel {
private:
    int K;                 // Number of topics
    int V;                 // Vocabulary size
    double alpha;
    double beta;
    
    vector<vector<int>> docs;          // Documents as word IDs
    vector<vector<int>> assignments;   // Topic assignments
    vector<vector<int>> ndk;           // Document-topic counts
    vector<vector<int>> nkw;           // Topic-word counts
    vector<int> nk;                    // Topic counts
    
    mt19937 rng;
    map<string, int> word2id;
    map<int, string> id2word;
    
public:
    LDAModel(int numTopics = 5, double a = 0.1, double b = 0.01) 
        : K(numTopics), alpha(a), beta(b), rng(42) {}
    
    void buildVocabulary(const vector<Document>& documents) {
        map<string, int> wordFreq;
        
        for (const auto& doc : documents) {
            for (const auto& token : doc.tokens) {
                wordFreq[token]++;
            }
        }
        
        // Filter rare words
        int id = 0;
        for (const auto& pair : wordFreq) {
            if (pair.second >= 2) {  // Keep words appearing at least twice
                word2id[pair.first] = id;
                id2word[id] = pair.first;
                id++;
            }
        }
        
        V = word2id.size();
        cout << "Vocabulary size: " << V << " words" << endl;
    }
    
    void prepareDocuments(const vector<Document>& documents) {
        docs.clear();
        
        for (const auto& doc : documents) {
            vector<int> docWords;
            for (const auto& token : doc.tokens) {
                auto it = word2id.find(token);
                if (it != word2id.end()) {
                    docWords.push_back(it->second);
                }
            }
            if (!docWords.empty()) {
                docs.push_back(docWords);
            }
        }
        
        cout << "Prepared " << docs.size() << " documents for training" << endl;
    }
    
    void initialize() {
        int D = docs.size();
        
        // Initialize count matrices
        ndk.assign(D, vector<int>(K, 0));
        nkw.assign(K, vector<int>(V, 0));
        nk.assign(K, 0);
        assignments.resize(D);
        
        // Random initialization with better distribution
        uniform_int_distribution<int> topicDist(0, K-1);
        
        for (int d = 0; d < D; d++) {
            int N = docs[d].size();
            assignments[d].resize(N);
            
            for (int n = 0; n < N; n++) {
                int topic = topicDist(rng);
                int word = docs[d][n];
                
                assignments[d][n] = topic;
                ndk[d][topic]++;
                nkw[topic][word]++;
                nk[topic]++;
            }
        }
    }
    
    void train(int iterations = 500) {
        cout << "Training LDA with " << K << " topics for " << iterations << " iterations..." << endl;
        
        int D = docs.size();
        
        for (int iter = 0; iter < iterations; iter++) {
            for (int d = 0; d < D; d++) {
                int N = docs[d].size();
                
                for (int n = 0; n < N; n++) {
                    int oldTopic = assignments[d][n];
                    int word = docs[d][n];
                    
                    // Remove old assignment
                    ndk[d][oldTopic]--;
                    nkw[oldTopic][word]--;
                    nk[oldTopic]--;
                    
                    // Calculate topic probabilities
                    vector<double> probs(K);
                    for (int k = 0; k < K; k++) {
                        double docTopic = (ndk[d][k] + alpha) / (N - 1 + K * alpha);
                        double topicWord = (nkw[k][word] + beta) / (nk[k] + V * beta);
                        probs[k] = docTopic * topicWord;
                    }
                    
                    // Sample new topic
                    double sum = 0;
                    for (double p : probs) sum += p;
                    
                    uniform_real_distribution<double> uni(0, sum);
                    double r = uni(rng);
                    
                    double cumsum = 0;
                    int newTopic = 0;
                    for (int k = 0; k < K; k++) {
                        cumsum += probs[k];
                        if (r <= cumsum) {
                            newTopic = k;
                            break;
                        }
                    }
                    
                    // Assign new topic
                    assignments[d][n] = newTopic;
                    ndk[d][newTopic]++;
                    nkw[newTopic][word]++;
                    nk[newTopic]++;
                }
            }
            
            if ((iter + 1) % 50 == 0) {
                cout << "  Completed iteration " << (iter + 1) << "/" << iterations << endl;
            }
        }
        
        cout << "Training completed!" << endl;
    }
    
    vector<double> getTopicDistribution(int docId) {
        vector<double> dist(K, 0.0);
        int totalWords = 0;
        
        for (int k = 0; k < K; k++) {
            totalWords += ndk[docId][k];
        }
        
        if (totalWords > 0) {
            for (int k = 0; k < K; k++) {
                dist[k] = (ndk[docId][k] + alpha) / (totalWords + K * alpha);
            }
        }
        
        return dist;
    }
    
    vector<pair<string, double>> getTopWords(int topic, int topN = 10) {
        vector<pair<int, double>> wordScores;
        
        for (int w = 0; w < V; w++) {
            double score = (nkw[topic][w] + beta) / (nk[topic] + V * beta);
            wordScores.push_back({w, score});
        }
        
        // Sort by score
        sort(wordScores.begin(), wordScores.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
        
        vector<pair<string, double>> result;
        for (int i = 0; i < min(topN, V); i++) {
            result.push_back({id2word[wordScores[i].first], wordScores[i].second});
        }
        
        return result;
    }
    
    int predictTopic(const vector<string>& tokens) {
        vector<double> topicScores(K, 0.0);
        
        for (const auto& token : tokens) {
            auto it = word2id.find(token);
            if (it != word2id.end()) {
                int wordId = it->second;
                for (int k = 0; k < K; k++) {
                    double prob = (nkw[k][wordId] + beta) / (nk[k] + V * beta);
                    topicScores[k] += log(prob + 1e-10);
                }
            }
        }
        
        // Find best topic
        return max_element(topicScores.begin(), topicScores.end()) - topicScores.begin();
    }
    
    void printTopics() {
        cout << "\n=== DISCOVERED TOPICS ===" << endl;
        for (int k = 0; k < K; k++) {
            auto topWords = getTopWords(k, 8);
            cout << "\nTopic " << k << ":" << endl;
            cout << "  Top words: ";
            for (size_t i = 0; i < topWords.size(); i++) {
                cout << topWords[i].first << " (" << fixed << setprecision(3) 
                     << topWords[i].second << ")";
                if (i < topWords.size() - 1) cout << ", ";
            }
            cout << endl;
        }
    }
};

// ========== TOPIC LABELER ==========
class TopicLabeler {
private:
    map<int, string> topicLabels;
    
    // Helper function to convert string to lowercase
    string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
public:
    TopicLabeler() {}
    
    void labelTopics(LDAModel& lda, int numTopics) {
        cout << "\n=== LABELING TOPICS ===" << endl;
        
        map<string, vector<string>> categoryKeywords = {
            {"Sports", {"sport", "game", "player", "team", "football", "basketball", 
                       "soccer", "win", "match", "score", "champion", "tournament",
                       "athlete", "league", "coach", "training", "fitness"}},
            
            {"Business", {"business", "company", "market", "stock", "profit", 
                         "revenue", "investment", "financial", "corporate", 
                         "economy", "industry", "executive", "startup", "money",
                         "growth", "strategy", "management"}},
            
            {"Education", {"education", "student", "school", "university", "teacher",
                          "study", "learn", "research", "academic", "college",
                          "class", "course", "degree", "professor", "learning",
                          "knowledge", "campus", "scholarship"}},
            
            {"Technology", {"technology", "software", "computer", "digital", "data",
                           "internet", "system", "program", "code", "application",
                           "network", "device", "mobile", "web", "cloud", "ai",
                           "artificial", "intelligence", "machine"}},
            
            {"Health", {"health","sick", "medical", "doctor", "hospital", "patient",
                       "treatment", "disease", "medicine", "care", "clinical",
                       "fitness", "nutrition", "wellness", "therapy", "mental",
                       "physical", "healthcare", "nurse", "surgery"}}
        };
        
        for (int k = 0; k < numTopics; k++) {
            auto topWords = lda.getTopWords(k, 15);
            
            // Score each category
            map<string, double> categoryScores;
            for (const auto& wordPair : topWords) {
                string word = toLower(wordPair.first);
                
                for (const auto& category : categoryKeywords) {
                    // Check if word matches any keyword
                    for (const auto& keyword : category.second) {
                        string keywordLower = toLower(keyword);
                        if (word.find(keywordLower) != string::npos || 
                            keywordLower.find(word) != string::npos) {
                            categoryScores[category.first] += wordPair.second * 10;
                        }
                    }
                }
            }
            
            // Find best category
            string bestLabel = "General";
            double bestScore = 0;
            
            for (const auto& pair : categoryScores) {
                if (pair.second > bestScore) {
                    bestScore = pair.second;
                    bestLabel = pair.first;
                }
            }
            
            // Also check for exact category names in top words
            for (const auto& wordPair : topWords) {
                string word = toLower(wordPair.first);
                for (const auto& category : categoryKeywords) {
                    string categoryStart = toLower(category.first.substr(0, 4));
                    if (word.find(categoryStart) != string::npos) {
                        bestLabel = category.first;
                        break;
                    }
                }
                if (bestLabel != "General") break;
            }
            
            topicLabels[k] = bestLabel;
            cout << "Topic " << k << " -> " << bestLabel << endl;
        }
    }
    
    string getLabel(int topicId) {
        auto it = topicLabels.find(topicId);
        return (it != topicLabels.end()) ? it->second : "Unknown";
    }
};

// ========== MAIN SYSTEM ==========
class TopicModelingSystem {
private:
    TextPreprocessor preprocessor;
    vector<Document> documents;
    LDAModel lda;
    TopicLabeler labeler;
    
public:
    TopicModelingSystem(int numTopics = 5) : lda(numTopics) {}
    
    void addDocument(const string& text, const string& label = "") {
        int id = documents.size();
        Document doc(id, text, label);
        doc.tokens = preprocessor.process(text);
        documents.push_back(doc);
    }
    
    void train() {
        cout << "\n========== TRAINING ==========" << endl;
        cout << "Documents: " << documents.size() << endl;
        
        // Build vocabulary
        lda.buildVocabulary(documents);
        
        // Prepare documents
        lda.prepareDocuments(documents);
        
        // Initialize and train
        lda.initialize();
        lda.train(300);  // 300 iterations
        
        // Label topics
        labeler.labelTopics(lda, 5);
        
        // Predict topics for training documents
        for (auto& doc : documents) {
            int topic = lda.predictTopic(doc.tokens);
            doc.predictedLabel = labeler.getLabel(topic);
            
            // Calculate confidence
            auto dist = lda.getTopicDistribution(doc.id);
            doc.confidence = *max_element(dist.begin(), dist.end());
        }
        
        cout << "\nTraining complete!" << endl;
    }
    
    void evaluate() {
        cout << "\n========== EVALUATION ==========" << endl;
        
        int correct = 0;
        int total = 0;
        
        for (const auto& doc : documents) {
            if (!doc.trueLabel.empty()) {
                total++;
                if (doc.predictedLabel == doc.trueLabel) {
                    correct++;
                }
            }
        }
        
        if (total > 0) {
            double accuracy = 100.0 * correct / total;
            cout << "Accuracy: " << fixed << setprecision(1) << accuracy 
                 << "% (" << correct << "/" << total << ")" << endl;
        }
        
        // Show topic distribution
        map<string, int> counts;
        for (const auto& doc : documents) {
            counts[doc.predictedLabel]++;
        }
        
        cout << "\nTopic Distribution:" << endl;
        for (const auto& pair : counts) {
            cout << "  " << setw(12) << left << pair.first << ": " 
                 << pair.second << " documents" << endl;
        }
        
        // Show confidence
        double avgConfidence = 0;
        for (const auto& doc : documents) {
            avgConfidence += doc.confidence;
        }
        avgConfidence /= documents.size();
        
        cout << "Average confidence: " << fixed << setprecision(3) << avgConfidence << endl;
    }
    
    void printSampleResults(int numSamples = 8) {
        cout << "\n========== SAMPLE RESULTS ==========" << endl;
        
        for (int i = 0; i < min(numSamples, (int)documents.size()); i++) {
            const auto& doc = documents[i];
            cout << "\nDocument " << i << ":" << endl;
            cout << "  Text: " << (doc.originalText.length() > 60 ? 
                  doc.originalText.substr(0, 60) + "..." : doc.originalText) << endl;
            
            if (!doc.trueLabel.empty()) {
                cout << "  True: " << doc.trueLabel << endl;
            }
            
            cout << "  Predicted: " << doc.predictedLabel 
                 << " (confidence: " << fixed << setprecision(2) << doc.confidence << ")" << endl;
        }
    }
    
    string predict(const string& text) {
        vector<string> tokens = preprocessor.process(text);
        int topic = lda.predictTopic(tokens);
        return labeler.getLabel(topic);
    }
    
    void printTopics() {
        lda.printTopics();
    }
};

// ========== MAIN FUNCTION ==========
int main() {
    cout << "==========================================" << endl;
    cout << "      TOPIC MODELING WITH GOOD ACCURACY   " << endl;
    cout << "==========================================" << endl;
    
    TopicModelingSystem system(5);
    
    // ===== LOAD TRAINING DATA =====
    cout << "\nLoading training data..." << endl;
    
    // Clear, unambiguous training examples
    // Sports
    system.addDocument("Basketball players compete in championship games scoring points and winning tournaments", "Sports");
    system.addDocument("Football teams play matches scoring goals and winning championships in sports tournaments", "Sports");
    system.addDocument("Tennis athletes compete in grand slam tournaments playing matches and winning trophies", "Sports");
    system.addDocument("Soccer teams train for world cup competitions playing games and scoring goals", "Sports");
    system.addDocument("Baseball players hit runs in league games competing for championships", "Sports");
    
    // Business
    system.addDocument("Companies report profits and revenue growth in business markets with financial investments", "Business");
    system.addDocument("Corporate executives manage business operations and financial strategies for profit", "Business");
    system.addDocument("Startup companies secure venture capital funding for business expansion and growth", "Business");
    system.addDocument("Stock markets trade shares and generate financial returns for investors", "Business");
    system.addDocument("Business industries develop economic strategies for corporate success and profit", "Business");
    
    // Education
    system.addDocument("University students study academic courses and learn from professors in education", "Education");
    system.addDocument("School teachers educate students in classroom lessons with academic curriculum", "Education");
    system.addDocument("College professors teach courses and conduct research in academic institutions", "Education");
    system.addDocument("Students learn knowledge and skills through educational programs and study", "Education");
    system.addDocument("Academic research studies topics and publishes findings in educational journals", "Education");
    
    // Technology
    system.addDocument("Software developers program computer applications and write code for systems", "Technology");
    system.addDocument("Technology companies create digital systems and software applications", "Technology");
    system.addDocument("Computer engineers build hardware and develop software programs", "Technology");
    system.addDocument("Programming code creates applications and software for digital technology", "Technology");
    system.addDocument("Internet networks connect devices and enable digital communication systems", "Technology");
    
    // Health
    system.addDocument("Medical doctors treat patients in hospitals providing healthcare and treatment", "Health");
    system.addDocument("Healthcare professionals give medical care and clinical treatment to patients", "Health");
    system.addDocument("Hospital patients receive medical attention and clinical care from doctors", "Health");
    system.addDocument("Medical research studies diseases and develops treatment options", "Health");
    system.addDocument("Healthcare providers deliver medical services and patient care", "Health");
    
    // ===== TRAIN =====
    system.train();
    
    // ===== EVALUATE =====
    system.evaluate();
    
    // ===== SHOW TOPICS =====
    system.printTopics();
    
    // ===== SHOW SAMPLES =====
    system.printSampleResults();
    
    // ===== TEST PREDICTIONS =====
    cout << "\n========== TESTING NEW DOCUMENTS ==========" << endl;
    
    vector<pair<string, string>> testCases = {
        {"The basketball team won the championship game with great performance", "Sports"},
        {"Company reported record profits and revenue growth this quarter", "Business"},
        {"University students are studying for final exams this week", "Education"},
        {"Software engineers developed new applications using programming code", "Technology"},
        {"Doctors provided medical treatment to hospital patients yesterday", "Health"},
        {"Football players train for upcoming tournament matches", "Sports"},
        {"Corporate executives attended business conference on market trends", "Business"},
        {"Professors teach academic courses to college students", "Education"},
        {"Computer programmers write code for new software systems", "Technology"},
        {"Medical researchers discovered new treatment for heart disease", "Health"},
        {"He loves to play football","Sports"},
        {"I am very sick","Health"},
         {"He teaches us english","Education"}
    

    };
    
    int correct = 0;
    for (size_t i = 0; i < testCases.size(); i++) {
        string prediction = system.predict(testCases[i].first);
        string expected = testCases[i].second;
        
        cout << "\nTest " << (i+1) << ":" << endl;
        cout << "  Text: " << testCases[i].first << endl;
        cout << "  Expected: " << expected << endl;
        cout << "  Predicted: " << prediction << endl;
        
        if (prediction == expected) {
            cout << "   CORRECT" << endl;
            correct++;
        } else {
            cout << "   WRONG" << endl;
        }
    }
    
    double testAccuracy = 100.0 * correct / testCases.size();
    cout << "\nTest Accuracy: " << fixed << setprecision(1) 
         << testAccuracy << "% (" << correct << "/" << testCases.size() << ")" << endl;
    
    cout << "\n==========================================" << endl;
    cout << "      SYSTEM READY FOR USE                " << endl;
    cout << "==========================================" << endl;
    
    return 0;
}