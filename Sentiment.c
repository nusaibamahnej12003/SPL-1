#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORD_LENGTH 50

// Simple Lexicon (Dictionaries)
const char *positiveWords[] = {"good", "great", "happy", "excellent", "love", "wonderful", "best", "nice", "joy"};
const char *negativeWords[] = {"bad", "sad", "terrible", "hate", "awful", "worst", "poor", "angry", "depressing"};

int numPos = 9;
int numNeg = 9;

// Function to clean a word (lowercase and remove punctuation)
void cleanWord(char *word) {
    int i, j = 0;
    char temp[MAX_WORD_LENGTH];
    for (i = 0; word[i]; i++) {
        if (isalpha(word[i])) {
            temp[j++] = tolower(word[i]);
        }
    }
    temp[j] = '\0';
    strcpy(word, temp);
}

// Function to analyze sentiment of a single line
void analyzeSentiment(char *line) {
    char tempLine[MAX_LINE_LENGTH];
    strcpy(tempLine, line); // Copy line to avoid modifying the original

    int score = 0;
    char *token = strtok(tempLine, " \t\n\r");

    while (token != NULL) {
        cleanWord(token);

        // Check against positive words
        for (int i = 0; i < numPos; i++) {
            if (strcmp(token, positiveWords[i]) == 0) {
                score++;
            }
        }

        // Check against negative words
        for (int i = 0; i < numNeg; i++) {
            if (strcmp(token, negativeWords[i]) == 0) {
                score--;
            }
        }

        token = strtok(NULL, " \t\n\r");
    }

    // Determine Result
    printf("Line: %s", line);
    if (score > 0) {
        printf("Sentiment: [POSITIVE] (Score: %d)\n", score);
    } else if (score < 0) {
        printf("Sentiment: [NEGATIVE] (Score: %d)\n", score);
    } else {
        printf("Sentiment: [NEUTRAL]  (Score: 0)\n");
    }
    printf("--------------------------------------------------\n");
}

int main() {
    FILE *file = fopen("input.txt", "r");

    if (file == NULL) {
        printf("Error: Could not open input.txt\n");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    printf("Analyzing sentiment from input.txt...\n\n");

    while (fgets(line, sizeof(line), file)) {
        // Only analyze lines that aren't just whitespace
        if (strlen(line) > 1) {
            analyzeSentiment(line);
        }
    }

    fclose(file);
    return 0;
}