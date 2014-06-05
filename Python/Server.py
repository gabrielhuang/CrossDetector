# -*- coding: utf8 -*-

#Server side
import socket
from math import *
from time import sleep

def initserv():
    global connexion_principale
    global connexion_avec_client
    global msg_recu
    global hote
    global port
    hote = ''
    port = 12800
    connexion_principale = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_principale.bind((hote, port))
    connexion_principale.listen(5)
    msg_recu = ""

def updatetarget():
    connexion_avec_client.send("update".encode())
    msg_recu = connexion_avec_client.recv(100)
    msg_recu = msg_recu.decode()
    print(msg_recu)
    x = msg_recu.split(",")
    return float(x[0]), float(x[1])

def getwaypoint(anglelacet, angleroulis, angletangage, altitude, currentlat,currentlong):
    deltapixx, deltapixy = updatetarget()
    deltapixx = deltapixx*1 #correction fisheye
    deltapixy = deltapixy*1 #correction fisheye
    """
    la latitude augmente vers le nord
    la longitude augmente vers l'est (Greenwich = 0)
    les angles sont nuls à l'horizontale, pointant vers le nord
    les angles en degres
    l'altitude en mé
    la position GPS en degres
    angleroulis est positifési le drone va vers la droite
    angletangage est positif si le drone va vers l'avant
    les deltapix sont nuls si le barycentre est au milieu de l'image,
    positifs si il est en haut à droite
    
    """
    currentlat = currentlat*111205.12 #conversion en m
    currentlong = currentlong*73517.0 #conversion en m
    lref = 1 #longueur de la regle utilisee pour le calibrage en m
    href = 1 #hauteur de la camera pour lé calibrage
    deltapixrefx = 200 #taille ée la regle pour le calibrage, apres correction fisheye
    deltapixrefy = 200 #taille de la règle pour le calibrage, après correction fisheye
    wplat = currentlat -\
            (tan(angletangage*pi/180)*altitude -
            deltapixy*altitude*lref/(cos(angletangage*pi/180)*href*deltapixrefx))\
            *cos(anglelacet*pi/180) +\
            (tan(angleroulis*pi/180)*altitude +
            deltapixx*altitude*lref/(cos(angleroulis*pi/180)*href*deltapixrefy))\
            *sin(anglelacet*pi/180)
    wplong = currentlong -\
            (tan(angletangage*pi/180)*altitude -
            deltapixy*altitude*lref/(cos(angletangage*pi/180)*href*deltapixrefx))\
            *sin(anglelacet*pi/180) +\
            (tan(angleroulis*pi/180)*altitude +
            deltapixx*altitude*lref/(cos(angleroulis*pi/180)*href*deltapixrefy))\
            *cos(anglelacet*pi/180)
    wplat = wplat/111205.12
    wplong = wplong/73517.0
    return (wplat, wplong)
    
def closeserv():
    print("Fermeture de la connexion")
    connexion_principale.close()
    return

if __name__ == "__main__":
    initserv()
    while True:
        print("Le serveur écoute à présent sur le port {}".format(port))
        try:
            connexion_avec_client, infos_connexion = connexion_principale.accept()
            print("Connexion acceptee de {}".format(infos_connexion))
            while True:
                print("Demande coordonnees...")
                updatetarget()
                sleep(1.)
        except:
            pass
        connexion_avec_client.send("fin".encode())
        connexion_avec_client.close()
    closeserv()
