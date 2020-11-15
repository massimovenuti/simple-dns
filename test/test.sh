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

function test_resolve() {
    debut_test 1 "Test de resolution"
    python3 $1 1 | grep "riri.toto.fr ::1:8080" > /dev/null || fail "Demande de resolution simple"
    python3 $1 2 | grep "fifi.fr Not found" > /dev/null || fail "Demande de resolution simple"
    python3 $1 3 | grep "riri.toto.fr ::1:8080" > /dev/null || fail "Demande de resolution avec fichier"
    fin_test 
}

function test_roud_robin() {
    debut_test 2 "Test du round_rebon"
    python3 $1 4 |& grep "127.0.0.1:6464\|::1:6565" > /dev/null || fail "round_rebon"
    fin_test 
}

function test_timeout() {
    debut_test 3 "Test du timeout"
    python3 $1 5 |& grep "riri.toto.fr Timeout" > /dev/null || fail "timeout"
    fin_test 
}

function test_stop_start() {
    debut_test 4 "Test de repise apret ansident"
    python3 $1 6 |& grep "riri.toto.fr Timeout\|riri.toto.fr ::1:8080" > /dev/null || fail "repise apret ansident"
    fin_test 
}

function test_memoir() {
    debut_test 5 "Test memoir"
    python3 $1 7 &> /dev/null || fail "Test memoir"
    fin_test 
}

PYEXEC=./test/launchServeur.py

test_resolve $PYEXEC
test_roud_robin $PYEXEC
test_timeout $PYEXEC
test_stop_start $PYEXEC
test_memoir $PYEXEC
