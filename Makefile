all: client-build serveur-build

client-build:
	(cd client; make)

serveur-build:
	(cd serveur; make)

test: test-client test-serveur
	./test/test.sh

test-client:
	(cd client; make test)

test-serveur:
	(cd serveur; make test)

clean:
	(cd client; make clean)
	(cd serveur; make clean)