#Client side
import sys
sys.path.append(r"C:\Python33\Lib")
import socket
import time
from math import *
import clr
import MissionPlanner

currGPSpos = ()
initGPSWP = None
initMAVWP = None
currGPSWP =  None
currMAVWP = None

a = pi / 180

def initcharge():

    print ('Installation charge 10s')
    time.sleep(1)
    print ('9s')
    time.sleep(1)
    print ('8s')
    time.sleep(1)
    print ('7s')
    time.sleep(1)
    print ('6s')
    time.sleep(1)
    print ('5s')
    time.sleep(1)
    print ('4s')
    time.sleep(1)
    print ('3s')
    time.sleep(1)
    print ('2s')
    time.sleep(1)
    print ('1s')
    time.sleep(1)
    print ('0s')
    Script.SendRC(6,1600,True)
    time.sleep(2)
    Script.SendRC(6,1400,True)
    time.sleep(4)
    Script.SendRC(6,1800,True)
    time.sleep(4)
    Script.SendRC(6,1600,True)
    print ('Installation charge terminée')
clr.AddReference("MissionPlanner.Utilities") # includes the Utilities class

def initclient():
    #Initialisation connexion serveur
    print("Bienvenue dans le Client MAVLink")
    hote = "localhost"
    port = 12800
    global connexion_avec_serveur
    global msg_recu
    connexion_avec_serveur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connexion_avec_serveur.connect((hote, port))

    #Récupération des GPSWP des cibles initiales
    msg_recu = connexion_avec_serveur.recv(200)
    msg_recu = msg_recu.decode()
    msg_recu = msg_recu.split(",")
    initGPSWP = [(),()]
    initGPSWP[0] = (float(msg_recu[0]),float(msg_recu[1]))
    initGPSWP[1] = (float(msg_recu[2]),float(msg_recu[3]))
    print(initGPSWP[0])
    print(initGPSWP[1])

    print("Création initWP") 
    #Création des MAVWP des cibles initiales, et init des MAV/GPSWP vers la cible 1
    initMAVWP = [0,0]
    for i in range(2):
        initMAVWP[i] = MissionPlanner.Utilities.Locationwp() # creating waypoint
        lat = initGPSWP[i][0]                       # Latitude value
        lng = initGPSWP[i][1]                       # Longitude value
        alt = 5                                     # altitude value
        MissionPlanner.Utilities.Locationwp.lat.SetValue(initMAVWP[i],lat)   # sets latitude
        MissionPlanner.Utilities.Locationwp.lng.SetValue(initMAVWP[i],lng)   # sets longitude
        MissionPlanner.Utilities.Locationwp.alt.SetValue(initMAVWP[i],alt)   # sets altitude
        if i==0:
            currGPSWP = (lat,lng)
    print("curmavinit1")
    currMAVWP = initMAVWP[0]
    print("curmavinit2")
    return

def waitdata():
    actualtarget = 0
    distok = 0
    x = ()
    nbpertedata = 0
    print("Attente cible 1")
    #En mode auto
    while actualtarget==0:
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu = msg_recu.decode()
        if msg_recu=="cible1":
            actualtarget = 1
            wpstop = cs.wpno
            Script.ChangeMode("Guided")
            MAV.setGuidedModeWP(initMAVWP[0])
            print("Guided mode ok")
        else:
            print(msg_recu)
            return "Input Target 1 Error"

    #En mode Guided vers la cible 1
    distok = 0
    while actualtarget==1:
        currGPSpos = (cs.lat,cs.lng)
        if distcalc(currGPSpos, initGPSWP[0]) < 10:

            #Demande de données cam
            connexion_avec_serveur.send("Data pls".encode())
            msg_recu = connexion_avec_serveur.recv(100)
            msg_recu =  msg_recu.decode()

            #Si aucune croix trouvée
            if msg_recu=="no target":
                print("On a perdu la cible!")
                nbpertedata = nbpertedata+1
                if nbpertedata==5:
                    print("Retour à la source")
                    currMAVWP = initMAVWP[0]
                    MAV.setGuidedModeWP(currMAVWP)
                    currGPSWP = initGPSWP[0]
                    distok = 0
                    nbpertedata=0

            #Si croix trouvée
            else:
                x = msg_recu.split(",")
                print("Updating Target: " + x[0], x[1])
                tempGPSWP = getwaypoint(cs.yaw,cs.roll,cs.pitch,cs.alt,currGPSpos[0],currGPSpos[1],float(x[0])*360,float(x[1])*288)
                print(tempGPSWP)

                print("actualisation wp")
                #Actualisation du WP
                if dist(tempGPSWP, initGPSWP[0]) <10:
                    currMAVWP = MissionPlanner.Utilities.Locationwp() # creating waypoint
                    lat = tempGPSWP[0]                                   # Latitude value
                    lng = tempGPSWP[1]                                   # Longitude value
                    alt = 5                                              # altitude value
                    MissionPlanner.Utilities.Locationwp.lat.SetValue(currMAVWP,lat)     # sets latitude
                    MissionPlanner.Utilities.Locationwp.lng.SetValue(currMAVWP,lng)   # sets longitude
                    MissionPlanner.Utilities.Locationwp.alt.SetValue(currMAVWP,alt)     # sets altitude
                    MAV.setGuidedModeWP(currMAVWP)
                    currGPSWP = (lat,lng)
                i = 0

            print("test largage")
            #Test si ok pour largage
            if dist(currGPSpos, currGPSWP) < 1:
                distok = distok+1
                if distok ==5:
                    Script.SendRC(6,1400,True) #servo in pos 2 : open
                    print ("Charge 1 larguée")
                    Script.ChangeMode("Auto")
                    MAV.setWPCurrent(wpstop)
                    break
            else:
                distok = 0
                
        sleep(2.)

    currMAVWP = initMAVWP[1]
    currGPSWP = initGPSWP[1]
    while actualtarget==1:
        msg_recu = connexion_avec_serveur.recv(100)
        msg_recu = msg_recu.decode()
        if msg_recu=="cible2":
            actualtarget = 2
            wpstop = cs.wpno
            Script.ChangeMode("Guided")
            MAV.setGuidedModeWP(currMAVWP)
        else:
            print(msg_recu)
            return "Input Target 2 Error"

    #En mode Guided vers la cible 2
    distok = 0
    while actualtarget==2:
        currGPSpos = (cs.lat,cs.lng)
        if distcalc(currGPSpos, initGPSWP[1]) < 10:

            #Demande de données cam
            connexion_avec_serveur.send("Data pls".encode())
            msg_recu = connexion_avec_serveur.recv(100)
            msg_recu =  msg_recu.decode()

            #Si aucune croix trouvée
            if msg_recu=="no target":
                print("On a perdu la cible!")
                nbpertedata = nbpertedata+1
                if nbpertedata==5:
                    print("Retour à la source")
                    currMAVWP = initMAVWP[1]
                    MAV.setGuidedModeWP(currMAVWP)
                    currGPSWP = initGPSWP[1]
                    distok = 0
                    nbpertedata=0

            #Si croix trouvée
            else:
                x = msg_recu.split(",")
                print("Updating Target: " + x[0], x[1])
                tempGPSWP = getwaypoint(cs.yaw,cs.roll,cs.pitch,cs.alt,currGPSpos[0],currGPSpos[1],float(x[0]),float(x[1]))
                print(tempGPSWP)

                #Actualisation du WP
                if dist(tempGPSWP, initGPSWP[1]) <10:
                    currMAVWP = MissionPlanner.Utilities.Locationwp() # creating waypoint
                    lat = tempGPSWP[0]                                   # Latitude value
                    lng = tempGPSWP[1]                                   # Longitude value
                    alt = 5                                              # altitude value
                    MissionPlanner.Utilities.Locationwp.lat.SetValue(currMAVWP,lat)     # sets latitude
                    MissionPlanner.Utilities.Locationwp.lng.SetValue(currMAVWP,lng)   # sets longitude
                    MissionPlanner.Utilities.Locationwp.alt.SetValue(currMAVWP,alt)     # sets altitude
                    MAV.setGuidedModeWP(currMAVWP)
                    currGPSWP = (lat,lng)
                i = 0

            #Test si ok pour largage
            if dist(currGPSpos, currGPSWP) < 1:
                distok = distok+1
                if distok ==5:
                    Script.SendRC(6,1800,True) #servo in pos 2 : open
                    print ("Charge 2 larguée")
                    Script.ChangeMode("Auto")
                    MAV.setWPCurrent(wpstop)
                    break
            else:
                distok = 0
                
        sleep(2.)
    print("FIN DE LA MISSION")
    return



def getwaypoint(anglelacet, angleroulis, angletangage, altitude, currentlat, currentlong, deltapixx, deltapixy):
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
    deltapixrefx = 192 #taille de la regle pour le calibrage, apres correction fisheye
    deltapixrefy = 265 #taille de la règle pour le calibrage, après correction fisheye
    wplat = currentlat -\
            (tan(-angletangage*pi/180)*altitude -
            deltapixy*altitude*lref/(cos(-angletangage*pi/180)*href*deltapixrefx))\
            *cos(anglelacet*pi/180) +\
            (tan(angleroulis*pi/180)*altitude +
            deltapixx*altitude*lref/(cos(angleroulis*pi/180)*href*deltapixrefy))\
            *sin(anglelacet*pi/180)
    wplong = currentlong -\
            (tan(-angletangage*pi/180)*altitude -
            deltapixy*altitude*lref/(cos(-angletangage*pi/180)*href*deltapixrefx))\
            *sin(anglelacet*pi/180) +\
            (tan(angleroulis*pi/180)*altitude +
            deltapixx*altitude*lref/(cos(angleroulis*pi/180)*href*deltapixrefy))\
            *cos(anglelacet*pi/180)
    wplat = wplat/111205.12
    wplong = wplong/73517.0
    return (wplat, wplong)

def distcalc(WP1,WP2):
    pos = (WP1[0]*a,WP1[1]*a) # drone pos
    pos2 = (WP2[0]*a,WP2[1]*a)
    t1 = sin(pos[0]) * sin(pos2[0])#distance to the target calcul
    t2 = cos(pos[0]) * cos(pos2[0])
    t3 = cos(pos[1] - pos2[1])
    t4 = t2 * t3
    t5 = t1 + t4
    rad_dist = atan(-t5/sqrt(-t5 * t5 +1)) + 2 * atan(1)
    return ((rad_dist * 3437.74677 * 1.1508) * 1.6093470878864446)*1000

#if __name__ == "__main__":
#initcharge()
initclient()
waitdata()
input()
