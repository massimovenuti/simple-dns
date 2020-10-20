if ! test -x ./bin/serveur; then
  echo "Error: no executable \"./bin/serveur\"" 1>&2
  exit 1
fi

if echo "stop" | ./bin/serveur 4242 test.conf; then
  printf "\033[32mSUCCESS\033[0m\n"
  exit 0
else
  printf "\n\033[31mFAIL\033[0m\n"
  exit 1
fi