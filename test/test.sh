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
    fin_test
}

PYEXEC=./test/launchServeur.py
TMPDIR=tmp

mkdir $TMPDIR

test_resolve $PYEXEC $TMPDIR

rmdir $TMPDIR

