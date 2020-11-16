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
    $1 |& grep "usage:" > /dev/null || fail "Nombre d'argument invalide (0)"
    $1 riri loulou fifi zaza |& grep "usage:" > /dev/null || fail "Nombre d'argument invalide (4)"
    fin_test
}

function test_bad_file() {
    debut_test 2 "Test d'existence de conf"
    $1 inexistant &> /dev/null
    test $? -eq 1 || fail "Appel avec conf inexistante"
    $1 ./samples/bad.conf &> /dev/null
    test $? -eq 1 || fail "Appel avec une mauvaise conf"
    fin_test
}

function test_run() {
    debut_test 3 "Test d'exécution"
    local FAIL=0
    coproc client ( $1 $2 )
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "exécution simple"
    fin_test
}

function test_comande() {
    debut_test 4 "Test des comande"
    local FAIL=0
    coproc client ( $1 $2 &> /dev/null )
    echo !monitoring >&"${client[1]}"
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test !monitoring"

    local FAIL=0
    coproc client ( $1 $2 &> /dev/null )
    echo !reset >&"${client[1]}"
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test !reset"

    local FAIL=0
    coproc client ( $1 $2 &> /dev/null )
    echo !status >&"${client[1]}"
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test !status"

    local FAIL=0
    coproc client ( $1 $2 &> /dev/null )
    echo !loadconf >&"${client[1]}"
    sleep 1
    echo "./samples/test2.conf" >&"${client[1]}"
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test !loadconf"

    local FAIL=0
    coproc client ( $1 $2 &> /dev/null )
    echo !loadreq >&"${client[1]}"
    sleep 1
    echo "./samples/test/comande.txt" >&"${client[1]}"
    sleep 1
    echo !stop >&"${client[1]}"
    wait ${client_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test !loadreq"

    fin_test
}

EXE=./bin/client
CONF=./samples/test1.conf
TMPDIR=tmp

mkdir $TMPDIR

test_bad_arg $EXE
test_bad_file $EXE
test_run $EXE $CONF
test_comande $EXE $CONF

rmdir $TMPDIR

