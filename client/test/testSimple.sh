#!/bin/bash

# Fonctions pour l'affichage
function debut_test () # numero titre
{
    echo -n "[  ] $1. $2... "
}

function fin_test ()
{
    echo -e "\r[\033[32mok\033[0m]"
}

function fail () # erreur
{
    echo -e "\r[\033[31mko\033[0m]"
    echo "==> Échec du test sur '$1'."
    exit 1
}

function test_bad_arg() {
    debut_test 1 "Test du nombre d'arguments"
    $1 |& grep "Usage:" > /dev/null || fail "Nombre d'argument invalide (0)"
    $1 riri loulou fifi zaza |& grep "Usage:" > /dev/null || fail "Nombre d'argument invalide (4)"
    fin_test
}

function test_bad_file() {
    debut_test 2 "Test d'existance de conf"
    $1 inexistant &> /dev/null
    test $? -eq 1 || fail "Appel avec conf inexistant"
    $1 ./samples/bad.conf &> /dev/null
    test $? -eq 1 || fail "Appel avec une mauvaise conf"
    fin_test
}

function test_run() {
    debut_test 3 "Test d'execution"
    local FAIL=0
    coproc client ( $1 $2 )
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "execution simple"
    fin_test
}

function test_memoir() {
    debut_test 4 "Test memoir"
    local FAIL=0
    coproc client (valgrind --leak-check=full --undef-value-errors=no --error-exitcode=1 $1 $2 &> /dev/null )
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "Test memoir"
    fin_test
}

EXE=./bin/client
CONF=./samples/test1.conf
TMPDIR=tmp

mkdir $TMPDIR

test_bad_arg $EXE
test_bad_file $EXE
test_run $EXE $CONF
test_memoir  $EXE $CONF

rmdir $TMPDIR

