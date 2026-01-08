#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_WORD_LENGTH 50


const char *positiveWords[] = {"good", "great", "happy", "excellent", "love", "wonderful", "best", "nice", "joy","better"};
const char *negativeWords[] = {"bad", "sad", "terrible", "hate", "awful", "worst", "poor", "angry", "depressing"};

int numPos = 9;
int numNeg = 9;

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

void analyzeSentiment(char *line) {
    char tempLine[MAX_LINE_LENGTH];
    strcpy(tempLine, line); 

    int score = 0;
    char *token = strtok(tempLine, " \t\n\r");

    while (token != NULL) {
        cleanWord(token);

       
        for (int i = 0; i < numPos; i++) {
            if (strcmp(token, positiveWords[i]) == 0) {
                score++;
            }
        }

        for (int i = 0; i < numNeg; i++) {
            if (strcmp(token, negativeWords[i]) == 0) {
                score--;
            }
        }

        token = strtok(NULL, " \t\n\r");
    }

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
        if (strlen(line) > 1) {
            analyzeSentiment(line);
        }
    }

    fclose(file);
    return 0;
}