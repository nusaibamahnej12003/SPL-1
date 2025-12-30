#include <bits/stdc++.h>
using namespace std;

string lowercase(const string &s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

int main() {
    ifstream fin("large_dataset.txt");
    if(!fin){
        cout << "Error: cannot open file large_dataset.txt\n";
        return 1;
    }

    vector<string> corpus;
    string line;
    while(getline(fin, line)){
        if(line.empty()) continue;
        corpus.push_back(line);
    }
    fin.close();
    int corpus_size = corpus.size();

    vector<string> code_docs = {
        "The basketball championship game featured intense competition between top teams",
"Professional football teams compete in tournaments to win championship titles",
"Tennis athletes require skill and endurance for tournament matches",
"Soccer players train extensively to score goals in matches",
"Baseball games involve strategic hitting and base running",
"Golf tournaments test precision on challenging courses worldwide",
"Swimming competitions showcase various strokes and racing techniques",
"Running events demonstrate speed and endurance capabilities",
"Cycling races demand stamina across different terrains",
"Boxing matches combine punching power with defensive skills",
"Companies report quarterly profits and financial growth results",
"Market analysis affects stock prices and investment choices",
"Banks provide financial services including loans and accounts",
"Office environments require teamwork and task completion",
"Factories produce goods using automated assembly lines",
"Retail stores sell products to consumer customers",
"Restaurants serve quality meals in dining settings",
"Hotels offer accommodation with various guest amenities",
"Airplanes transport passengers safely to destinations",
"Ships carry cargo across international waters",
"University students pursue academic degrees through study",
"Schools provide foundational education for young learners",
"College campuses offer diverse learning environments",
"Libraries contain extensive collections for research",
"Classrooms facilitate interactive learning experiences",
"Homework assignments reinforce classroom teachings",
"Examinations assess student knowledge acquisition",
"Doctors diagnose medical conditions and prescribe treatments",
"Nurses provide essential patient care in hospitals",
"Dentists maintain oral health through dental procedures",
"Pharmacies dispense prescribed medications to patients",
"Clinics offer medical services for various needs",
"Ambulances provide emergency medical transportation",
"Apples are nutritious fruits enjoyed worldwide",
"Bananas provide potassium and natural energy",
"Oranges are rich sources of vitamin C",
"Grapes are used for wine production",
"Strawberries are sweet berries popular in desserts",
"Watermelons are refreshing summer fruits",
"Carrots are vitamin-rich root vegetables",
"Tomatoes are versatile cooking ingredients",
"Potatoes are staple food items globally",
"Software developers create computer applications",
"Technology companies innovate digital solutions",
"Computer systems process complex data sets",
"Internet connectivity enables global communication",
"Digital cameras capture high-quality images",
"Television broadcasts entertainment programs",
"Radio stations transmit audio content",
"Newspapers deliver daily news coverage",
"Books preserve knowledge and stories",
"Medical research advances treatment options",
"Healthcare professionals improve patient outcomes",
"Hospitals provide comprehensive medical services",
"Pharmaceutical companies develop new medicines",
"Dental clinics offer preventive care",
"Nutrition affects overall health status",
"Exercise promotes physical fitness",
"Mental health requires attention and care",
"Preventive medicine reduces disease risk",
"Rehabilitation helps recovery from injuries",
"Sports medicine treats athletic injuries",
"Physical therapy restores mobility",
"Occupational health ensures workplace safety",
"Public health protects communities",
"Veterinary medicine cares for animals",
"Alternative medicine offers complementary treatments",
"Traditional medicine uses historical practices",
"Emergency medicine handles urgent cases",
"Pediatrics focuses on child health",
"Geriatrics addresses aging concerns",
"Oncology treats cancer patients",
"Immunology researches immune responses",
"Cardiology deals with heart health",
"Neurology studies nervous system",
"Psychology examines mental processes",
"Dermatology treats skin conditions",
"Rheumatology deals with joint disorders",
"Ophthalmology focuses on eye health",
"Orthopedics addresses bone issues",
"Radiology uses imaging technology",
"Pathology studies disease causes",
"Microbiology examines microorganisms",
"Genetics studies hereditary traits",
"Epidemiology tracks disease patterns",
"Biochemistry analyzes chemical processes",
"Pharmacology studies drug effects",
"Toxicology examines harmful substances",
"Endocrinology studies hormone systems",
"Gastroenterology treats digestive issues",
"Urology addresses urinary health",
"Nephrology focuses on kidney function",
"Pulmonology treats respiratory conditions",
"Hematology studies blood disorders"
    };
    int code_size = code_docs.size();

    map<string,int> word2id;
    vector<string> vocab;
    vector<vector<int>> documents;

    auto process_line = [&](const string &doc) -> vector<int> {
        stringstream ss(doc);
        string word;
        vector<int> doc_ids;
        while(ss >> word){
            word = lowercase(word);
            if(word2id.find(word) == word2id.end()){
                int id = vocab.size();
                word2id[word] = id;
                vocab.push_back(word);
            }
            doc_ids.push_back(word2id[word]);
        }
        return doc_ids;
    };

    for(auto &doc : corpus) documents.push_back(process_line(doc));
    for(auto &doc : code_docs) documents.push_back(process_line(doc));

    int D = documents.size();
    int V = vocab.size();
    int K = 5; 
    double alpha = 0.1, beta = 0.01;
    int iterations = 500;

    vector<vector<int>> z(D);
    vector<int> nw(V*K, 0);        
    vector<int> nd(D*K, 0);        
    vector<int> nwsum(K, 0);       
    vector<int> ndsum(D,0);        

    srand(time(0));
    for(int d=0; d<D; d++){
        for(int w : documents[d]){
            int topic = rand() % K;
            z[d].push_back(topic);
            nw[w*K + topic]++;
            nd[d*K + topic]++;
            nwsum[topic]++;
        }
        ndsum[d] = documents[d].size();
    }

    for(int it=0; it<iterations; it++){
        for(int d=0; d<D; d++){
            for(int i=0; i<documents[d].size(); i++){
                int w = documents[d][i];
                int old_topic = z[d][i];

                nw[w*K + old_topic]--;
                nd[d*K + old_topic]--;
                nwsum[old_topic]--;
                ndsum[d]--;

                vector<double> p(K);
                double sum = 0.0;
                for(int k=0;k<K;k++){
                    p[k] = (nw[w*K+k]+beta)/(nwsum[k]+V*beta) *
                           (nd[d*K+k]+alpha);
                    sum += p[k];
                }

                double r = ((double) rand() / RAND_MAX) * sum;
                int new_topic = 0;
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
                ndsum[d]++;
            }
        }
    }

   vector<vector<int>> topic_docs(K);

for(int d=corpus_size; d<D; d++){ 
    vector<double> theta(K);
    double sum = 0.0;
    for(int k=0;k<K;k++){
        theta[k] = (nd[d*K + k] + alpha) / (ndsum[d] + K*alpha);
    }
    
    int dominant_topic = max_element(theta.begin(), theta.end()) - theta.begin();
    topic_docs[dominant_topic].push_back(d - corpus_size);
}


cout << "\nDocuments clustered by dominant topic:\n";
for(int k=0;k<K;k++){
    cout << "Topic " << k << ":\n";
    for(int idx : topic_docs[k]){
        cout << "  - Document " << idx << ": \"" << code_docs[idx] << "\"\n";
    }
    cout << endl;
}

    return 0;
}
