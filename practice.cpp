#include <bits/stdc++.h>
using namespace std;

int main() {
    // ======= Hardcoded documents =======
    vector<string> docs = {
        "I love football",
        "AI is changing the world",
        "The football match was exciting",
        "Machine learning and AI are the future",
        "Football players are very skilled"
    };

    int D = docs.size();

    // ======= Build vocabulary =======
    map<string,int> word2id;
    vector<string> vocab;
    vector<vector<int>> documents(D);

    for(int d=0; d<D; d++){
        stringstream ss(docs[d]);
        string word;
        while(ss >> word){
            // lowercase for uniformity
            transform(word.begin(), word.end(), word.begin(), ::tolower);

            if(word2id.find(word) == word2id.end()){
                int id = vocab.size();
                word2id[word] = id;
                vocab.push_back(word);
            }
            documents[d].push_back(word2id[word]);
        }
    }

    int V = vocab.size();
    int K = 2; // number of topics
    double alpha = 0.1, beta = 0.1;

    // ======= Random topic assignment =======
    vector<vector<int>> z(D);
    vector<int> nw(V*K, 0);  // word-topic count
    vector<int> nd(D*K, 0);  // doc-topic count
    vector<int> nwsum(K,0);  // total words per topic

    srand(time(0));
    for(int d=0; d<D; d++){
        for(int w : documents[d]){
            int topic = rand() % K;
            z[d].push_back(topic);
            nw[w*K + topic]++;
            nd[d*K + topic]++;
            nwsum[topic]++;
        }
    }

    // ======= One iteration of Gibbs sampling =======
    for(int d=0; d<D; d++){
        for(int i=0; i<documents[d].size(); i++){
            int w = documents[d][i];
            int old_topic = z[d][i];

            nw[w*K + old_topic]--;
            nd[d*K + old_topic]--;
            nwsum[old_topic]--;

            vector<double> p(K);
            for(int k=0;k<K;k++){
                p[k] = (nw[w*K+k]+beta)/(nwsum[k]+V*beta) *
                       (nd[d*K+k]+alpha);
            }

            double sum = accumulate(p.begin(), p.end(), 0.0);
            double r = ((double) rand() / RAND_MAX) * sum;

            int new_topic=0;
            double cum=0;
            for(int k=0;k<K;k++){
                cum += p[k];
                if(r <= cum){
                    new_topic = k;
                    break;
                }
            }

            z[d][i] = new_topic;
            nw[w*K + new_topic]++;
            nd[d*K + new_topic]++;
            nwsum[new_topic]++;
        }
    }

    // ======= Print topic-word distribution =======
    cout << "\nTopic-Word Distribution (counts):\n";
    for(int k=0;k<K;k++){
        cout << "Topic " << k << ": ";
        for(int v=0; v<V; v++){
            cout << vocab[v] << "(" << nw[v*K+k] << ") ";
        }
        cout << endl;
    }

    return 0;
}
