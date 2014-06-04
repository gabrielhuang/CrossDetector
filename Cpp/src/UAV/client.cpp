//// Client.cpp : définit le point d'entrée pour l'application console.
////
//
//#include <winsock2.h> // pour les fonctions socket
////#include <cstdio> // Pour les Sprintf
//#include <time.h> //Pour le random
//#include <string.h>
//
//#include <iostream>
//
//
//
//// ********************************************************
//// Définition des variables
//// ********************************************************
//WSADATA initialisation_win32; // Variable permettant de récupérer la structure d'information sur l'initialisation
//int erreur; // Variable permettant de récupérer la valeur de retour des fonctions utilisées
//int tempo; // Variable temporaire de type int
//int nombre_de_caractere; // Indique le nombre de caractères qui a été reçu ou envoyé
//char buffer[65535]; // Tampon contennant les données reçues
//int var1; //première coordonnée
//int var2; //deuxième coordonnée
//char strvar1[20];
//char strvar2[20];
//
//fd_set readfs;
//SOCKET id_de_la_socket; // Identifiant de la socket
//SOCKADDR_IN information_sur_la_destination; // Déclaration de la structure des informations lié au serveur
//
//int main (int argc, char* argv[])
//{
//	printf("\nBonjour, vous etes du cote client.\n");
//
//	// ********************************************************
//	// Initialisation de Winsock
//	// ********************************************************
//	erreur=WSAStartup(MAKEWORD(2,2),&initialisation_win32);
//	if (erreur!=0)
//		cout << "\nDesole, je ne peux pas initialiser Winsock du a l'erreur : %d %d" << erreur << WSAGetLastError());
//	else
//		printf("\nWSAStartup  : OK");
//
//	// ********************************************************
//	// Ouverture d'une Socket
//	// ********************************************************
//	id_de_la_socket=socket(AF_INET,SOCK_STREAM,0);
//	if (id_de_la_socket==INVALID_SOCKET)
//		printf("\nDesole, je ne peux pas creer la socket du a l'erreur : %d",WSAGetLastError());
//	else
//		printf("\nsocket      : OK");
//
//	// ********************************************************
//	// Activation de l'option permettant d'activer l'algorithme de Nagle
//	// ********************************************************
//	tempo=1;
//	erreur=setsockopt(id_de_la_socket,IPPROTO_TCP,TCP_NODELAY,(char *)&tempo,sizeof(tempo));
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas configurer cette options du à l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nsetsockopt  : OK");
//
//	// ********************************************************
//	// Etablissement de l'ouverture de session
//	// ********************************************************
//	information_sur_la_destination.sin_family=AF_INET;
//	information_sur_la_destination.sin_addr.s_addr=inet_addr("127.0.0.1"); // Indiquez l'adresse IP de votre serveur 
//	information_sur_la_destination.sin_port=htons(12800); // Port écouté du serveur (12800)
//	erreur=connect(id_de_la_socket,(struct sockaddr*)&information_sur_la_destination,sizeof(information_sur_la_destination));
//	if (erreur!=0)
//		printf("\nDesole, je n'ai pas pu ouvrir la session TCP : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nsetsockopt  : OK");
//
//	// ********************************************************
//	// Envoi des données
//	// ********************************************************
//	const timeval timer = {0,0};//attente de 0sec et 0µsec pour les données(fonction recv non bloquante)
//	while(1){
//		var1 = rand()%720 - 360;
//		var2 = rand()%576 - 288;
//		itoa(var1,strvar1,10);
//		itoa(var2,strvar2,10);
//		strcat(strvar1,",");
//		strcat(strvar1,strvar2);
//		printf("\nEn attente d'ordres");
//		int ret = 0;
//		FD_ZERO(&readfs);
//		FD_SET(id_de_la_socket, &readfs);
//
//		if((ret = select(id_de_la_socket + 1, &readfs, NULL, NULL, &timer)) < 0)
//		{
//			perror("select()");
//			exit(errno);
//		}
//
//		if(ret == 0)
//		{
//			continue;//ici le code si la temporisation (dernier argument) est écoulée (il faut bien évidemment avoir mis quelque chose en dernier argument).
//		}	
//
//		if(FD_ISSET(id_de_la_socket, &readfs))
//		{
//			/* des données sont disponibles sur le socket */
//			/* traitement des données */
//			nombre_de_caractere=recv(id_de_la_socket,buffer,1515,0);
//			if (nombre_de_caractere==SOCKET_ERROR){
//				printf("\nDesole, je n'ai pas recu de donnee");
//				break;
//			}
//			else
//			{
//				buffer[nombre_de_caractere]=0; // Permet de fermer le tableau après le contenu des data, car la fonction recv ne le fait pas
//				printf("\nVoici les donnees : %s",buffer);
//				if(memcmp(buffer,"update", strlen(buffer))==0){
//					nombre_de_caractere=send(id_de_la_socket,strvar1,strlen(strvar1),0);
//					if (nombre_de_caractere==SOCKET_ERROR)
//						printf("\nDesole, je n'ai pas envoyer les donnees du a l'erreur : %d",WSAGetLastError());
//					else
//						printf("\nsend        : OK");
//				}
//				if(memcmp(buffer,"fin", strlen(buffer))==0)
//					break;
//			} 
//		}
//	}
//	// ********************************************************
//	// Fermeture de la session TCP Correspondant à la commande connect()
//	// ********************************************************
//	erreur=shutdown(id_de_la_socket,2); // 2 signifie socket d'émission et d'écoute
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas fermer la session TCP du a l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nshutdown    : OK");
//
//	// ********************************************************
//	// Fermeture de la socket correspondant à la commande socket()
//	// ********************************************************
//	erreur=closesocket(id_de_la_socket);
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas liberer la socket du a l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nclosesocket : OK");
//
//	// ********************************************************
//	// Quitte proprement le winsock ouvert avec la commande WSAStartup
//	// ********************************************************
//	erreur=WSACleanup(); // A appeler autant de fois qu'il a été ouvert.
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas liberer winsock du a l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nWSACleanup  : OK");
//}