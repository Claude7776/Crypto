#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 2048

// Fréquences approximatives du français
double freq_fr[26] = {
    7.64, 0.90, 3.26, 3.67, 14.72, 1.06, 0.87,
    0.74, 7.53, 0.61, 0.05, 5.46, 2.97, 7.10,
    5.38, 3.02, 1.36, 6.55, 7.95, 7.24, 6.31,
    1.83, 0.04, 0.42, 0.19, 0.21
};

void decrypt(char *cipher, int shift, char *output) {
    int len = strlen(cipher);

    for (int i = 0; i < len; i++) {
        char c = cipher[i];

        if (isupper(c)) {
            output[i] = ((c - 'A' - shift + 26) % 26) + 'A';
        }
        else if (islower(c)) {
            output[i] = ((c - 'a' - shift + 26) % 26) + 'a';
        }
        else {
            output[i] = c; // conserve espaces et ponctuation
        }
    }
    output[len] = '\0';
}

double score_text(char *text) {
    int count[26] = {0};
    int total = 0;

    for (int i = 0; text[i]; i++) {
        if (isalpha(text[i])) {
            count[toupper(text[i]) - 'A']++;
            total++;
        }
    }

    if (total == 0) return 1e9;

    double score = 0.0;
    for (int i = 0; i < 26; i++) {
        double observed = (double)count[i] / total * 100;
        score += pow(observed - freq_fr[i], 2);
    }

    return score;
}

int main(int argc, char *argv[]) {

    char cipher[MAX];
    char decrypted[MAX];
    char best[MAX];
    double best_score = 1e9;
    int best_shift = 0;

    // 🔹 Lecture depuis fichier si argument fourni
    if (argc == 2) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            printf("Erreur ouverture fichier.\n");
            return 1;
        }
        fread(cipher, 1, MAX, f);
        fclose(f);
    }
    else {
        printf("Entrez le texte chiffré :\n");
        fgets(cipher, MAX, stdin);
    }

    for (int shift = 0; shift < 26; shift++) {
        decrypt(cipher, shift, decrypted);
        double s = score_text(decrypted);

        printf("Décalage %2d : %s", shift, decrypted);

        if (s < best_score) {
            best_score = s;
            best_shift = shift;
            strcpy(best, decrypted);
        }
    }

    printf("\n🔎 Décalage probable : %d\n", best_shift);
    printf("✅ Texte probable   : %s\n", best);

    return 0;
}
