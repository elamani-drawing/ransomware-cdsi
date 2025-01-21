# Ransomware (Projet éducatif)
Ce projet est une implémentation éducative d'un ransomware en C, utilisant l'algorithme de chiffrement **AES-256 en mode CBC**. Ce projet est uniquement destiné à des fins éducatives et **ne doit pas être utilisé à des fins malveillantes**.

---

# Fonctionnalités
- **Chiffrement de fichiers** : Chiffre un fichier ou un répertoire entier avec AES-256.
- **Déchiffrement de fichiers** : Déchiffre un fichier ou un répertoire entier avec la clé et l'IV corrects.
- **Gestion des répertoires** : Parcourt récursivement les répertoires pour chiffrer ou déchiffrer tous les fichiers.

---

# Avertissements de sécurité

⚠️ **Ne lancez pas le fichier `.exe` ni n'exécutez pas le projet en dehors d'une machine virtuelle (VM).**  
Ce projet est conçu pour être exécuté dans un environnement contrôlé, comme une machine virtuelle, pour éviter tout impact accidentel sur votre système ou vos fichiers.

⚠️ **Ce projet est uniquement destiné à des fins éducatives.**  
Il ne doit pas être utilisé pour chiffrer des fichiers sans autorisation ou à des fins malveillantes.

---

# Prérequis
Ce projet fonctionne uniquement sur Windows et nécessite les outils suivants :

- **MinGW-w64 (64 bits)** :
  - Téléchargez et installez MinGW-w64 depuis  depuis [https://github.com/niXman/mingw-builds-binaries/releases](https://github.com/niXman/mingw-builds-binaries/releases).
  - Assurez-vous d'installer une version **64 bits** par exemple `x86_64-14.2.0-release-mcf-seh-ucrt-rt_v12-rev1.7z`.

- **OpenSSL (64 bits)** :
  - Téléchargez et installez OpenSSL pour Windows depuis [https://slproweb.com/products/Win32OpenSSL.html](https://slproweb.com/products/Win32OpenSSL.html).
  - Choisissez une version **64 bits** par exemple `Win64 OpenSSL v3.4.0`.

- ⚠️ Vous pouvez choisir de téléchargez des versions 32 bits, mais OpenSSL et MinGW-w64 doivent être sur la même architecture.

- **Fichiers libcrypto.def et libssl.def** :
  - Ces fichiers sont nécessaires pour compiler les bibliothèques OpenSSL.
  - Vous pouvez les générer à partir des fichiers `.dll` d'OpenSSL en utilisant l'outil `gendef` (inclus avec MinGW-w64). 

---

# Compilation

### Générer les fichiers `.def` :
1. Ouvrez un terminal et naviguez jusqu'au répertoire contenant les fichiers `libcrypto.dll` et `libssl.dll`. (Elles doivent se trouver dans `..\OpenSSL-Win64\lib\VC\x64\MD`)
2. Exécutez les commandes suivantes pour générer les fichiers `.def` :

```bash
gendef libcrypto.dll
gendef libssl.dll
```
puis
```bash
dlltool -d libcrypto.def -l libcrypto.a
dlltool -d libssl.def -l libssl.a
```
## Compiler le projet

Ouvrez un terminal et naviguez jusqu'au répertoire du projet.  
Compilez le projet avec la commande suivante :

```bash
make
```
# Limitations  

- **Windows uniquement** : Ce projet est conçu pour fonctionner uniquement sur Windows.  
- **OpenSSL 64 bits** : Le projet nécessite OpenSSL en 64 bits.  
- **MinGW-w64** : Le projet doit être compilé avec MinGW-w64 en 64 bits.  

---

# Avertissement légal  

Ce projet est fourni à **des fins éducatives uniquement**.  
L'auteur décline toute responsabilité pour toute utilisation malveillante ou illégale de ce code.  

**Utilisez ce projet à vos propres risques et dans un environnement contrôlé.**
