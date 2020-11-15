import os, sys, time

class Serveur:
    conf = ""
    port = 0
    pipe = ()
    pid = 0
    exit_code = 0

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
    
    def run_valgrind(self):
        self.pipe = os.pipe()
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.pipe[1])
            os.dup2(self.pipe[0], sys.stdin.fileno())
            os.execl('/usr/bin/valgrind', '/usr/bin/valgrind', '--leak-check=full', '--undef-value-errors=no', '--show-leak-kinds=all', '--error-exitcode=1', './serveur/bin/serveur', str(self.port), self.conf)
        else:
            os.close(self.pipe[0])

    def stop(self):
        os.write(self.pipe[1], str.encode('stop\n'))
        status = os.waitpid(self.pid, 0)[1]
        
        if os.WIFEXITED(status):
            self.exit_code = os.WEXITSTATUS(status)
        else:
            self.exit_code = 1

        os.close(self.pipe[1])

class Client:
    conf = ""
    req_file = ""
    pipe = ()
    pid = 0
    exit_code = 0

    def __init__(self, conf, req_file = ""):
        self.conf = conf
        self.req_file = req_file
    
    def run(self):
        self.pipe = os.pipe()
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.pipe[1])
            os.dup2(self.pipe[0], sys.stdin.fileno())
            if self.req_file == "":
                os.execl('./client/bin/client', './client/bin/client', self.conf)
            else:
                os.execl('./client/bin/client', './client/bin/client', self.conf, self.req_file)
        else:
            os.close(self.pipe[0])
    
    def run_valgrind(self):       
        self.pipe = os.pipe()
        self.pid = os.fork()
        if self.pid == 0:
            os.close(self.pipe[1])
            os.dup2(self.pipe[0], sys.stdin.fileno())
            if self.req_file == "":
                os.execl('/usr/bin/valgrind', '/usr/bin/valgrind', '--leak-check=full', '--undef-value-errors=no', '--show-leak-kinds=all', '--error-exitcode=1', './client/bin/client', self.conf)
            else:
                os.execl('/usr/bin/valgrind', '/usr/bin/valgrind', '--leak-check=full', '--undef-value-errors=no', '--show-leak-kinds=all', '--error-exitcode=1', './client/bin/client', self.conf, self.req_file)
        else:
            os.close(self.pipe[0])
    
    def send(self, mes):
        os.write(self.pipe[1], str.encode((mes + '\n')))

    def stop(self):
        os.write(self.pipe[1], str.encode('!stop\n'))
        status = os.waitpid(self.pid, 0)[1]

        if os.WIFEXITED(status):
            self.exit_code = os.WEXITSTATUS(status)
        else:
            self.exit_code = 1

        os.close(self.pipe[1])

def test_requet_simple():
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

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

def test_requet_not_found():
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
    client.send("fifi.fr")
    time.sleep(5)
    client.stop()

    root.stop()
    fr.stop()
    en.stop()
    toto1.stop()
    toto2.stop()

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

def test_requet_file():
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")
    en = Serveur(6161, "./serveur/samples/en.conf")
    toto1 = Serveur(6464, "./serveur/samples/toto.conf")
    toto2 = Serveur(6565, "./serveur/samples/toto.conf")

    client = Client("./client/samples/test1.conf", "./client/samples/req.txt")

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

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

def test_round_robin():
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")
    en = Serveur(6161, "./serveur/samples/en.conf")
    toto1 = Serveur(6464, "./serveur/samples/toto.conf")
    toto2 = Serveur(6565, "./serveur/samples/toto.conf")

    client = Client("./client/samples/test1.conf", "./client/samples/test/req.txt")

    root.run()
    fr.run()
    en.run()
    toto1.run()
    toto2.run()

    client.run()
    time.sleep(5)
    client.stop()

    root.stop()
    fr.stop()
    en.stop()
    toto1.stop()
    toto2.stop()

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

def test_time_out():
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")

    client = Client("./client/samples/test1.conf")

    root.run()
    fr.run()

    client.run()
    client.send("riri.toto.fr")
    time.sleep(30)
    client.stop()

    root.stop()
    fr.stop()

    return client.exit_code + root.exit_code + fr.exit_code

def test_start_stop():
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")
    en = Serveur(6161, "./serveur/samples/en.conf")
    toto1 = Serveur(6464, "./serveur/samples/toto.conf")
    toto2 = Serveur(6565, "./serveur/samples/toto.conf")

    client = Client("./client/samples/test1.conf")

    root.run()
    en.run()
    toto1.run()
    toto2.run()

    client.run()
    client.send("riri.toto.fr")
    time.sleep(25)
    fr.run()
    time.sleep(10)
    client.send("riri.toto.fr")
    time.sleep(5)
    client.stop()

    root.stop()
    fr.stop()
    en.stop()
    toto1.stop()
    toto2.stop()

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

def test_memoir():
    root = Serveur(4242, "./serveur/samples/root.conf")
    fr = Serveur(6060, "./serveur/samples/fr.conf")
    en = Serveur(6161, "./serveur/samples/en.conf")
    toto1 = Serveur(6464, "./serveur/samples/toto.conf")
    toto2 = Serveur(6565, "./serveur/samples/toto.conf")

    client = Client("./client/samples/test1.conf", "./client/samples/req.txt")

    root.run_valgrind()
    en.run_valgrind()
    fr.run_valgrind()
    toto1.run_valgrind()
    toto2.run_valgrind()

    client.run_valgrind()
    time.sleep(10)
    fr.stop()
    client.send("riri.toto.fr")
    time.sleep(25)
    fr.run_valgrind()
    time.sleep(10)
    client.send("riri.toto.fr")
    time.sleep(5)
    client.stop()

    root.stop()
    fr.stop()
    en.stop()
    toto1.stop()
    toto2.stop()

    return client.exit_code + root.exit_code + fr.exit_code + en.exit_code + toto1.exit_code + toto2.exit_code

if __name__ == "__main__":
    if sys.argv[1] == '1':
        exit(test_requet_simple())
    
    if sys.argv[1] == '2':
        exit(test_requet_not_found())
    
    if sys.argv[1] == '3':
        exit(test_requet_file())

    if sys.argv[1] == '4':
        exit(test_round_robin())
    
    if sys.argv[1] == '5':
        exit(test_time_out())

    if sys.argv[1] == '6':
        exit(test_start_stop())
    
    if sys.argv[1] == '7':
        exit(test_memoir())