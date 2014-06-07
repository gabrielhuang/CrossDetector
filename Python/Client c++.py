#Client side
import socket
from random import randint
from time import sleep

def initclient():
    print("Client c++")
    print("Initialisation...")
    hote = "localhost"
    port = 12800
    global connexion_avec_serveur
    global msg_a_envoyer
    connexion_avec_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_avec_serveur.connect((hote, port))
    print("Connexion établie avec le serveur sur le port {}".format(port))
    msg_a_envoyer = ""
    print("Done")
    return

def senddata():
    while True:
        if randint(1,10)==1:
            msg_a_envoyer = "no target"
        else:
            msg_a_envoyer = str(float(randint(-1000,1000))/1000)+","+str(float(randint(-1000,1000))/1000)
        print("Données envoyées: " + msg_a_envoyer)
        connexion_avec_serveur.send(msg_a_envoyer.encode())
        sleep(0.5)
    return

if __name__ == "__main__":
    initclient()
    print("Début de l'envoi des données...")
    senddata()
