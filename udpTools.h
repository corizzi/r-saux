/*
 *
 *    Ce code a été écrit dans le cadre d'un cours de réseaux
 *    suivi à l'université de Strasbourg.
 *
 *    Copyright Alexandre Corizzi (alexandre.corizzi@gmail.com)
 *
 *    Copyright Pierre David (pda@unistra.fr)
 *
 *    Ce logiciel est régi par la licence CeCILL-B soumise au droit
 *    français et respectant les principes de diffusion des logiciels
 *    libres. Vous pouvez utiliser, modifier et/ou redistribuer ce programme
 *    sous les conditions de la licence CeCILL-B telle que diffusée par
 *    le CEA, le CNRS et l'INRIA sur le site "http://www.cecill.info".
 *
*/


#ifndef UDPTOOL
#define UDPTOOL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>	// open()
#include <ctype.h>	// isdigit()
#include <pthread.h>

#define _SEP "/"

#define	MAXLEN	1024
#define	MAXLEN_NAME 32	// Convention, le nom ne depasse pas 80 (console)
#define	MAXSOCK	32
#define HEADER_LENGTH 512
#define MAX_DIGIT_PORT 4
#define MAX_CHAN 10
#define MAX_CLI 10
#define IPLEN 39

// Taille max = port/addresse/nom/seperations\n * nombre Cannaux max
#define MAX_ARRAY_COM_STR (MAX_DIGIT_PORT+39+MAXLEN_NAME+3+2)*MAX_CHAN

#define INIT_FLAG 1
#define SPECIAL_FLAG 2
#define INIT_CODE "INIT"

#define ONCE_RCP 1

struct comm
{
    char nom[MAXLEN];
    char padr[IPLEN + 1];
    int s;
    int etat;
    pthread_mutex_t mutex;
    struct sockaddr_storage sadr;
    socklen_t salong;
    unsigned short port;
    char padding[2];
};

int testStringComm (void);
int envoiUDP (struct comm *src, struct comm *dst, const char *message);
void raler (char *msg);
void reception (struct comm *me, int s, void type (), void *arg);
int serveur (struct comm *me, void type (), void *arg);
void ecritureEntete (struct comm *src, char *message);
void lectureEntete (char *message, char *entete);
int testStringComm ();

void commInit (struct comm *a);
void commToString (struct comm *e, char *buf);
void commCpy (struct comm *dst, struct comm *src);
void commArrayToString (struct comm *array, char *result);
void commStringToArray (char *string, struct comm *result);
void stringToComm (char *buf, struct comm *e);
int commCompare (struct comm *, struct comm *);
int commArrayCompare (struct comm *, struct comm *);
void commArrayInit (struct comm *array);

//! \brief Ouverture d'une socket vers un serveur UDP 
//! \param[in] e "envoi * e" Pointeur sur l'objet vers lequel communiquer
int ouverture (struct comm *e);


//void commande ( char * buf ) ;
//void help () ;
//void configuration_reader ( char * port ) ;
//void printSep() ;
//unsigned short port_check ( char * port ) ;
//int traiterRequete ( char * buf ) ;

#endif
