//// Client.cpp�: d�finit le point d'entr�e pour l'application console.
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
//// D�finition des variables
//// ********************************************************
//WSADATA initialisation_win32; // Variable permettant de r�cup�rer la structure d'information sur l'initialisation
//int erreur; // Variable permettant de r�cup�rer la valeur de retour des fonctions utilis�es
//int tempo; // Variable temporaire de type int
//int nombre_de_caractere; // Indique le nombre de caract�res qui a �t� re�u ou envoy�
//char buffer[65535]; // Tampon contennant les donn�es re�ues
//int var1; //premi�re coordonn�e
//int var2; //deuxi�me coordonn�e
//char strvar1[20];
//char strvar2[20];
//
//fd_set readfs;
//SOCKET id_de_la_socket; // Identifiant de la socket
//SOCKADDR_IN information_sur_la_destination; // D�claration de la structure des informations li� au serveur
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
//		printf("\nDesole, je ne peux pas configurer cette options du � l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nsetsockopt  : OK");
//
//	// ********************************************************
//	// Etablissement de l'ouverture de session
//	// ********************************************************
//	information_sur_la_destination.sin_family=AF_INET;
//	information_sur_la_destination.sin_addr.s_addr=inet_addr("127.0.0.1"); // Indiquez l'adresse IP de votre serveur 
//	information_sur_la_destination.sin_port=htons(12800); // Port �cout� du serveur (12800)
//	erreur=connect(id_de_la_socket,(struct sockaddr*)&information_sur_la_destination,sizeof(information_sur_la_destination));
//	if (erreur!=0)
//		printf("\nDesole, je n'ai pas pu ouvrir la session TCP : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nsetsockopt  : OK");
//
//	// ********************************************************
//	// Envoi des donn�es
//	// ********************************************************
//	const timeval timer = {0,0};//attente de 0sec et 0�sec pour les donn�es(fonction recv non bloquante)
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
//			continue;//ici le code si la temporisation (dernier argument) est �coul�e (il faut bien �videmment avoir mis quelque chose en dernier argument).
//		}	
//
//		if(FD_ISSET(id_de_la_socket, &readfs))
//		{
//			/* des donn�es sont disponibles sur le socket */
//			/* traitement des donn�es */
//			nombre_de_caractere=recv(id_de_la_socket,buffer,1515,0);
//			if (nombre_de_caractere==SOCKET_ERROR){
//				printf("\nDesole, je n'ai pas recu de donnee");
//				break;
//			}
//			else
//			{
//				buffer[nombre_de_caractere]=0; // Permet de fermer le tableau apr�s le contenu des data, car la fonction recv ne le fait pas
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
//	// Fermeture de la session TCP Correspondant � la commande connect()
//	// ********************************************************
//	erreur=shutdown(id_de_la_socket,2); // 2 signifie socket d'�mission et d'�coute
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas fermer la session TCP du a l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nshutdown    : OK");
//
//	// ********************************************************
//	// Fermeture de la socket correspondant � la commande socket()
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
//	erreur=WSACleanup(); // A appeler autant de fois qu'il a �t� ouvert.
//	if (erreur!=0)
//		printf("\nDesole, je ne peux pas liberer winsock du a l'erreur : %d %d",erreur,WSAGetLastError());
//	else
//		printf("\nWSACleanup  : OK");
//}