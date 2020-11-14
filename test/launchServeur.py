import os, sys, time

class Serveur:
    conf = ""
    port = 0
    pipe = ()
    pid = 0
    status = 0

    def __init__(self, port, conf):
        self.conf = conf
        self.port = port
    
    def run(self):
        self.pipe = os.pipe()
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.pipe[1])
            os.dup2(self.pipe[0], sys.stdin.fileno())
            os.execl('./serveur/bin/serveur', './serveur/bin/serveur', str(self.port), self.conf)
        else:
            os.close(self.pipe[0])
    
    def stop(self):
        os.write(self.pipe[1], str.encode('stop\n'))
        self.status = os.waitpid(self.pid, 0)[1]
        os.close(self.pipe[1])

class Client:
    conf = ""
    pipe = ()
    pid = 0
    status = 0

    def __init__(self, conf):
        self.conf = conf
    
    def run(self):
        self.pipe = os.pipe()
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.pipe[1])
            os.dup2(self.pipe[0], sys.stdin.fileno())
            os.execl('./client/bin/client', './client/bin/client', self.conf)
        else:
            os.close(self.pipe[0])
    
    def send(self, mes):
        os.write(self.pipe[1], str.encode((mes + '\n')))

    def stop(self):
        os.write(self.pipe[1], str.encode('!stop\n'))
        self.status = os.waitpid(self.pid, 0)[1]
        os.close(self.pipe[1])

if __name__ == "__main__":
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")
    en = Serveur(6161, "./serveur/samples/en.conf")
    toto1 = Serveur(6464, "./serveur/samples/toto.conf")
    toto2 = Serveur(6565, "./serveur/samples/toto.conf")

    client = Client("./client/samples/test1.conf")

    root.run()
    fr.run()
    en.run()
    toto1.run()
    toto2.run()

    client.run()
    client.send("riri.toto.fr")
    time.sleep(5)
    client.stop()

    root.stop()
    fr.stop()
    en.stop()
    toto1.stop()
    toto2.stop()