from sklearn.feature_extraction.text import CountVectorizer
from sklearn.decomposition import LatentDirichletAllocation
from sklearn.linear_model import LogisticRegression
from sklearn.pipeline import Pipeline
from sklearn.base import BaseEstimator, TransformerMixin
import numpy as np

# Your 50 training documents and labels (same as before)
documents = [ ... ]  # Paste the list here

labels = ["Sports"]*10 + ["Politics"]*10 + ["Technology"]*10 + ["Health"]*10 + ["Environment"]*10

# New documents
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

# Custom transformer to get LDA topic probabilities
class LDAFeatures(BaseEstimator, TransformerMixin):
    def __init__(self, n_components=5):
        self.n_components = n_components
        self.lda = LatentDirichletAllocation(n_components=n_components, random_state=42)
        self.vectorizer = CountVectorizer(stop_words='english')
    
    def fit(self, X, y=None):
        counts = self.vectorizer.fit_transform(X)
        self.lda.fit(counts)
        return self
    
    def transform(self, X):
        counts = self.vectorizer.transform(X)
        topic_probs = self.lda.transform(counts)
        return topic_probs  # Shape: (n_docs, 5)

# Pipeline: LDA features -> Logistic Regression (better than Naive Bayes here)
pipeline = Pipeline([
    ('lda', LDAFeatures(n_components=5)),
    ('clf', LogisticRegression(random_state=42))
])

# Train the model (LDA is unsupervised, classifier uses labels)
pipeline.fit(documents, labels)

# Predict on new documents
predictions = pipeline.predict(new_docs)
probabilities = pipeline.predict_proba(new_docs)

print("Predictions (using LDA + supervised classifier):")
for doc, pred, probs in zip(new_docs, predictions, probabilities):
    confidence = np.max(probs)
    print(f"Document: '{doc}'")
    print(f"Predicted Topic: {pred} (confidence: {confidence:.2f})\n")

# Training accuracy (should be 1.00)
train_accuracy = pipeline.score(documents, labels)
print(f"Training accuracy: {train_accuracy:.2f}")