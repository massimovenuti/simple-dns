if ! test -x ./bin/serveur; then
  echo "Error: no executable \"./bin/serveur\"" 1>&2
  exit 1
fi

echo "test 1"
if echo "stop" | ./bin/serveur 4242 ./samples/test1.conf; then
  printf "\033[32mSUCCESS\033[0m\n"
   exit 0
else
  printf "\n\033[31mFAIL\033[0m\n"
  exit 1
fi

# echo "test 2"
# if (sleep 10 && echo "stop") | ./bin/serveur 4242 toto > /dev/null & sleep 1 && nc -u 127.0.0.1 4242 < /proc/cpuinfo > /dev/null & sleep 1 && nc -u ::1 4242 < /proc/cpuinfo > /dev/null & wait ; then
#   printf "\033[32mSUCCESS\033[0m\n"
#   exit 0
# else
#   printf "\n\033[31mFAIL\033[0m\n"
#   exit 1
# fi
