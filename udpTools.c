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

#include "udpTools.h"

int
testStringComm (void)
{
    struct comm a;

    commInit (&a);
    strncpy (a.padr, "::1", sizeof "::1");
    strncpy (a.nom, "NULL", sizeof "NULL");
    a.port = 4002;

    struct comm b;

    commCpy (&b, &a);

    struct comm array[MAX_CHAN];
    struct comm array2[MAX_CHAN];
    int i;

    for (i = 0; i < MAX_CHAN - 8; i++)
	commCpy (&array[i], &b);

    char res[MAX_ARRAY_COM_STR];

    memset (res, '\0', sizeof (res));
    commArrayToString (array, res);
    commStringToArray (res, array2);

//  printf("res :\n%s",res);
//  envoiUDP (&a, &a, "coucou");
//  printf ("Reception de -> %s\n", buf);
//  char entete[HEADER_LENGTH];
//  memset (entete, '\0', sizeof (entete));
//  lectureEntete (buf, entete);
//  printf ("Entete lue : %s \n", entete);
//  struct comm b;
//  stringToComm (entete, &b);
//  printf("Port lu : %i\n", b.port) ;
//  printf("Nom lu : %s\n", b.nom) ;
//  printf("Adr lue : %s\n", b.padr) ;

    return commCompare (&a, &b) + commArrayCompare (array, array2);
}

void
commInit (struct comm *a)
{
    memset (a->nom, '\0', sizeof (a->nom));
    memset (a->padr, '\0', sizeof (a->padr));
    strncpy (a->padr, "0", strlen ("0"));
    strncpy (a->nom, "0", strlen ("0"));
    a->mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    a->etat = 0;
    a->port = 0;
}

void
commArrayInit (struct comm *array)
{
    int i;

    for (i = 0; i < MAX_CHAN; i++)
	commInit (&array[i]);
}


void
commArrayToString (struct comm *array, char *result)
{
    char tmp[MAXLEN];

    int i;

    for (i = 0; i < MAX_CHAN; i++)
    {
	if (array[i].port != 0)
	{
	    memset (tmp, '\0', sizeof tmp);
	    commToString (&array[i], tmp);
	    strncat (result, tmp, strlen (tmp));
	    strncat (result, "\n", strlen ("\n"));
	    //printf("(%i) %s\n", i, tmp) ;
	}
    }
}

void
commStringToArray (char *string, struct comm *result)
{
    char tmp[MAXLEN];

    int i = 0, j = 0, offset;

    while (string[i] != '\0')
    {
	offset = i;
	memset (tmp, '\0', sizeof tmp);
	for (; string[++i] != '\n' && string[i] != '\0';);
	strncpy (tmp, &string[offset], (unsigned long) (i - offset));
	//printf("(%i)%s\n",j, tmp);
	stringToComm (tmp, &result[j]);
	i++;
	j++;
    }
}

int
commArrayCompare (struct comm *array1, struct comm *array2)
{
    int i, c = 0;

    for (i = 0; i < MAX_CHAN; i++)
	c += commCompare (&array1[i], &array2[i]);
    return c;
}

int
commCompare (struct comm *a, struct comm *b)
{
//    printf("a = %s vs %s = b\n", a->nom, b->nom);
//    printf("a = %s vs %s = b\n", a->padr, b->padr);
    if (a->port != b->port)
	return 1;
    if (strcmp (a->nom, b->nom) != 0)
    {
	return 1;
    }
    if (strcmp (a->padr, b->padr) != 0)
    {
	return 1;
    }
    return 0;
}

void
commCpy (struct comm *dst, struct comm *src)
{
    commInit (dst);
    dst->port = src->port;
    dst->etat = src->etat;
    memset (dst->nom, '\0', sizeof (dst->nom));
    memset (dst->padr, '\0', sizeof (dst->padr));
    strncpy (dst->nom, src->nom, strlen (src->nom));
    strncpy (dst->padr, src->padr, strlen (src->padr));
}

void
commToString (struct comm *e, char *buf)
{
    snprintf (buf, MAX_DIGIT_PORT + 1, "%i", e->port);
    if ((int) strlen (buf) > MAX_DIGIT_PORT)
    {
	printf ("Attention, MAX_DIGIT_PORT = %i \n", MAX_DIGIT_PORT);
    }
    strcat (buf, _SEP);
    strncat (buf, e->nom, sizeof e->nom);
    strcat (buf, _SEP);
    strcat (buf, e->padr);
    strcat (buf, _SEP);
}

void
stringToComm (char *string, struct comm *e)
{
    char port[MAX_DIGIT_PORT + 1];

    memset (port, '\0', sizeof port);
    char name[MAXLEN_NAME];

    memset (name, '\0', sizeof name);
    char addr[IPLEN];

    memset (addr, '\0', sizeof addr);

    int i = 0, offset = 0;

    for (; string[++i] != '/' && string[i] != '\0';);
    strncpy (port, &string[offset], (size_t) i);
    i++;
    offset = i;
    for (; string[++i] != '/' && string[i] != '\0';);
    strncpy (name, &string[offset], (unsigned long) (i - offset));
    i++;
    offset = i;
    for (; string[++i] != '/' && string[i] != '\0';);
    strncpy (addr, &string[offset], (unsigned long) (i - offset));

    commInit (e);
    e->port = (unsigned short) strtol (port, (char **) NULL, 10);
    strncpy (e->nom, name, strlen (name));
    strncpy (e->padr, addr, sizeof addr);
}

void
lectureEntete (char *message, char *entete)
{
    char new[MAXLEN];

    memset (new, '\0', sizeof (new));
    char headSz[2];
    int headerSize = 0;

    strncpy (headSz, message, 2);
    headerSize = (int) strtol (headSz, (char **) NULL, 10);
    if (headerSize > 99)
    {
	printf ("Erreur lecture taille entete %i\n", headerSize);
	headerSize = (int) headerSize / 10;
    }
    //printf ("taille entete : %i\n", headerSize);
    strncpy (entete, &message[2], (unsigned long) headerSize);
    strncpy (new, &message[headerSize + 2],
	     (unsigned long) (MAXLEN - headerSize - 2));
    memset (message, '\0', strlen (message));
    strncpy (message, new, strlen (new));
}

void
ecritureEntete (struct comm *src, char *message)
{
#if DEBUG == 1
//    printf ("Entete sur : %s\n", message);
#endif
    char entete[HEADER_LENGTH];
    char new[MAXLEN];

    memset (new, '\0', sizeof (new));
    memset (entete, '\0', sizeof (entete));
    commToString (src, entete);
    sprintf (new, "%lu", strlen (entete));
    if (strlen (entete) < 10 || strlen (entete) > 99)
	printf ("Erreur taille entete !\n");
    strncat (new, entete, MAXLEN);
    strncat (new, message, MAXLEN);
    strncpy (message, new, sizeof (new));
    //  message = new;
}

int
envoiUDP (struct comm *src, struct comm *dst, const char *message)
{
    //printf("%s\n", message);
    char msg[MAXLEN];

    memset (msg, '\0', sizeof (msg));
    strncpy (msg, message, strlen (message));
    ecritureEntete (src, msg);
    ouverture (dst);


    char dstStr[HEADER_LENGTH];

    memset (dstStr, '\0', sizeof dstStr);
    commToString (dst, dstStr);

    int r;

    r = (int) sendto (dst->s, msg, strlen (msg), 0,
		      (struct sockaddr *) &dst->sadr, dst->salong);

#if DEBUG == 1
    printf ("--> [F: %s\"\n", dstStr);
    printf ("     M:\"%s\"] (%i octets) \n", msg, r);
#endif

    return 0;
}

int
ouverture (struct comm *e)
{
    struct sockaddr_in *sadr4 = (struct sockaddr_in *) &e->sadr;
    struct sockaddr_in6 *sadr6 = (struct sockaddr_in6 *) &e->sadr;
    int family, o;

    memset (&e->sadr, 0, sizeof e->sadr);
    unsigned short port = htons (e->port);

    // Detection du type d'adresse + conversion format network
    if (inet_pton (AF_INET6, e->padr, &sadr6->sin6_addr) == 1)
    {
	family = PF_INET6;
	sadr6->sin6_family = AF_INET6;
	sadr6->sin6_port = port;
	e->salong = sizeof *sadr6;
    }
    else if (inet_pton (AF_INET, e->padr, &sadr4->sin_addr) == 1)
    {
	family = PF_INET;
	sadr4->sin_family = AF_INET;
	sadr4->sin_port = port;
	e->salong = sizeof *sadr4;
    }
    else
    {
	fprintf (stderr, "Adresse '%s' non reconnue\n", e->padr);
	return -1;
    }

    e->s = socket (family, SOCK_DGRAM, 0);
    if (e->s == -1)
    {
	perror ("socket");
	return -1;
    }

    o = 1;
    setsockopt (e->s, SOL_SOCKET, SO_BROADCAST, &o, sizeof o);
    return 0;
}

int
serveur (struct comm *me, void type (), void *arg)
{
    char servR[MAXLEN];

    sprintf (servR, "%i", me->port);

    int s[MAXSOCK], nsock, r, INIT = 1;
    struct addrinfo hints, *res, *res0;
    const char *cause = NULL;

    memset (&hints, 0, sizeof hints);
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    r = getaddrinfo (NULL, servR, &hints, &res0);
    if (r != 0)
    {
	fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (r));
	return -1;
    }

    nsock = 0;
    for (res = res0; res && nsock < MAXSOCK; res = res->ai_next)
    {
	s[nsock] =
	    socket (res->ai_family, res->ai_socktype, res->ai_protocol);
	if (s[nsock] == -1)
	    cause = "socket";
	else
	{
	    int o = 1;		/* pour Linux */

	    setsockopt (s[nsock], IPPROTO_IPV6, IPV6_V6ONLY, &o, sizeof o);

	    r = bind (s[nsock], res->ai_addr, res->ai_addrlen);
	    if (r == -1)
	    {
		cause = "bind";
		close (s[nsock]);
		return -1;
	    }
	    else
		nsock++;
	}
    }
    if (nsock == 0)
	perror (cause);
    freeaddrinfo (res0);

    for (;;)
    {
	if (INIT == 1)
	{
	    INIT = 0;
	    type (me, NULL, (char *) INIT_FLAG, arg);
	}

	// Multiplex
	fd_set readfds;
	int i, max = 0;

	FD_ZERO (&readfds);
	for (i = 0; i < nsock; i++)
	{
	    FD_SET (s[i], &readfds);
	    if (s[i] > max)
		max = s[i];
	}
	if (select (max + 1, &readfds, NULL, NULL, NULL) == -1)
	{
	    perror ("select");
	    return -1;
	}

	for (i = 0; i < nsock; i++)
	    if (FD_ISSET (s[i], &readfds))
	    {
		reception (me, s[i], type, arg);
		//if ((int) arg == ONCE_RCP)
		//   return 0;
	    }
    }
}

void
reception (struct comm *me, int s, void type (), void *arg)
{
    struct sockaddr_storage sonadr;
    socklen_t salong;
    int r, af;
    void *nadr = NULL;		/* au format network */
    char padr[INET6_ADDRSTRLEN];	/* au format presentation */

    char buf[MAXLEN];

    memset (buf, '\0', sizeof buf);

    salong = sizeof sonadr;
    r = (int) recvfrom (s, buf, MAXLEN, 0, (struct sockaddr *) &sonadr,
			&salong);
    af = ((struct sockaddr *) &sonadr)->sa_family;
    switch (af)
    {
	case AF_INET:
	    nadr = &((struct sockaddr_in *) &sonadr)->sin_addr;
	    break;
	case AF_INET6:
	    nadr = &((struct sockaddr_in6 *) &sonadr)->sin6_addr;
	    break;
    }
    inet_ntop (af, nadr, padr, sizeof padr);

#if DEBUG == 1
    //printf ("(me) %s \n ", padr);
#endif
    struct comm src;

    commInit (&src);

    char entete[HEADER_LENGTH];

    memset (entete, '\0', sizeof entete);

    char source[HEADER_LENGTH];

    memset (source, '\0', sizeof source);

    lectureEntete (buf, entete);
    stringToComm (entete, &src);
    commToString (&src, source);

#if DEBUG == 1
    printf ("<-- [F: %s\n", source);
    printf ("     M:\"%s\" ] (%i octets) \n", buf, r);
#endif

    type (me, &src, buf, arg);
    memset (buf, '\0', sizeof buf);
}
