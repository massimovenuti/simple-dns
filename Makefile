all: client serveur

.PHONY:client
client:
	(cd client; make)

.PHONY:serveur
:
	(cd serveur; make)

.PHONY:test
test: test-client test-serveur
	echo "Test Global"
	./test/test.sh

test-client: client
	echo "Test Client"
	(cd client; make test)

test-serveur: serveur
	echo "Test Serveur"
	(cd serveur; make test)

clean:
	(cd client; make clean)
	(cd serveur; make clean)