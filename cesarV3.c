#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 4096
#define MAX_KEY 12

// Fréquences FR
double freq_fr[26] = {
    7.64,0.90,3.26,3.67,14.72,1.06,0.87,0.74,7.53,0.61,0.05,5.46,
    2.97,7.10,5.38,3.02,1.36,6.55,7.95,7.24,6.31,1.83,0.04,0.42,0.19,0.21
};

// Fréquences EN
double freq_en[26] = {
    8.17,1.49,2.78,4.25,12.70,2.23,2.02,6.09,6.97,0.15,0.77,4.03,
    2.41,6.75,7.51,1.93,0.10,5.99,6.33,9.06,2.76,0.98,2.36,0.15,1.97,0.07
};

void decrypt_caesar(char *cipher, int shift, char *output) {
    int len = strlen(cipher);
    for (int i = 0; i < len; i++) {
        char c = cipher[i];
        if (isupper(c))
            output[i] = ((c - 'A' - shift + 26) % 26) + 'A';
        else if (islower(c))
            output[i] = ((c - 'a' - shift + 26) % 26) + 'a';
        else
            output[i] = c;
    }
    output[len] = '\0';
}

double score_text(char *text, double freq[]) {
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
        score += pow(observed - freq[i], 2);
    }

    return score;
}

/* -------- VIGENERE -------- */

void decrypt_vigenere(char *cipher, char *key, char *output) {
    int len = strlen(cipher);
    int key_len = strlen(key);
    int j = 0;

    for (int i = 0; i < len; i++) {
        if (isalpha(cipher[i])) {
            char base = isupper(cipher[i]) ? 'A' : 'a';
            int shift = toupper(key[j % key_len]) - 'A';
            output[i] = ((cipher[i] - base - shift + 26) % 26) + base;
            j++;
        } else {
            output[i] = cipher[i];
        }
    }
    output[len] = '\0';
}

// attaque simplifiée : suppose longueur clé 1 à MAX_KEY
void brute_vigenere(char *cipher) {
   // char segment[MAX]; -------> variable non utilisé donc commenter
    char decrypted[MAX];

    printf("\n===== Tentative Vigenere =====\n");

    for (int key_len = 2; key_len <= MAX_KEY; key_len++) {

        char key[MAX_KEY+1];

        for (int i = 0; i < key_len; i++) {
            char sub[MAX];
            int index = 0;

            for (int j = i; cipher[j]; j += key_len) {
                if (isalpha(cipher[j]))
                    sub[index++] = cipher[j];
            }
            sub[index] = '\0';

            double best_score = 1e9;
            int best_shift = 0;
            char temp[MAX];

            for (int shift = 0; shift < 26; shift++) {
                decrypt_caesar(sub, shift, temp);
                double s = score_text(temp, freq_en);

                if (s < best_score) {
                    best_score = s;
                    best_shift = shift;
                }
            }

            key[i] = 'A' + best_shift;
        }

        key[key_len] = '\0';

        decrypt_vigenere(cipher, key, decrypted);
        printf("Clé probable (%d) : %s\n", key_len, key);
        printf("Texte : %s\n\n", decrypted);
    }
}

/* -------- MAIN -------- */

int main(int argc, char *argv[]) {

    char cipher[MAX] = {0};
    char decrypted[MAX];

    /* ===== Lecture ===== */

    if (argc == 2) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            perror("Erreur ouverture fichier");
            return 1;
        }

        size_t bytes_read = fread(cipher, 1, MAX - 1, f);
        cipher[bytes_read] = '\0';
        fclose(f);

        printf("Fichier chargé (%zu bytes)\n", bytes_read);

    } else {
        printf("Entrez texte chiffré :\n");
        fgets(cipher, MAX, stdin);
    }

    /* ===== Suppression newline final ===== */
    cipher[strcspn(cipher, "\n")] = 0;

    /* ===== Analyse Caesar ===== */

    double best_score = 1e9;
    int best_shift = 0;
    char best_lang[3] = "FR";

    for (int shift = 0; shift < 26; shift++) {

        decrypt_caesar(cipher, shift, decrypted);

        double scoreFR = score_text(decrypted, freq_fr);
        double scoreEN = score_text(decrypted, freq_en);

        double minScore = scoreFR;
        char lang[3] = "FR";

        if (scoreEN < scoreFR) {
            minScore = scoreEN;
            strcpy(lang, "EN");
        }

        if (minScore < best_score) {
            best_score = minScore;
            best_shift = shift;
            strcpy(best_lang, lang);
        }
    }

    decrypt_caesar(cipher, best_shift, decrypted);

    printf("\n===== RESULTAT CAESAR =====\n");
    printf("Langue probable : %s\n", best_lang);
    printf("Décalage : %d\n", best_shift);
    printf("Texte : %s\n", decrypted);

    if (best_score > 200) {
        printf("\n⚠️ Probablement pas un Caesar simple.\n");
        brute_vigenere(cipher);
    }

    return 0;
}
