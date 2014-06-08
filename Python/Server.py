# -*- coding: utf8 -*-

#Server side
import socket
from math import *
from time import sleep
import select
import sys

global camval
camval = ""

def inittargetWP():
    global TargetGPSWP
    TargetGPSWP = input("Entrez les coordonnées de la 1ère cible:\n") + "," +\
    input("Entrez les coordonnées de la 2ème cible:\n")
    print(TargetGPSWP)


def initserv():
    global connexion_principale
    global msg_recu
    global hote
    global port
    global inputs
    hote = ''
    port = 12800
    connexion_principale = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_principale.bind((hote, port))
    connexion_principale.listen(5)
    msg_recu = ""
    inputs = []
    
def tempserv():
    global connexion_avec_client_camera
    global connexion_avec_client_script
    global connexion_avec_client_trigger
    while True:
        inputs.append(connexion_principale)
        notconnected=True
        print("Le serveur écoute à présent sur le port {}".format(port))
        try:
            while notconnected:
                print("En attente du client c++")
                inputready,outputready,exceptready = select.select(inputs,[],[])
                for s in inputready:
                    if s==connexion_principale:
                        connexion_avec_client_camera, infos_connexion = connexion_principale.accept()
                        inputs.append(connexion_avec_client_camera)
                        print("Connexion acceptee de {}".format(infos_connexion))
                        notconnected = False
                    else:
                        msg_recu = msg_recu = s.recv(100)
                        if msg_recu not in ["Data pls", "cible1", "cible2"]:
                            camval = msg_recu
            notconnected = True

            while notconnected:
                print("En attente du script MAVLink")
                inputready,outputready,exceptready = select.select(inputs,[],[])
                for s in inputready:
                    if s==connexion_principale:
                        connexion_avec_client_script, infos_connexion = connexion_principale.accept()
                        inputs.append(connexion_avec_client_script)
                        print("Connexion acceptee de {}".format(infos_connexion))
                        notconnected = False
                        connexion_avec_client_script.send(TargetGPSWP.encode())
                    else:
                        msg_recu = msg_recu = s.recv(100)
                        if msg_recu not in ["Data pls", "cible1", "cible2"]:
                            camval = msg_recu
            notconnected = True
            
            while notconnected:
                print("En attente du trigger")
                inputready,outputready,exceptready = select.select(inputs,[],[])
                for s in inputready:
                    if s==connexion_principale:
                        connexion_avec_client_trigger, infos_connexion = connexion_principale.accept()
                        inputs.append(connexion_avec_client_trigger)
                        print("Connexion acceptee de {}".format(infos_connexion))
                        notconnected = False
                    else:
                        msg_recu = msg_recu = s.recv(100)
                        if msg_recu not in ["Data pls", "cible1", "cible2"]:
                            camval = msg_recu
            break
        except:
            pass
        print("Erreur de connexion, veuillez recommencer :D")
        connexion_avec_client_camera.close()
        connexion_avec_client_script.close()
        connexion_avec_client_trigger.close()

def checkdata():
    while True:
        print("En attente de données...")
        inputready,outputready,exceptready = select.select(inputs,[],[])
        for s in inputready:
            msg_recu = s.recv(100)
            msg_recu = msg_recu.decode()
            print("Vous avez 1 nouveau message: " + msg_recu)
            if msg_recu:
                if msg_recu not in ["Data pls", "cible1", "cible2"]:
                    camval = msg_recu
                elif msg_recu== "Data pls":
                    connexion_avec_client_script.send(camval.encode())
                    print("Données envoyées: "+camval)
                elif (msg_recu=="cible1" or msg_recu=="cible2"):
                    connexion_avec_client_script.send(msg_recu.encode())
                    print("Message Transmis: "+ msg_recu)
            else:
                s.close()
                print("Mayday Mayday Mayday, on a perdu un client!")                
                inputs.remove(s)
                return "Erreur"
        
if __name__ == "__main__":
    inittargetWP()
    initserv()
    tempserv()
    checkdata()
