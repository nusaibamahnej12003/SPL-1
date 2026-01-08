from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.naive_bayes import MultinomialNB

# -------------------------
# Sample dataset (50 documents, 5 topics)
documents = [
    # Sports
    "cricket match was exciting and players performed well",
    "football team won the championship after a tough game",
    "the athlete trained hard to win the marathon",
    "basketball match was intense with last minute scores",
    "fans celebrated the victory of their favorite team",
    "the tournament showcased emerging young players",
    "team strategy led to a remarkable win",
    "player broke the record in the cricket league",
    "stadium was full during the football finals",
    "athletes prepared diligently for the upcoming games",

    # Politics
    "government announced new policy reforms",
    "election results surprised many citizens",
    "parliament passed a significant bill yesterday",
    "political leaders debated on the new legislation",
    "voters expressed their opinions during the elections",
    "the campaign strategies were discussed extensively",
    "government plans to improve infrastructure",
    "politicians addressed the public concerns",
    "policy changes will affect healthcare and education",
    "the president met with international leaders",

    # Technology
    "new AI technology is transforming industries rapidly",
    "software development trends show increased adoption of cloud computing",
    "tech companies innovate continuously to stay competitive",
    "machine learning models are becoming more accurate",
    "programmers develop mobile applications for users worldwide",
    "artificial intelligence helps automate routine tasks",
    "emerging technologies drive digital transformation",
    "startups focus on blockchain and fintech innovations",
    "developers contribute to open-source software projects",
    "robotics and AI are revolutionizing manufacturing",

    # Health
    "healthcare system requires more investment for quality service",
    "doctors recommend regular exercise for better health",
    "medical research advances treatments for chronic diseases",
    "patients benefit from early diagnosis and care",
    "nutrition and diet impact overall wellbeing",
    "mental health awareness is increasing globally",
    "vaccination programs reduce the spread of diseases",
    "fitness activities improve physical and mental health",
    "health professionals are improving patient care",
    "new medical devices assist in surgeries and treatment",

    # Environment
    "climate change affects global weather patterns",
    "deforestation leads to loss of biodiversity",
    "renewable energy sources reduce pollution and carbon emissions",
    "conservation efforts help protect endangered species",
    "environmental policies aim to reduce carbon footprint",
    "recycling and waste management are important for sustainability",
    "oceans and rivers face threats from plastic pollution",
    "renewable energy projects are being implemented worldwide",
    "air quality improves with reduced industrial emissions",
    "green initiatives promote eco-friendly practices"
]

# Corresponding labels
labels = ["Sports"]*10 + ["Politics"]*10 + ["Technology"]*10 + ["Health"]*10 + ["Environment"]*10

# -------------------------
# Convert text to TF-IDF features
vectorizer = TfidfVectorizer()
X = vectorizer.fit_transform(documents)

# Train Naive Bayes classifier
clf = MultinomialNB()
clf.fit(X, labels)

# -------------------------
# Predict topics for new documents
new_docs = [
    "the football team played an amazing match",
    "new AI models are transforming software industry",
    "government announced new healthcare policy",
    "climate change is affecting weather patterns globally",
    "cricket is a famous game",
    "you should take care of your health",
    "yesterday was election",
        "software"

]

X_new = vectorizer.transform(new_docs)
predictions = clf.predict(X_new)

for doc, pred in zip(new_docs, predictions):
    print(f"Document: '{doc}'")
    print(f"Predicted Topic: {pred}\n")
