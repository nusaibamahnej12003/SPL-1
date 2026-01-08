#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <random>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <numeric>

// Comprehensive English stopwords
const std::unordered_set<std::string> STOPWORDS = {
    "the", "be", "to", "of", "and", "a", "in", "that", "have", "i",
    "it", "for", "not", "on", "with", "he", "as", "you", "do", "at",
    "this", "but", "his", "by", "from", "they", "we", "say", "her", "she",
    "or", "an", "will", "my", "one", "all", "would", "there", "their",
    "what", "so", "up", "out", "if", "about", "who", "get", "which", "go",
    "me", "when", "make", "can", "like", "time", "no", "just", "him", "know",
    "take", "people", "into", "year", "your", "good", "some", "could", "them",
    "see", "other", "than", "then", "now", "look", "only", "come", "its", "over",
    "think", "also", "back", "after", "use", "two", "how", "our", "work",
    "first", "well", "way", "even", "new", "want", "because", "any", "these",
    "give", "day", "most", "us", "is", "was", "are", "been", "has", "had",
    "were", "said", "did", "having", "may", "should", "does", "being", "am",
    "where", "much", "through", "such", "before", "must", "too", "under",
    "own", "same", "while", "those", "both", "each", "few", "more", "very",
    "many", "here", "off", "down", "during", "again", "further", "once",
    "might", "still", "around", "really", "something", "every", "another",
    "found", "though", "made", "since", "without", "last", "long", "ever",
    "never", "per", "via", "etc", "thus", "hence", "therefore", "however"
};

class LDA {
private:
    int K;           // number of topics
    int V;           // vocabulary size
    int D;           // number of documents
    double alpha;    // document-topic prior
    double beta;     // topic-word prior
    
    std::vector<std::vector<int>> docs;
    std::vector<std::vector<int>> z;
    std::vector<std::string> vocabulary;
    std::unordered_map<std::string, int> word2id;
    std::vector<std::string> original_lines;
    std::vector<int> word_freq;
    
    std::vector<std::vector<int>> n_dk;  // n_dk[d][k]: topic k in doc d
    std::vector<std::vector<int>> n_kw;  // n_kw[k][w]: word w in topic k
    std::vector<int> n_k;                // total words in topic k
    std::vector<int> n_d;                // total words in doc d
    
    std::vector<std::string> topic_labels;
    std::mt19937 rng;
    std::vector<double> log_likelihoods;
    double best_log_likelihood;
    
public:
    LDA(int num_topics, double alpha_val, double beta_val) 
        : K(num_topics), alpha(alpha_val), beta(beta_val), 
          rng(std::random_device{}()), best_log_likelihood(-INFINITY) {}
    
    std::vector<std::string> preprocess(const std::string& text) {
        std::vector<std::string> words;
        std::string word;
        for (char c : text) {
            if (std::isalpha(c)) {
                word += std::tolower(c);
            } else if (!word.empty()) {
                if (word.length() >= 3 && STOPWORDS.find(word) == STOPWORDS.end()) {
                    words.push_back(word);
                }
                word.clear();
            }
        }
        if (!word.empty() && word.length() >= 3 && STOPWORDS.find(word) == STOPWORDS.end()) {
            words.push_back(word);
        }
        return words;
    }

    void load_documents(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            exit(1);
        }
        std::string line;
        std::unordered_map<std::string, int> global_word_count;
        std::vector<std::vector<std::string>> all_words;

        while (std::getline(file, line)) {
            if (line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos) continue;
            original_lines.push_back(line);
            auto words = preprocess(line);
            all_words.push_back(words);
            for (const auto& w : words) global_word_count[w]++;
        }
        if (original_lines.empty()) {
            std::cerr << "Error: No valid documents found in file" << std::endl;
            exit(1);
        }
        int min_word_freq = std::max(1, (int)(original_lines.size() * 0.01));
        for (const auto& pair : global_word_count) {
            if (pair.second >= min_word_freq) {
                int id = vocabulary.size();
                word2id[pair.first] = id;
                vocabulary.push_back(pair.first);
                word_freq.push_back(pair.second);
            }
        }
        for (size_t i = 0; i < all_words.size(); i++) {
            std::vector<int> doc;
            for (const auto& word : all_words[i]) {
                if (word2id.find(word) != word2id.end()) {
                    doc.push_back(word2id[word]);
                }
            }
            docs.push_back(doc.empty() ? std::vector<int>() : doc);
        }
        D = docs.size();
        V = vocabulary.size();
        std::cout << "Loaded " << D << " documents\n";
        std::cout << "Vocabulary size: " << V << " unique words\n";
        int total_words = 0, non_empty_docs = 0;
        for (const auto& doc : docs) {
            total_words += doc.size();
            if (!doc.empty()) non_empty_docs++;
        }
        if (non_empty_docs > 0) {
            std::cout << "Average document length: " << std::fixed << std::setprecision(1)
                      << (double)total_words / non_empty_docs << " words\n";
        }
        std::cout << std::endl;
    }

    void initialize() {
        n_dk.assign(D, std::vector<int>(K, 0));
        n_kw.assign(K, std::vector<int>(V, 0));
        n_k.assign(K, 0);
        n_d.assign(D, 0);
        z.resize(D);

        std::uniform_int_distribution<int> topic_dist(0, K - 1);
        for (int d = 0; d < D; d++) {
            z[d].resize(docs[d].size());
            for (size_t n = 0; n < docs[d].size(); n++) {
                int topic = topic_dist(rng);
                z[d][n] = topic;
                int word = docs[d][n];
                n_dk[d][topic]++;
                n_kw[topic][word]++;
                n_k[topic]++;
                n_d[d]++;
            }
        }
        std::cout << "Initialized " << K << " topics with random assignments\n";
    }
    
    int sample_topic(int d, int n) {
        int word = docs[d][n];
        int old_topic = z[d][n];
        n_dk[d][old_topic]--;
        n_kw[old_topic][word]--;
        n_k[old_topic]--;
        double sum = 0.0;
        std::vector<double> p(K);
        for (int k = 0; k < K; k++) {
            double phi = (n_kw[k][word] + beta) / (n_k[k] + V * beta);
            double theta = (n_dk[d][k] + alpha) / (n_d[d] - 1 + K * alpha);
            p[k] = phi * theta;
            sum += p[k];
        }
        if (sum > 0) for (int k = 0; k < K; k++) p[k] /= sum;
        else for (int k = 0; k < K; k++) p[k] = 1.0 / K;
        std::discrete_distribution<int> dist(p.begin(), p.end());
        int new_topic = dist(rng);
        z[d][n] = new_topic;
        n_dk[d][new_topic]++;
        n_kw[new_topic][word]++;
        n_k[new_topic]++;
        return new_topic;
    }

    double compute_perplexity() {
        double log_likelihood = 0.0;
        int total_words = 0;
        for (int d = 0; d < D; d++) {
            for (size_t n = 0; n < docs[d].size(); n++) {
                int word = docs[d][n];
                double word_prob = 0.0;
                for (int k = 0; k < K; k++) {
                    double theta = (n_dk[d][k] + alpha) / (n_d[d] + K * alpha);
                    double phi = (n_kw[k][word] + beta) / (n_k[k] + V * beta);
                    word_prob += theta * phi;
                }
                if (word_prob > 0) {
                    log_likelihood += std::log(word_prob);
                    total_words++;
                }
            }
        }
        return std::exp(-log_likelihood / total_words);
    }

    double compute_log_likelihood() {
        double ll = 0.0;
        for (int d = 0; d < D; d++) {
            for (size_t n = 0; n < docs[d].size(); n++) {
                int word = docs[d][n];
                double word_prob = 0.0;
                for (int k = 0; k < K; k++) {
                    double theta = (n_dk[d][k] + alpha) / (n_d[d] + K * alpha);
                    double phi = (n_kw[k][word] + beta) / (n_k[k] + V * beta);
                    word_prob += theta * phi;
                }
                if (word_prob > 0) ll += std::log(word_prob);
            }
        }
        return ll;
    }

    void train(int iterations, int burn_in = 200) {
        std::cout << "Training LDA model...\n";
        std::cout << "Iterations: " << iterations << " (burn-in: " << burn_in << ")\n\n";
        int report_interval = std::max(10, iterations / 20);
        for (int iter = 0; iter < iterations; iter++) {
            for (int d = 0; d < D; d++) {
                std::vector<int> indices(docs[d].size());
                std::iota(indices.begin(), indices.end(), 0);
                std::shuffle(indices.begin(), indices.end(), rng);
                for (int idx : indices) sample_topic(d, idx);
            }
            if ((iter + 1) % report_interval == 0 || iter == iterations - 1) {
                double ll = compute_log_likelihood();
                double perplexity = compute_perplexity();
                log_likelihoods.push_back(ll);
                if (ll > best_log_likelihood) best_log_likelihood = ll;
                std::cout << "Iteration " << std::setw(4) << (iter + 1) << "/" << iterations 
                          << " | Log-lik: " << std::fixed << std::setprecision(2) << ll
                          << " | Perpl.: " << std::setprecision(2) << perplexity << std::endl;
            }
        }
        std::cout << "\nTraining completed successfully!\n";
        std::cout << "Final perplexity: " << std::fixed << std::setprecision(2) << compute_perplexity() << "\n\n";
    }

    void generate_topic_labels() {
        topic_labels.clear();
        topic_labels = {
            "Business",
            "Sports",
            "Health",
            "Tech",
            "Educational"
        };
    }

    std::pair<int, double> get_dominant_topic_with_prob(int d) {
        if (n_d[d] == 0) return {0, 0.0};
        int max_topic = 0;
        double max_prob = 0.0;
        for (int k = 0; k < K; k++) {
            double prob = (n_dk[d][k] + alpha) / (n_d[d] + K * alpha);
            if (prob > max_prob) {
                max_prob = prob;
                max_topic = k;
            }
        }
        return {max_topic, max_prob};
    }

    std::vector<double> get_topic_distribution(int d) {
        std::vector<double> dist(K);
        if (n_d[d] == 0) {
            for (int k = 0; k < K; k++) dist[k] = 1.0 / K;
            return dist;
        }
        for (int k = 0; k < K; k++)
            dist[k] = (n_dk[d][k] + alpha) / (n_d[d] + K * alpha);
        return dist;
    }

    std::vector<std::pair<std::string, double>> get_top_words(int topic, int n) {
        std::vector<std::pair<int, double>> word_probs;
        for (int w = 0; w < V; w++) {
            double prob = (n_kw[topic][w] + beta) / (n_k[topic] + V * beta);
            word_probs.push_back({w, prob});
        }
        std::partial_sort(word_probs.begin(), 
            word_probs.begin() + std::min(n, (int)word_probs.size()),
            word_probs.end(),
            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                return a.second > b.second;
            });
        std::vector<std::pair<std::string, double>> result;
        for (int i = 0; i < std::min(n, (int)word_probs.size()); i++)
            result.push_back({vocabulary[word_probs[i].first], word_probs[i].second});
        return result;
    }

    double calculate_topic_coherence(int topic, int top_n = 10) {
        auto top_words = get_top_words(topic, top_n);
        if (top_words.size() < 2) return 0.0;
        double coherence = 0.0;
        int pairs = 0;
        for (size_t i = 0; i < top_words.size(); i++) {
            for (size_t j = i + 1; j < top_words.size(); j++) {
                int word_i = word2id[top_words[i].first];
                int word_j = word2id[top_words[j].first];
                int co_occur = 0, occur_i = 0, occur_j = 0;
                for (int d = 0; d < D; d++) {
                    bool has_i = false, has_j = false;
                    for (int w : docs[d]) {
                        if (w == word_i) has_i = true;
                        if (w == word_j) has_j = true;
                    }
                    if (has_i && has_j) co_occur++;
                    if (has_i) occur_i++;
                    if (has_j) occur_j++;
                }
                if (co_occur > 0 && occur_i > 0 && occur_j > 0) {
                    double pmi = std::log(((double)co_occur + 1) * D / ((double)occur_i * occur_j + 1));
                    coherence += pmi;
                    pairs++;
                }
            }
        }
        return pairs > 0 ? coherence / pairs : 0.0;
    }

    void print_results() {
        generate_topic_labels();
        std::cout << "\n============= DOCUMENT-TOPIC ASSIGNMENTS =============\n\n";
        for (int d = 0; d < D; d++) {
            auto topic_prob = get_dominant_topic_with_prob(d);
            int topic = topic_prob.first;
            double prob = topic_prob.second;
            std::cout << "Document " << (d + 1) << " --> " << topic_labels[topic];
            std::cout << " (" << std::fixed << std::setprecision(0) << (prob * 100) << "% confidence)\n";
            std::cout << "  \"" << original_lines[d] << "\"\n";
            auto dist = get_topic_distribution(d);
            if (prob < 0.55) {
                std::cout << "  [Mixed:";
                for (int k = 0; k < K; k++) {
                    if (dist[k] > 0.15)
                        std::cout << " " << topic_labels[k] << " " << (int)(dist[k] * 100) << "%,";
                }
                std::cout << "]\n";
            }
            std::cout << std::endl;
        }
        std::cout << "\n============= DISCOVERED TOPICS =============\n\n";
        for (int k = 0; k < K; k++) {
            std::cout << "*** " << topic_labels[k] << " ***\n";
            auto top_words = get_top_words(k, 10);
            std::cout << "Top words: ";
            for (size_t i = 0; i < std::min((size_t)5, top_words.size()); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << top_words[i].first;
            }
            std::cout << "\nFull distribution:\n";
            for (size_t i = 0; i < top_words.size(); i++) {
                std::cout << "  " << std::setw(2) << (i + 1) << ". " 
                    << std::left << std::setw(18) << top_words[i].first 
                    << std::right << " | " << std::fixed << std::setprecision(4) 
                    << top_words[i].second << "\n";
            }
            double coherence = calculate_topic_coherence(k);
            std::cout << "Coherence score: " << std::fixed << std::setprecision(3) << coherence;
            if (coherence > 2.0) std::cout << " (excellent)";
            else if (coherence > 1.0) std::cout << " (good)";
            else if (coherence > 0.0) std::cout << " (moderate)";
            else std::cout << " (poor)";
            std::cout << "\n\n";
        }
        std::cout << "\n============= MODEL STATISTICS =============\n\n";
        if (!log_likelihoods.empty()) {
            std::cout << "Final log-likelihood: " << std::fixed << std::setprecision(2)
                      << log_likelihoods.back() << "\n";
            if (log_likelihoods.size() > 1) {
                double improvement = log_likelihoods.back() - log_likelihoods[0];
                std::cout << "Improvement: " << std::showpos << improvement << std::noshowpos << "\n";
            }
        }
        std::cout << "Final perplexity: " << std::fixed << std::setprecision(2)
                  << compute_perplexity() << "\n";
        double avg_coherence = 0.0;
        for (int k = 0; k < K; k++) avg_coherence += calculate_topic_coherence(k);
        avg_coherence /= K;
        std::cout << "Average topic coherence: " << std::fixed << std::setprecision(3)
                  << avg_coherence << "\n\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        std::cerr << "\nThe input file should contain one document per line.\n";
        std::cerr << "Example: " << argv[0] << " input.txt\n";
        return 1;
    }
    std::string filename = argv[1];
    int num_topics = 5;
    double alpha = 0.2;     // Less sparse for small document sets
    double beta = 0.05;     // Smoother topic-word distribution
    int iterations = 2000;

    std::cout << "\n============== Latent Dirichlet Allocation (LDA) ==============\n";
    std::cout << "Configuration:\n";
    std::cout << "  Topics: " << num_topics << "\n";
    std::cout << "  Alpha (doc-topic): " << alpha << "\n";
    std::cout << "  Beta (topic-word): " << beta << "\n";
    std::cout << "  Iterations: " << iterations << "\n\n";
    LDA lda(num_topics, alpha, beta);
    lda.load_documents(filename);
    lda.initialize();
    lda.train(iterations, 200);
    lda.print_results();
    return 0;
}
