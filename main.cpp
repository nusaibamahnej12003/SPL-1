#include "topic_model.h"
#include "sentiment.h"

int main()
{
    cout << "        SIMPLE NLP ANALYSIS SYSTEM             " << endl;

    // Initialize modules
    SupervisedLDA     topicModel;
    SentimentAnalyzer sentAnalyzer;

    // Load & train topic model
    topicModel.loadData("input.txt");
    topicModel.train();

    // Input sentences
    vector<string> inputs = {
        "she reads in class 5",
        "he is very sick",
        "I love to play cricket",
        "AI market is increasing very highly",
        "This TV show is funny",
        "Wireless charging uses electromagnetic fields to transfer energy",
        "HE is Happy",
        "The Doctor's behaviour is very nice",
        "I am reading books",
        "He is a very nice teacher",
    };

    // Output table
    cout << "\n";
    cout << left << setw(30) << "INPUT"
         << setw(15) << "TOPIC"
         << setw(20) << "SENTIMENT" << endl;

    cout << string(75, '-') << endl;

    for (const string& sentence : inputs)
    {
        string topic = topicModel.predict(sentence);
        SentimentResult sr = sentAnalyzer.analyze(sentence);

        string sentiment = sr.intensity.empty()
                           ? sr.label
                           : sr.intensity + " " + sr.label;

        string display = sentence.length() > 28
                         ? sentence.substr(0, 25) + "..."
                         : sentence;

        cout << left << setw(30) << display
             << setw(15) << topic
             << setw(20) << sentiment << endl;
    }

    cout << string(75, '=') << endl;

    return 0;
}