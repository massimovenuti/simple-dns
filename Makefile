all: client-build serveur-build

client-build:
	(cd client; make)

serveur-build:
	(cd serveur; make)

test: test-client test-serveur
	echo "Test Global"
	./test/test.sh

test-client: client-build
	echo "Test Client"
	(cd client; make test)

test-serveur: serveur-build
	echo "Test Serveur"
	(cd serveur; make test)

clean:
	(cd client; make clean)
	(cd serveur; make clean)