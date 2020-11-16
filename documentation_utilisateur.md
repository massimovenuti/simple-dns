---
title: "Documentation Utilisateur "
author: [VOGEL Alexandre, VENUTI Massimo]
date: "2020"
lang: "fr"
titlepage: true
...

# Build

`make` compile le client et le serveur

`make client` compile le client seulement 

`make serveur` compiler le serveur 

# Serveur

Lancer le serveur avec `./serveur/bin/serveur <port> <fichier de configuration>`.

Les entrées du fichier de configuration doivent être de la forme suivante:
```
<domaine>|<IP1>|<port1>
<domaine>|<IP2>|<port2>
...
```

Pour arrêter le serveur, écrire `stop` sur l'entrée standard.

# Client

Lancer le client avec `./client/bin/client <fichier de configuration> <fichier de commande>`.

Le fichier de configuration doit avoir ses entrées sous la forme suivante:
```
<IP1>|<port1>
<IP2>|<port2>
…
<IPn>|<portn>
```
Le fichier de commandes est optionnel. S' il est présent, il sera lu et traité par le client. Ce dernier se mettra ensuite en mode interactif.

En mode interactif, toute instruction non précédée par un ! passée sur l’entrée standard sera considérée comme un nom à résoudre.  Si elle est précédée par un !, elle sera traitée comme une commande. 

## Liste des commandes:

`!stop` -> Arrête le serveur\
`!monitoring` -> Active ou désactive le mode monitoring\ 
`!ignored` -> Affiche les serveurs qui sont actuellement ignorés\
`!reset` -> Vide la liste des serveurs ignorés \
`!status` -> Affiche le statut courant du client (les serveurs ignorés, les serveurs qui n’ont pas répondus une première fois, les informations concernant les serveurs utilisés si le mode monitoring a été activé) \
`!loadconf` -> Charge une nouvelle configuration et écrase celle courante\
`!loadreq` -> Charge un fichier de commandes

# Test

`make test` lance les tests client, serveur et global

`make test-client` lance les tests client

`make test-serveur` lance les tests serveur

`make test-client` lance les tests client

# Scénarios 

le script `./test/launchServeur.py`  permet de lancer plusieurs scénarios client-serveur à l'aide de la commande `python3 ./test/launchServeur.py <numéro du scénario>` .

## liste des différents scénarios:
**1** -> Requête simple, le client tente de résoudre le nom `riri.toto.fr` et cela fonctione\
**2** -> Requête introuvable, le client tente de résoudre le nom `fifi.fr` et cela échoue\
**3** -> Requêtes à partir du fichier `./client/samples/req.txt`, et demande de résolution  de `riri.toto.fr` depuis le mode interactif_
**4** -> Réalise 2 requêtes identiques successives à partir du fichier `./client/samples/test/req.txt`\
**5** -> Tente de résoudre le nom `riri.toto.fr` mais tout les serveurs `toto` sont inaccessibles.\
**6** -> Tente de résoudre le nom `riri.toto.fr` mais tous les serveurs `fr` sont inaccessibles, puis relance les serveurs `fr` et essaie à nouveau la résolution.\ 
**7** -> Enchaîne les scénarios  **3** et **6**
