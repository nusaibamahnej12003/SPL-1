# my_lda.py
import numpy as np
import random

class LDAModel:
    def __init__(self, docs, num_topics=4, alpha=0.1, beta=0.01, iterations=200, topic_labels=None):
        self.docs = docs
        self.num_topics = num_topics
        self.alpha = alpha
        self.beta = beta
        self.iterations = iterations
        self.topic_labels = topic_labels if topic_labels else [f"Topic {i}" for i in range(num_topics)]

        # Build vocabulary
        self.word2id = {}
        self.id2word = {}
        self.build_vocab()
        self.num_words = len(self.word2id)

        # Initialize counts
        self.doc_topic_count = np.zeros((len(docs), num_topics)) + alpha
        self.topic_word_count = np.zeros((num_topics, self.num_words)) + beta
        self.topic_count = np.zeros(num_topics) + self.num_words * beta

        # Assign random topics
        self.topics = []
        for d_index, doc in enumerate(docs):
            current_doc_topics = []
            for word in doc:
                topic = random.randint(0, num_topics-1)
                w_id = self.word2id[word]
                self.doc_topic_count[d_index, topic] += 1
                self.topic_word_count[topic, w_id] += 1
                self.topic_count[topic] += 1
                current_doc_topics.append(topic)
            self.topics.append(current_doc_topics)

    def build_vocab(self):
        idx = 0
        for doc in self.docs:
            for word in doc:
                if word not in self.word2id:
                    self.word2id[word] = idx
                    self.id2word[idx] = word
                    idx += 1

    def run(self):
        for it in range(self.iterations):
            for d_index, doc in enumerate(self.docs):
                for w_index, word in enumerate(doc):
                    old_topic = self.topics[d_index][w_index]
                    w_id = self.word2id[word]

                    # Remove old counts
                    self.doc_topic_count[d_index, old_topic] -= 1
                    self.topic_word_count[old_topic, w_id] -= 1
                    self.topic_count[old_topic] -= 1

                    # Compute topic probabilities
                    p = (self.topic_word_count[:, w_id] / self.topic_count) * \
                        self.doc_topic_count[d_index, :]
                    p /= np.sum(p)

                    # Sample new topic
                    new_topic = np.random.choice(self.num_topics, p=p)
                    self.topics[d_index][w_index] = new_topic

                    # Add new counts
                    self.doc_topic_count[d_index, new_topic] += 1
                    self.topic_word_count[new_topic, w_id] += 1
                    self.topic_count[new_topic] += 1

    def get_topics(self, top_n=5):
        topic_words = []
        for t in range(self.num_topics):
            word_probs = self.topic_word_count[t, :] / self.topic_count[t]
            top_word_ids = word_probs.argsort()[-top_n:][::-1]
            topic_words.append([self.id2word[i] for i in top_word_ids])
        return topic_words

    def get_document_topics(self, d_index):
        doc_topic_dist = self.doc_topic_count[d_index, :] / np.sum(self.doc_topic_count[d_index, :])
        return doc_topic_dist

    def get_document_label(self, d_index):
        # Get the topic with max probability
        probs = self.get_document_topics(d_index)
        top_topic = np.argmax(probs)
        return self.topic_labels[top_topic]
