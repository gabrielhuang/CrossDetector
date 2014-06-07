#Client side
import socket
from random import randint
from time import sleep
import os

def initclient():
    print("Client Trigger")
    hote = "localhost"
    port = 12800
    global connexion_avec_serveur
    global msg_a_envoyer
    connexion_avec_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_avec_serveur.connect((hote, port))
    print("Connexion établie avec le serveur sur le port {}".format(port))
    msg_a_envoyer = ""
    return

def senddata():
    while True:
        print("Tapez '1' pour partir")
        msg_a_envoyer = str(input())
        if (msg_a_envoyer=="1"):
            print("Egyptien mon capitaine!")
            connexion_avec_serveur.send("cible1".encode())
            break
        print("Mauvaise commande, la cible actuelle est la 0")
            
    while True:
        print("Tapez '2' pour partir")
        msg_a_envoyer = str(input())
        if (msg_a_envoyer=="2"):
            print("Tout petit vu d'ici!")
            connexion_avec_serveur.send("cible2".encode())
            break
        print("Mauvaise commande, la cible actuelle est la 1")
    return

if __name__ == "__main__":
    initclient()
    senddata()
    print("Fin de la mission, c'est du bon boulot!")
    input()
