all: client-build serveur-build

client-build:
	(cd client; make)

serveur-build:
	(cd serveur; make)
	
clean:
	(cd client; make clean)
	(cd serveur; make clean)