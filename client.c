/*
 *
 *    Ce code a été écrit dans le cadre d'un cours de réseaux
 *    suivi à l'université de Strasbourg.
 *
 *    Copyright Alexandre Corizzi (alexandre.corizzi@gmail.com)
 *
 *    Ce logiciel est régi par la licence CeCILL-B soumise au droit
 *    français et respectant les principes de diffusion des logiciels
 *    libres. Vous pouvez utiliser, modifier et/ou redistribuer ce programme
 *    sous les conditions de la licence CeCILL-B telle que diffusée par
 *    le CEA, le CNRS et l'INRIA sur le site "http://www.cecill.info".
 *
*/

#include "udpTools.h"

// Parametres :
#define CHAN_ALIVE_MAX 5
#define SUFF_SUP "_sup"

#define ESPACE_PORT 1000
#define PLAGE_CANAL 1000
#define PLAGE_CLIENT 2000

// Différents Etats d'un comm
#define STATE_INIT      0	/*!< Etat de base. */

#define STATE_CL_SIM_INIT    1	/*!< Est un client. */
#define STATE_CL_SUP_INIT    2	/*!< Est un client. */

#define STATE_CL_SIM         4	/*!< Est un client. */
#define STATE_CL_SUP         5	/*!< Est un client. */

#define STATE_CH_ASK         7	/*!< Est un canal. */
#define STATE_CH_SUP         8	// est un canal suppléé //
#define STATE_CH_SUP_INIT    9
#define STATE_CH_WAT         10	// est un canal supplant //
#define STATE_ANNUAIRE       11	/*!< Est un annuaire. */


// Différentes entetes
#define SIZE_CODE 4

#define CODE_OK           "0000"

#define CODE_AliveReq  "0001"
#define CODE_AliveAns  "0002"

#define CODE_AN_addChan   "0003"
#define CODE_AN_ListReq   "0004"
#define CODE_AN_ListAns   "0005"
#define CODE_AN_ListEmpty "0006"

#define CODE_CH_addCli    "0007"
#define CODE_CH_addSup    "0008"
#define CODE_CH_msgCli    "0009"
#define CODE_CH_supID     "0010"
#define CODE_CH_ListAns   "0011"
#define CODE_CH_SPECIAL   "0012"

#define CODE_CL_replace   "0013"
#define CODE_CL_msgCha    "0014"
#define CODE_CL_desSup    "0015"

void annuaireSub (struct comm *me, struct comm *src, char *buf, void *arg);
void *annuaireThread (void *arg);

void canalSub (struct comm *me, struct comm *src, char *buf, void *arg);
void *canalThread (void *arg);

void clientSub (struct comm *me, struct comm *src, char *buf, void *arg);
void *clientThread (void *arg);

void *watchingThread (void *arg);

void tests (void);

static int ALIVE_SHARED;
static pthread_mutex_t ALIVE_mutex = PTHREAD_MUTEX_INITIALIZER;

// Configuration commune : un annuaire principal
//                       + un tableau de 2 comm

static struct comm annuaire;
static struct comm moi[2];

#ifndef DEBUG
#define DEBUG = 0
#endif

int
main (int argc, char *argv[])
{

    srand ((unsigned int) time (NULL));
    commInit (&annuaire);
    commInit (&moi[0]);
    commInit (&moi[1]);

    annuaire.etat = STATE_ANNUAIRE;
    strncpy (annuaire.nom, "NULL", sizeof "NULL");

#if DEBUG == 1 || DEBUG == 2
    //strncpy (annuaire.padr, "::1", sizeof "::1");
    strncpy (annuaire.padr, "127.0.0.1", sizeof "127.0.0.1");
    annuaire.port = 9000;
    printf (" --------   Mode DEBUG    -------- \n");
    if (argc == 2)
    {
	if (strcmp (argv[1], "--annuaire") == 0)
	{
	    // Lancement de l'annuaire
	    printf ("Annuaire\n");
	    annuaireThread (NULL);
	}
	else if (strcmp (argv[1], "--canal") == 0)
	{
	    printf ("Canal\n");
	    moi[0].etat = STATE_INIT;
	    strncpy (moi[0].padr, "::1", sizeof "::1");
	    strncpy (moi[0].nom, "Channel1", sizeof "Channel1");
	    canalThread (&moi);
	}
	else if (strcmp (argv[1], "--simple") == 0)
	{
	    printf ("Client Simple \n");
	    moi[0].etat = STATE_CL_SIM_INIT;
	    strncpy (moi[0].padr, "::1", sizeof "::1");
	    strncpy (moi[0].nom, "Alice", sizeof "Alice");
	    clientThread (&moi);
	}

	else if (strcmp (argv[1], "--classique") == 0)
	{
	    printf ("Client Classique \n");
	    moi[0].etat = STATE_CL_SUP_INIT;
	    strncpy (moi[0].padr, "::1", sizeof ("::1"));
	    strncpy (moi[0].nom, "Bob", sizeof ("Bob"));
	    clientThread (&moi);
	}
	else if (strcmp (argv[1], "--test") == 0)
	{
	    tests ();
	}
	else
	{
	    printf ("Argument invalide\n");
	}
    }
    else
    {
	printf ("Nombre d'arguments invalide\n");
    }
#else
#endif
    exit (0);
}

void
tests (void)
{
    if (testStringComm () == 0)
	printf ("Test String-Comm : ok \n");
    else
	printf ("Test String-Comm : echec \n");
}

void
annuaireSub (struct comm *me, struct comm *src, char *buf, void *arg)
{
    struct comm *listeCannaux = arg;

    if ((int) buf == INIT_FLAG)
    {
	printf ("Initialisation de l'annuaire %s...\n", me->nom);
    }

    // Inscription d'un nouveau serveur : 
    else if (strncmp (buf, CODE_AN_addChan, SIZE_CODE) == 0)
    {
	// Pour l'affichage 
	char source[HEADER_LENGTH];

	memset (source, '\0', sizeof source);
	commToString (src, source);

	printf ("Nouveau canal : %s \n", source);

	int i;

	for (i = 0; i < MAX_CHAN && listeCannaux[i].port != 0; i++);
	commCpy (&listeCannaux[i], src);
	sleep (1);
	envoiUDP (me, src, CODE_OK);
    }

    // Reponse pour liste des cannaux
    else if (strncmp (buf, CODE_AN_ListReq, SIZE_CODE) == 0)
    {
	char listeStr[MAX_ARRAY_COM_STR];

	memset (listeStr, '\0', sizeof listeStr);
	sleep (1);
	strncpy (listeStr, CODE_AN_ListAns, strlen (CODE_AN_ListAns));
	strncat (listeStr, "\n", strlen ("\n"));
	commArrayToString (listeCannaux, listeStr);
	if (listeCannaux[0].port != 0)
	    envoiUDP (me, src, listeStr);
	else
	    envoiUDP (me, src, CODE_AN_ListEmpty);
    }
}

void *
annuaireThread (void *arg)
{
    (void) arg;
    struct comm listeCannaux[MAX_CHAN];

    commArrayInit (listeCannaux);

    serveur (&annuaire, &annuaireSub, listeCannaux);
    pthread_exit (NULL);
}

void
canalSub (struct comm *me, struct comm *src, char *buf, void *arg)
{
    struct comm *listeClients = arg;

    // INITIALISATION 
    if ((int) buf == INIT_FLAG)
    {
	if (me->etat == STATE_INIT)
	{
	    printf ("Initialisation du canal %s...\n", me->nom);
	    printf ("Sur : %s:%i\n", me->padr, me->port);
	    printf ("Prise de contact avec le serveur annuaire...\n");
	    pthread_mutex_lock (&annuaire.mutex);
	    envoiUDP (me, &annuaire, CODE_AN_addChan);
	    pthread_mutex_unlock (&annuaire.mutex);

	    // Si la liste n'est pas vide on, BroadCast à tous les clients
	    // le changement d'adresse du canal.
	    if (listeClients[0].port != 0)
	    {
		char remp[MAXLEN];
		char chan[MAXLEN];

		memset (remp, '\0', sizeof remp);
		strncpy (remp, CODE_CL_replace, strlen (CODE_CL_replace));
		commToString (&me[1], chan);
		strncat (remp, chan, strlen (chan));

		int i;

		for (i = 0; i < MAX_CHAN && listeClients[i].port != 0; i++)
		{
		    envoiUDP (me, &listeClients[i], remp);
		    printf ("Envoie à %s\n", listeClients[i].nom);
		}
	    }

	    commInit (&me[1]);
	    me->etat = STATE_CH_ASK;
	}
	else if (me->etat == STATE_CH_WAT)
	{
	    printf ("Initialisation du canal suppléant : %s...\n",
		    me->nom);
	    printf ("Sur : %s:%i\n", me->padr, me->port);

	    envoiUDP (&me[0], &me[1], CODE_CH_supID);

	    // On thread un "watcher" pour surveiller
	    // l'etat du serveur
	    pthread_mutex_lock (&ALIVE_mutex);
	    ALIVE_SHARED = 0;
	    pthread_mutex_unlock (&ALIVE_mutex);

	    pthread_t watcher;

	    if (pthread_create (&watcher, NULL, &watchingThread, me))
	    {
		printf ("Impossible de demarrer un surveillant.\n");
	    }
	}
    }
    // Reception du flag special ('alarme par le thread')
    else if ((strncmp (buf, CODE_CH_SPECIAL, SIZE_CODE) == 0))
    {
	// Si je suis channel suppléé :
	if (me->etat == STATE_CH_SUP)
	{
	    // Je pleure
	    printf ("%s n'est plus suppléant.\n", me[1].nom);
	    // TODO Je l'efface mon suppléant de la liste

	    // je redeviens demandeur de suppléant
	    me->etat = STATE_CH_ASK;
	    canalSub (me, NULL, (char *) INIT_FLAG, arg);
	}
	// Si je suis un surveillant :
	else if (me->etat == STATE_CH_WAT)
	{
	    // Je change d'état...
	    me->etat = STATE_INIT;
	    // ...pour m''initialiser
	    canalSub (me, NULL, (char *) INIT_FLAG, arg);
	}
	else
	    printf ("Erreur 'Alarme' : etat incongru !");

    }
    // Inscription sur l'annuaire effective
    else if (strncmp (buf, CODE_OK, SIZE_CODE) == 0)
    {
	printf ("...Inscription ok ! :)\n");
    }
    // Demande d'abonnement d'un client
    else if (strncmp (buf, CODE_CH_addCli, SIZE_CODE) == 0 ||
	     strncmp (buf, CODE_CH_addSup, SIZE_CODE) == 0)
    {
	// Ajout du nouveau client
	int i;

	for (i = 0; i < MAX_CHAN && listeClients[i].port != 0; i++);

	if (strncmp (buf, CODE_CH_addSup, SIZE_CODE) == 0)
	    src->etat = STATE_CL_SUP;
	else
	{
	    src->etat = STATE_CL_SIM;
	}
	commCpy (&listeClients[i], src);
	if (me->etat != STATE_CH_WAT)
	    envoiUDP (me, src, CODE_OK);
	printf ("Nouveau client : %s \n", src->nom);

	// Recherche d'un canal suppléant
	if (me->etat == STATE_CH_ASK)
	{
#if DEBUG == 1
	    printf ("Recherche canal supp'\n");
#endif
	    for (i = 0; i < MAX_CHAN &&
		 listeClients[i].etat != STATE_CL_SUP; i++)
	    {
#if DEBUG == 1
		printf ("(%i) : %i\n", i, listeClients[i].etat);
#endif
	    }
	    if (listeClients[i].etat == STATE_CL_SUP)
	    {
		envoiUDP (me, &listeClients[i], CODE_CL_desSup);
#if DEBUG == 2
		printf ("Demande à %s de me suppléer.\n",
			listeClients[i].nom);
#endif
		me->etat = STATE_CH_SUP_INIT;
	    }
	}
	// Pour tous les messsages, si je suis suppléé
	// je redirige le message vers mon suppléant
	else if (me->etat == STATE_CH_SUP)
	{
	    envoiUDP (src, &me[1], buf);
	}
    }
    // Reception d'un message provenant d'un channel de secours
    // identifiant la suppléance qu'il propose (i.e. : qui il est)
    else if (strncmp (buf, CODE_CH_supID, SIZE_CODE) == 0)
    {
	if (me->etat == STATE_CH_SUP_INIT)
	{
#if DEBUG == 2
	    printf ("Je suis suppléé par : %s \n", src->nom);
#endif
	    // On envoie la liste des clients
	    char listeStr[MAX_ARRAY_COM_STR];

	    memset (listeStr, '\0', sizeof listeStr);
	    strncpy (listeStr, CODE_CH_ListAns, strlen (CODE_CH_ListAns));
	    strncat (listeStr, "\n", strlen ("\n"));
	    commArrayToString (listeClients, listeStr);

	    printf ("Envoi de la liste : %s", listeStr);
	    // Envoi de la liste des clients au suppléant
	    envoiUDP (me, src, listeStr);

	    // Association du suppléant à moi meme en vu de l'observer
	    commCpy (&me[1], src);

	    // Démarrage d'un thread pour surveiller le suppléant.
	    pthread_mutex_lock (&ALIVE_mutex);
	    ALIVE_SHARED = 0;
	    pthread_mutex_unlock (&ALIVE_mutex);

	    pthread_t watcher;

	    if (pthread_create (&watcher, NULL, &watchingThread, me))
	    {
		printf ("Thread error : canal vers surveillant.\n");
	    }

	    me->etat = STATE_CH_SUP;
	}
	else
	{
	    printf ("Erreur : Suppléance inatendue.\n");
	}
    }
    // Reception + Diffusion d'un message provenant de la
    // part d'un client
    else if (strncmp (buf, CODE_CH_msgCli, SIZE_CODE) == 0)
    {
	char msg[MAXLEN];

	memset (msg, '\0', sizeof msg);
	strncpy (msg, CODE_CL_msgCha, SIZE_CODE);
	strncat (msg, src->nom, strlen (src->nom));
	strncat (msg, " : ", strlen (" : "));
	strncat (msg, &buf[SIZE_CODE], strlen (buf) - SIZE_CODE);
	int i;

	for (i = 0; i < MAX_CHAN && listeClients[i].port != 0; i++)
	    if (commCompare (&listeClients[i], src))
		envoiUDP (me, &listeClients[i], msg);
    }
    // Reception d'un signal ALIVE : question
    else if (strncmp (buf, CODE_AliveReq, SIZE_CODE) == 0)
    {
	envoiUDP (me, src, CODE_AliveAns);
    }
    // Reception d'un signal ALIVE : reponse
    else if (strncmp (buf, CODE_AliveAns, SIZE_CODE) == 0)
    {
	pthread_mutex_lock (&ALIVE_mutex);
	ALIVE_SHARED++;
#if DEBUG == 1
	printf ("Valeur alive : %i \n", ALIVE_SHARED);
#endif
	pthread_mutex_unlock (&ALIVE_mutex);
    }
    else if (strncmp (buf, CODE_CH_ListAns, SIZE_CODE) == 0)
    {
	// On réalise une copie compléte des clients
	commStringToArray (&buf[SIZE_CODE + 1], listeClients);
    }
}


void *
canalThread (void *arg)
{
    struct comm *a = arg;

    struct comm can[2];

    struct comm listeClients[MAX_CLI];

    commArrayInit (listeClients);

    switch (a->etat)
    {
	case STATE_INIT:;
	    commCpy (&can[0], &a[0]);
	    commInit (&can[1]);
	    can[0].port = rand () % ESPACE_PORT + PLAGE_CANAL;
	    serveur (can, &canalSub, listeClients);
	    break;

	case STATE_CL_SUP:;
	    // Le client 'me' ouvre un thread (can) 
	    // identification dédié à suppléer 'sup'.

	    // On copie le nom du serveur à surveiller.
	    strncpy (can[0].nom, a[1].nom, strlen (a[1].nom));
	    // On lui ajoute une petite extension pour le
	    // distinguer.
	    strncat (can[0].nom, SUFF_SUP, strlen (SUFF_SUP));

	    // On prend notre adresse et tire au sort un port.
	    strncpy (can[0].padr, a[0].padr, strlen (a[0].padr));
	    can[0].port = rand () % ESPACE_PORT + PLAGE_CANAL;

	    // On s'affecte l'etat 'watcher'.
	    can[0].etat = STATE_CH_WAT;

	    // On conserve l'identité du serveur à surveiller.
	    commCpy (&can[1], &a[1]);
	    serveur (can, &canalSub, listeClients);
	    break;

	default:;
	    printf ("/!\\ Etat invalide ! (canal)\n");
    }

    pthread_exit (NULL);
}

void
clientSub (struct comm *me, struct comm *src, char *buf, void *arg)
{
    (void) arg;

    if ((int) buf == INIT_FLAG)
    {
	printf ("Initialisation du client\n");
	return;
    }

    // Reception de la liste des cannaux + selection
    else if (strncmp (buf, CODE_AN_ListAns, SIZE_CODE) == 0)
    {
	struct comm listeCannaux[MAX_CHAN];
	int i, select = 0;
	long r;
	char entree[MAXLEN];

	printf ("Liste des cannaux disponibles :\n");
	commStringToArray (&buf[SIZE_CODE + 1], listeCannaux);
	for (i = 0; i < MAX_CHAN && listeCannaux[i].port != 0; i++)
	    printf ("[%i] %s\n", i, listeCannaux[i].nom);

	printf ("Entrez un nombre entre 0 et %i \n", i - 1);
	while (i != 1 && (r = read (0, entree, MAXLEN)) > 0)
	{
	    select = (int) strtol (&entree[0], (char **) NULL, 10);
	    if (select <= i && select >= 0)
		break;
	    else
	    {
		printf ("Selection invalide !\n");
		memset (entree, '\0', sizeof entree);
	    }
	}

	printf ("Connexion à %s\n", listeCannaux[select].nom);

	switch (me->etat)
	{
	    case STATE_CL_SIM_INIT:
		envoiUDP (me, &listeCannaux[select], CODE_CH_addCli);
		me->etat = STATE_CL_SIM;
		break;
	    case STATE_CL_SUP_INIT:
		envoiUDP (me, &listeCannaux[select], CODE_CH_addSup);
		me->etat = STATE_CL_SUP;
		break;
	    default:
		printf ("/!\\ Etat invalide ! (client)\n");
	}

	// Demarrage d'un thread pour écrire

	pthread_mutex_lock (&moi[1].mutex);
	commCpy (&moi[1], &listeCannaux[select]);
	pthread_mutex_unlock (&moi[1].mutex);

	struct comm a[2];

	a[0] = *me;
	a[1] = listeCannaux[select];

	pthread_t clavier;

	if (pthread_create (&clavier, NULL, &clientThread, &a))
	{
	    printf ("Impossible de demarrer le canal suppléant.\n");
	    perror ("pthread_create");
	    exit (1);
	}
    }
    // Abonnement effectué auprès du canal
    else if (strncmp (buf, CODE_OK, SIZE_CODE) == 0)
    {
	printf ("Vous êtes %s sur %s.\n", me->nom, src->nom);
    }
    // Exception : l'annuaire n'a pas de cannal inscrit.
    else if (strncmp (buf, CODE_AN_ListEmpty, SIZE_CODE) == 0)
    {
	printf ("Pas de cannal sur cet annuaire. \n");
	exit (0);
    }
    // Reception et affichage d'un message venant d'un canal.
    else if (strncmp (buf, CODE_CL_msgCha, SIZE_CODE) == 0)
    {
	char aff[MAXLEN];

	memset (aff, '\0', sizeof aff);
	strncpy (aff, &buf[SIZE_CODE], strlen (buf) - SIZE_CODE);
	printf ("%s->%s\n", src->nom, aff);
    }
    // Reception CODE designation de suppleance
    else if (strncmp (buf, CODE_CL_desSup, SIZE_CODE) == 0
	     && me->etat == STATE_CL_SUP)
    {
	printf ("Je deviens suppléant pour %s\n", src->nom);

	struct comm a[2];

	a[0] = *me;
	a[1] = *src;

	pthread_t canalSup;

	if (pthread_create (&canalSup, NULL, &canalThread, &a))
	{
	    perror ("pthread_create");
	    exit (1);
	}
    }
    else if (strncmp (buf, CODE_CL_replace, SIZE_CODE) == 0)
    {
	printf ("-->> Message de remplacement%s\n", buf);

	pthread_mutex_lock (&moi[1].mutex);
	commCpy (&moi[1], src);
	pthread_mutex_unlock (&moi[1].mutex);
    }
}

void *
clientThread (void *arg)
{
    struct comm *m = arg;

    if (m->etat == STATE_CL_SIM_INIT || m->etat == STATE_CL_SUP_INIT)
    {
	// Contact de l'annuaire et ouverture du serveur
	m->port = rand () % ESPACE_PORT + PLAGE_CLIENT;
	pthread_mutex_lock (&annuaire.mutex);
	envoiUDP (m, &annuaire, CODE_AN_ListReq);
	pthread_mutex_unlock (&annuaire.mutex);
	serveur (m, &clientSub, NULL);
    }

    // Thread d'entree au clavier
    else if (m->etat == STATE_CL_SIM || m->etat == STATE_CL_SUP)
    {
	long r;
	char entree[MAXLEN];
	char msg[MAXLEN];

	memset (entree, '\0', sizeof entree);
	memset (msg, '\0', sizeof msg);

	while ((r = read (0, entree, MAXLEN)) > 0)
	{
	    strncpy (msg, CODE_CH_msgCli, SIZE_CODE);
	    strncat (msg, entree, strlen (entree) - 1);
	    pthread_mutex_lock (&moi[1].mutex);
	    envoiUDP (&moi[0], &moi[1], msg);
	    pthread_mutex_unlock (&moi[1].mutex);
	    memset (entree, '\0', sizeof entree);
	    memset (msg, '\0', sizeof msg);
	}
    }
    pthread_exit (NULL);
}

void *
watchingThread (void *arg)
{
    struct comm *a = arg;

    while (1)
    {
	envoiUDP (&a[0], &a[1], CODE_AliveReq);
	pthread_mutex_lock (&ALIVE_mutex);
	if (abs (ALIVE_SHARED) >= CHAN_ALIVE_MAX)
	{
	    pthread_mutex_unlock (&ALIVE_mutex);
#if DEBUG == 2
	    printf ("#&@&------CRASH------#&@&!\n");
#endif
	    envoiUDP (&a[0], &a[0], CODE_CH_SPECIAL);
	    pthread_exit (NULL);
	}
	ALIVE_SHARED--;
	pthread_mutex_unlock (&ALIVE_mutex);
	sleep (1);
    }
}
