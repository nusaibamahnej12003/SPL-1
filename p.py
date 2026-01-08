# Install gensim if you haven't already
# pip install gensim

from gensim import corpora, models

# -------------------------
# Sample 5-topic dataset
documents = [
    # -------- Sports (10) --------
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

    # -------- Politics (10) --------
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

    # -------- Technology (10) --------
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

    # -------- Health (10) --------
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

    # -------- Environment (10) --------
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

# Simple stopwords
stop_words = set(["the", "and", "of", "was", "are", "is", "a", "an", "by", "for", "in", "on", "to"])

# Preprocess: lowercase, tokenize, remove stopwords
texts = [[word for word in doc.lower().split() if word not in stop_words] for doc in documents]

# -------------------------
# Create dictionary and corpus
dictionary = corpora.Dictionary(texts)
corpus = [dictionary.doc2bow(text) for text in texts]

# -------------------------
# Train LDA model with 5 topics
lda_model = models.LdaModel(corpus, num_topics=5, id2word=dictionary, passes=20, random_state=42)

# -------------------------
# Inspect topics
print("---- Topics ----")
topics = lda_model.print_topics(num_words=5)
for i, topic in topics:
    print(f"Topic {i}: {topic}")

# -------------------------
# Assign labels manually based on top words
topic_labels = ["Sports", "Politics", "Technology", "Health", "Environment"]

# -------------------------
# Show document-topic probabilities and predicted label
print("\n---- Document Predictions ----")
for i, doc_bow in enumerate(corpus):
    doc_topics = lda_model.get_document_topics(doc_bow)
    # Pick topic with highest probability
    top_topic_id = max(doc_topics, key=lambda x: x[1])[0]
    predicted_label = topic_labels[top_topic_id]
    probs = [0]*5
    for topic_id, prob in doc_topics:
        probs[topic_id] = prob
    print(f"Document {i+1}: {predicted_label}, Probabilities: {[round(p,2) for p in probs]}")
