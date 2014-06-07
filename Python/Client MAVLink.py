#Client side
import sys
sys.path.append(r"C:\Python33\Lib")
import socket
from time import sleep
from math import *

def initclient():
    print("Bienvenue dans le Client MAVLink")
    hote = "localhost"
    port = 12800
    global connexion_avec_serveur
    global msg_recu
    connexion_avec_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_avec_serveur.connect((hote, port))
    msg_recu = connexion_avec_serveur.recv(200)
    msg_recu = msg_recu.decode()
    msg_recu = msg_recu.split(",")
    global TargetGPSWP
    TargetGPSWP = [0,0]
    TargetGPSWP[0] = (float(msg_recu[0]),float(msg_recu[1]))
    TargetGPSWP[1] = (float(msg_recu[2]),float(msg_recu[3]))
    print(TargetGPSWP[0])
    print(TargetGPSWP[1])
    return

def waitdata():
    actualtarget = 0
    distok = 0
    x = ()
    newWP = TargetGPSWP[0]
    i = 0
    while actualtarget==0:
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu = msg_recu.decode()
        if msg_recu=="cible1":
            actualtarget = 1
            #SWITCH TO GUIDED MODE
        else:
            print(msg_recu)
            return "Input Target 1 Error"
    j=0
    while actualtarget==1:
        j = j+1
        if j == 10:
            break
    ##IF DIST TO TARGET < 30:
        #IF DIST TO TARGET < 1:
            #distok++
            #IF distok ==5:
                # LARGUER ET AUTO, PUIS BREAK
        #ELSE:
            #distok=0
        connexion_avec_serveur.send("Data pls".encode())
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu =  msg_recu.decode()
        if msg_recu=="no target":
            print("On a perdu la cible!")
            i = i+1
            if i==5:
                print("Retour à la source")
                newWP = TargetGPSWP[0]
        else:
            x = msg_recu.split(",")
            print("Updating Target: " + x[0], x[1])
            newWP = getwaypoint(0,0,0,10,newWP[0],newWP[1],float(x[0]),float(x[1]))
            print(newWP)
            #MAJ WP
            i = 0
        sleep(1.)
    ##
    while actualtarget==1:
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu = msg_recu.decode()
        if msg_recu=="cible2":
            actualtarget = 2
            newWP = TargetGPSWP[1]
            #SWITCH TO GUIDED MODE
        else:
            print(msg_recu)
            return "Input Target 2 Error"
        
    while actualtarget==2:
        ##IF DIST TO TARGET < 30:
        #IF DIST TO TARGET < 1:
            #distok++
            #IF distok ==5:
                # LARGUER ET AUTO, PUIS BREAK
        #ELSE:
            #distok=0
        connexion_avec_serveur.send("Data pls".encode())
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu =  msg_recu.decode()
        if msg_recu=="no target":
            print("On a perdu la cible!")
            i = i+1
            if i==5:
                print("Retour à la source")
                newWP = TargetGPSWP[1]
        else:
            x = msg_recu.split(",")
            print("Updating Target: " + x[0], x[1])
            newWP = getwaypoint(0,0,0,10,newWP[0],newWP[1],float(x[0]),float(x[1]))
            print(newWP)
            #MAJ WP
            i = 0
        sleep(1.)
    print("FIN DE LA MISSION")
    return

    

def getwaypoint(anglelacet, angleroulis, angletangage, altitude, currentlat, currentlong, deltapixx, deltapixy):
    deltapixx = deltapixx*1 #correction fisheye
    deltapixy = deltapixy*1 #correction fisheye
    """
    la latitude augmente vers le nord (Equateur = 0)
    la longitude augmente vers l'est (Greenwich = 0)
    les angles sont nuls à l'horizontale, pointant vers le nord
    les angles en degres
    l'altitude en m
    la position GPS en degres
    angleroulis est positif si le drone va vers la droite
    angletangage est positif si le drone va vers l'avant
    les deltapix sont nuls si le barycentre est au milieu de l'image,
    positifs si il est en haut à droite
    
    """
    currentlat = currentlat*111205.12 #conversion en m
    currentlong = currentlong*73517.0 #conversion en m
    lref = 2.50 #longueur de la regle utilisee pour le calibrage en m
    href = 2.70 #hauteur de la camera pour lé calibrage
    deltapixrefx = 192 #taille ée la regle pour le calibrage, apres correction fisheye
    deltapixrefy = 265 #taille de la règle pour le calibrage, après correction fisheye
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

#if __name__ == "__main__":
initclient()
waitdata()
