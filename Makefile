CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g -I"C:/Program Files/OpenSSL-Win64/include"
LDFLAGS = -L"C:\Program Files\OpenSSL-Win64\lib\VC\x64\MD" -lcrypto -lssl -static-libgcc

SRC = main.c ransomware.c
OBJ = $(SRC:.c=.o)
EXEC = ransomware

# Règle par défaut
all: $(EXEC)

# Règle pour créer l'exécutable
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Règle pour compiler les fichiers .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	del $(OBJ) $(EXEC)
	
# Nettoyage des fichiers générés + exe
cleanf: clean 
	del *.exe
	 
# Nettoyage complet (incluant les fichiers temporaires)
distclean: clean
	del *~ .*~

# Test de l'exécution
run: $(EXEC)
	$(EXEC)
run-enc: $(EXEC)
	$(EXEC) ./test "0123456789abcdef0123456789abcdef" "abcdef9876543210" 0
run-dec: $(EXEC)
	$(EXEC) ./test "0123456789abcdef0123456789abcdef" "abcdef9876543210" 1
