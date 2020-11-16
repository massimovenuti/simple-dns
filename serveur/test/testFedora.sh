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
    debut_test 2 "Test d'existence de conf"
    $1 $2 inexistant &> /dev/null
    test $? -eq 1 || fail "Appel avec conf inexistante"
    $1 $2 ./samples/bad.conf &> /dev/null
    test $? -eq 1 || fail "Appel avec une mauvaise conf"
    fin_test
}

function test_run() {
    debut_test 3 "Test d'exécution"
    local FAIL=0
    coproc serv ( $1 $2 $3 )
    sleep 1
    echo stop >&"${serv[1]}"
    wait ${serv_PID} || FAIL=1
    test $FAIL -eq 0 || fail "exécution"
    fin_test
}

function test_req() {
    debut_test 4 "Test de requête"
    
    printf "1|123,456|toto.fr\n|1|.fr,10.0.0.1,7575|.fr,127.0.0.1,6060|.fr,::1,6060\0" > $4/wait.txt

    local FAIL=0
    coproc serv ( $1 $2 $3 )
    sleep 1
    (echo "1|123,456|toto.fr"; sleep 1) | nc -u4 -p 50000 127.0.0.1 $2 &> $4/res.txt || FAIL=1
    (echo "ack|1"; sleep 10) | nc -u4 -p 50000 127.0.0.1 $2 &>> $4/res.txt || FAIL=1
    echo stop >&"${serv[1]}"
    wait ${serv_PID}

    diff $4/wait.txt $4/res.txt
    test $? -eq 0 -a $FAIL -eq 0 || fail "envoi d'une requête IPV4"

    coproc serv ( $1 $2 $3 )
    sleep 1
    (echo "1|123,456|toto.fr"; sleep 1) | nc -u6 -p 50000 ::1 $2 &> $4/res.txt || FAIL=1
    (echo "ack|1"; sleep 10) | nc -u6 -p 50000 ::1 $2 &>> $4/res.txt || FAIL=1
    echo stop >&"${serv[1]}"
    wait ${serv_PID} || FAIL=1

    diff $4/wait.txt $4/res.txt
    test $? -eq 0 -a $FAIL -eq 0 || fail "envoi d'une requête IPV6"

    coproc serv ( $1 $2 $3 &> /dev/null )
    sleep 1
    (echo "Oh, des regrets, des regrets, des regrets"; sleep 1) | nc -u6 -p 50000 ::1 $2 &> $4/res.txt || FAIL=1
    echo stop >&"${serv[1]}"
    wait ${serv_PID} || FAIL=1

    test ! -s $4/res.txt -a $FAIL -eq 0 || fail "envoi d'une requête invalide"

    printf "1|123,456|toto.fr\n|1|.fr,10.0.0.1,7575|.fr,127.0.0.1,6060|.fr,::1,6060\0" >> $4/wait.txt

    coproc serv ( $1 $2 $3 )
    sleep 1
    (echo "1|123,456|toto.fr"; sleep 10) | nc -u6 -p 50000 ::1 $2 &> $4/res.txt || FAIL=1
    echo stop >&"${serv[1]}"
    wait ${serv_PID} || FAIL=1

    diff $4/wait.txt $4/res.txt
    test $? -eq 0 -a $FAIL -eq 0 || fail "retry si pas de ack"

    rm $4/wait.txt
    rm $4/res.txt
    fin_test
}

function test_charge() {
    debut_test 5 "Test de charge"
    local FAIL=0
    coproc serv ( $1 $2 $3 || FAIL=1 )
    sleep 1
    for i in {1..1000}
    do
        echo "$i|123,456|toto.fr" | nc -u6 -p 50000 ::1 $2 &> /dev/null || FAIL=1
        echo "ack|$i" | nc -u6 -p 50000 ::1 $2 &>> /dev/null || FAIL=1
    done
    sleep 1
    echo stop >&"${serv[1]}"
    wait ${serv_PID} || FAIL=1
    test $FAIL -eq 0 || fail "test de charge"
    fin_test
}

function test_memoir() {
    debut_test 6 "Test de mémoire"
    local FAIL=0
    coproc serv ( valgrind --leak-check=full --undef-value-errors=no --error-exitcode=1 $1 $2 $3 &> /dev/null)
    sleep 1
    for i in {1..1000}
    do
        echo "$i|123,456|toto.fr" | nc -u6 -p 50000 ::1 $2 &> /dev/null || FAIL=1
        echo "ack|$i" | nc -u6 -p 50000 ::1 $2 &>> /dev/null || FAIL=1
    done
    sleep 1
    echo stop >&"${serv[1]}"
    wait ${serv_PID}  || FAIL=1
    test $FAIL -eq 0 || fail "test de mémoire"
    fin_test
}

EXE=./bin/serveur
PORT=4242
CONF=./samples/test/test1.conf
CHARGE_CONF=./samples/test/charge.conf
TMPDIR=tmp

mkdir $TMPDIR

test_bad_arg $EXE $PORT
test_bad_file $EXE $PORT
test_run $EXE $PORT $CONF
test_req $EXE $PORT $CONF $TMPDIR
test_charge $EXE $PORT $CHARGE_CONF
test_memoir $EXE $PORT $CHARGE_CONF

rmdir $TMPDIR

