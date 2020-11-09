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
    $1 $2 riri loulou |& grep "Usage:" > /dev/null || fail "Nombre d'argument invalide (3)"
    fin_test
}

function test_bad_file() {
    debut_test 2 "Test d'existance de conf"
    $1 $2 inexistant &> /dev/null
    test $? -eq 1 || fail "Appel avec conf inexistant"
    $1 $2 ./samples/bad.conf &> /dev/null
    test $? -eq 1 || fail "Appel avec une mauvaise conf"
    fin_test
}

function test_req() {
    debut_test 3 "Test de requet"
    
    printf "1|123,456|toto.fr\n|1|.fr,127.0.0.1,6060|.fr,::1,6060" > $4/wait.txt

    (sleep 5 && echo "stop") | $1 $2 $3 &
    sleep 1 && (echo "1|123,456|toto.fr"; sleep 1) | nc -u4 127.0.0.1 $2 > $4/res.txt &
    wait

    diff $4/wait.txt $4/res.txt || fail "envoi d'un requet IPV4"

    (sleep 5 && echo "stop") | $1 $2 $3 &
    sleep 1 && (echo "1|123,456|toto.fr"; sleep 1) | nc -u6 ::1 $2 > $4/res.txt &
    wait

    diff $4/wait.txt $4/res.txt || fail "envoi d'un requet IPV6"
    rm $4/wait.txt
    rm $4/res.txt
    fin_test
}

function test_charge() {
    debut_test 3 "Test de charge"
    (sleep 10 && echo "stop") | $1 $2 $3 &
    sleep 1
    for i in {1..1000}
    do
        echo "1|123,456|toto.fr" | nc -u ::1 $2 > /dev/null
    done
    test $? || fail "testde charge"
    fin_test
}

EXE=./bin/serveur
PORT=4242
CONF=./samples/test1.conf
TMPDIR=tmp

mkdir $TMPDIR

test_bad_arg $EXE $PORT
test_bad_file $EXE $PORT
test_req $EXE $PORT $CONF $TMPDIR
test_charge $EXE $PORT $CONF

rmdir $TMPDIR

