if ! test -x ./bin/client; then
  echo "Error: no executable \"./bin/client\"" 1>&2
  exit 1
fi

if echo "unistra.fr" | ./bin/client ./samples/test1.conf; then
  printf "\033[32mSUCCESS\033[0m\n"
  exit 0
else
  printf "\n\033[31mFAIL\033[0m\n"
  exit 1
fi