#!/bin/bash

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
MINISHELL="./minishell"
VALGRIND="valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --trace-children=yes --suppressions=readline.supp"
TEST_DIR="test_files"

# Créer le répertoire de test
mkdir -p $TEST_DIR

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   MINISHELL TESTER - MEMORY & FD      ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════╝${NC}\n"

# Vérifier que minishell existe
if [ ! -f "$MINISHELL" ]; then
    echo -e "${RED}Erreur: $MINISHELL n'existe pas${NC}"
    echo "Compilez d'abord votre minishell avec 'make'"
    exit 1
fi

# Créer le fichier de suppression pour readline
cat > readline.supp << EOF
{
   readline_leak
   Memcheck:Leak
   ...
   fun:readline
}
{
   add_history_leak
   Memcheck:Leak
   ...
   fun:add_history
}
EOF

# Fonction pour exécuter un test
run_test() {
    local test_name="$1"
    local commands="$2"

    echo -e "${YELLOW}Test: $test_name${NC}"

    # Créer un fichier temporaire avec les commandes
    echo -e "$commands" > /tmp/minishell_test_input.txt

    # Exécuter avec valgrind
    $VALGRIND --log-file=/tmp/valgrind_output.txt $MINISHELL < /tmp/minishell_test_input.txt > /dev/null 2>&1

    # Analyser les résultats
    local leaks=$(grep "definitely lost" /tmp/valgrind_output.txt | awk '{print $4}')
    local fds=$(grep "FILE DESCRIPTORS" /tmp/valgrind_output.txt -A 1 | tail -1)

    if [ "$leaks" = "0" ]; then
        echo -e "${GREEN}✓ Pas de fuites mémoire${NC}"
    else
        echo -e "${RED}✗ Fuites mémoire détectées: $leaks bytes${NC}"
        grep "definitely lost\|indirectly lost" /tmp/valgrind_output.txt
    fi

    echo -e "${BLUE}FD: $fds${NC}"
    echo ""
}

# Créer des fichiers de test
echo "contenu test 1" > $TEST_DIR/file1.txt
echo "contenu test 2" > $TEST_DIR/file2.txt
echo "ligne 1
ligne 2
ligne 3" > $TEST_DIR/multiline.txt

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}  TESTS DE BASE${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}\n"

# Test 1: Commandes simples
run_test "Commandes simples" "echo hello
ls
pwd
exit"

# Test 2: Pipes
run_test "Pipes simples" "ls | grep test
echo hello | cat | cat
cat $TEST_DIR/file1.txt | wc -l
exit"

# Test 3: Redirections
run_test "Redirections" "echo test > $TEST_DIR/out1.txt
cat < $TEST_DIR/file1.txt
cat < $TEST_DIR/file1.txt > $TEST_DIR/out2.txt
echo append >> $TEST_DIR/out1.txt
exit"

# Test 4: Here-doc
run_test "Here-doc" "cat << EOF
ligne 1
ligne 2
EOF
exit"

# Test 5: Multiple pipes
run_test "Multiple pipes" "cat $TEST_DIR/multiline.txt | grep ligne | wc -l
ls -l | grep test | cat | wc -w
exit"

# Test 6: Variables d'environnement
run_test "Variables ENV" "echo \$PATH
echo \$HOME
export TEST=hello
echo \$TEST
exit"

# Test 7: Commandes builtin
run_test "Builtins" "cd /tmp
pwd
cd -
env | grep PATH
exit"

# Test 8: Quotes
run_test "Quotes" "echo 'hello world'
echo \"hello world\"
echo 'test \$PATH'
echo \"test \$PATH\"
exit"

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}  TESTS DE FUITES INTENSIFS${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}\n"

# Test 9: Boucle de commandes
run_test "100 commandes simples" "$(for i in {1..100}; do echo "echo test$i"; done)
exit"

# Test 10: Pipes multiples répétés
run_test "Pipes répétés" "$(for i in {1..50}; do echo "echo test | cat | cat"; done)
exit"

# Test 11: Redirections multiples
run_test "Redirections répétées" "$(for i in {1..50}; do echo "echo test$i > $TEST_DIR/out_$i.txt"; done)
exit"

# Test 12: Mix de tout
run_test "Mix complexe" "echo start
ls | grep test
cat < $TEST_DIR/file1.txt | wc -l > $TEST_DIR/result.txt
cat << EOF | grep ligne
ligne test
autre ligne
EOF
export VAR=value
echo \$VAR
pwd
exit"

echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${BLUE}  TEST FD DÉTAILLÉ${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}\n"

# Test spécifique pour les FD
echo -e "${YELLOW}Analyse des file descriptors...${NC}"
echo "ls
exit" > /tmp/fd_test.txt

$VALGRIND --track-fds=yes --log-file=/tmp/fd_detailed.txt $MINISHELL < /tmp/fd_test.txt > /dev/null 2>&1

echo -e "${BLUE}File descriptors ouverts:${NC}"
grep "Open file descriptor" /tmp/fd_detailed.txt | head -20

echo ""
echo -e "${BLUE}═══════════════════════════════════════${NC}"
echo -e "${GREEN}  TESTS TERMINÉS${NC}"
echo -e "${BLUE}═══════════════════════════════════════${NC}\n"

# Résumé
echo -e "${YELLOW}Résumé des fuites:${NC}"
total_leaks=0
for log in /tmp/valgrind_*.txt; do
    if [ -f "$log" ]; then
        leaks=$(grep "definitely lost:" "$log" | awk '{sum+=$4} END {print sum}')
        total_leaks=$((total_leaks + leaks))
    fi
done

if [ $total_leaks -eq 0 ]; then
    echo -e "${GREEN}✓ Aucune fuite mémoire détectée !${NC}"
else
    echo -e "${RED}✗ Total de fuites: $total_leaks bytes${NC}"
fi

echo -e "\n${YELLOW}Logs détaillés disponibles dans:${NC}"
echo "- /tmp/valgrind_output.txt (dernier test)"
echo "- /tmp/fd_detailed.txt (analyse FD)"

# Nettoyage
echo -e "\n${YELLOW}Nettoyage...${NC}"
rm -f /tmp/minishell_test_input.txt /tmp/fd_test.txt
rm -rf $TEST_DIR

echo -e "${GREEN}Done!${NC}"
