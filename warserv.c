/* WarServ War Card Game Bot - NeoStats Addon Module
** Copyright (c) 2004 DeadNotBuried
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
*/

#include "neostats.h"    /* Required for bot support */

Bot *ws_bot;

char warroom[MAXCHANLEN];
char currentwargamestatus[10];
int currentwarplayercount;
char wplayernick[10][MAXNICK];
int wplayercardstotal[10];
int wplayercardsinhand[10][52];
int wplayercardplayed[10];
int warinprogress;
int wplayeratwar[10];
int wplayerwarcardsplayed[10][3];
int wplayerwarcarddisplayed[10];
int wstackcards[52];
int wstackcardscurrent;
int currentplayer;
int wpln;
int wplnh;
char csuit[10];
char csuitcolour[10];
char csuitcard[10];

int startwar(void);
int PlayerNickChange (CmdParams* cmdparams);
int ChanMessage (CmdParams* cmdparams);
int BotMessage (CmdParams* cmdparams);

/** Copyright info */
const char *ws_copyright[] = {
	"Copyright (c) 2004 DeadNotBuried",
	"Portions Copyright (c) 1999-2004, NeoStats",
	NULL
};

/*
 * Module Info definition 
*/
ModuleInfo module_info = {
	"WarServ",
	"War Card Game Module For NeoStats",
	ws_copyright,
	NULL,
	NEOSTATS_VERSION,
	"3.0",
	__DATE__,
	__TIME__,
	0,
	0,
};

/*
 * Module event list
*/
ModuleEvent module_events[] = {
	{EVENT_NICK, PlayerNickChange},
	{EVENT_PRIVATE, BotMessage},
	{EVENT_CPRIVATE, ChanMessage},		
	{EVENT_NULL, NULL}
};

/*
 * Channel message processing
*/
int ChanMessage (CmdParams* cmdparams)
{
	static char line[512];
	int argc;
	char **argv;

	strlcpy (line, cmdparams->param, 512);
	argc = split_buf (line, &argv, 0);

	if (argc < 1) {
		ns_free (argv);
		return -1;
	}
	if (ircstrcasecmp (cmdparams->channel->name, warroom)) {
		ns_free (argv);
		return -1;
	}
	if (argc == 1) {
		if (!ircstrcasecmp (argv[0], "!rules")) {
			irc_prefmsg (ws_bot, cmdparams->source, "Welcome To War (The Card Game) Ver 1.2 %s", cmdparams->source->name);
			irc_prefmsg (ws_bot, cmdparams->source, "Written By DeadNotBuried");
			irc_prefmsg (ws_bot, cmdparams->source, " ");
			irc_prefmsg (ws_bot, cmdparams->source, "All cards are Dealt out evenly when the game starts.");
			irc_prefmsg (ws_bot, cmdparams->source, "The Object of the game is to hold ALL the cards.");
			irc_prefmsg (ws_bot, cmdparams->source, "Each Player plays a card from their hand, and the");
			irc_prefmsg (ws_bot, cmdparams->source, "highest card wins all cards played that turn.");
			irc_prefmsg (ws_bot, cmdparams->source, " ");
			irc_prefmsg (ws_bot, cmdparams->source, "If the played cards are equal, there is a War.");
			irc_prefmsg (ws_bot, cmdparams->source, "all played cards stay out, and each player involved");
			irc_prefmsg (ws_bot, cmdparams->source, "in the war plays 3 more cards. the War continues the");
			irc_prefmsg (ws_bot, cmdparams->source, "same way, untill someone wins all the played cards.");
			irc_prefmsg (ws_bot, cmdparams->source, " ");
			irc_prefmsg (ws_bot, cmdparams->source, "If you don't have enough cards to play, your cards are");
			irc_prefmsg (ws_bot, cmdparams->source, "automatically put into the center, and you surrender.");
			irc_prefmsg (ws_bot, cmdparams->source, " ");
			irc_prefmsg (ws_bot, cmdparams->source, "NOTE: Game can't be joined to after play has started.");
		} else if (!ircstrcasecmp (argv[0], "!whelp")) {
			irc_prefmsg (ws_bot, cmdparams->source, "Currently available public WarServ commands");
			irc_prefmsg (ws_bot, cmdparams->source, "===========================================");
			irc_prefmsg (ws_bot, cmdparams->source, "!whelp      -- Displays this help");
			irc_prefmsg (ws_bot, cmdparams->source, "!rules      -- displays war rules");
			irc_prefmsg (ws_bot, cmdparams->source, "!about      -- displays about information");
			irc_prefmsg (ws_bot, cmdparams->source, "!start      -- Starts a new game");
			irc_prefmsg (ws_bot, cmdparams->source, "!stop       -- Stops the current game");
			irc_prefmsg (ws_bot, cmdparams->source, "!players    -- Show current players");
			irc_prefmsg (ws_bot, cmdparams->source, "!turn       -- Show who's turn it is");
			irc_prefmsg (ws_bot, cmdparams->source, "join        -- Joins the current game");
			irc_prefmsg (ws_bot, cmdparams->source, "play # # #  -- Plays the card(s) number");
			irc_prefmsg (ws_bot, cmdparams->source, "!remove     -- Removes you from the current game");
			ns_free (argv);
			return 1;
		} else if (!ircstrcasecmp (argv[0], "!about")) {
			irc_prefmsg (ws_bot, cmdparams->source, "Welcome To War (The Card Game) v1.0 by DeadNotBuried");
			ns_free (argv);
			return 1;
		} else if (!ircstrcasecmp (argv[0], "!stop")) {
			 if (ircstrcasecmp (currentwargamestatus, "stopped")) {
				 irc_chanprivmsg (ws_bot, warroom, "\0039Stopping Current Game");
				 stopwar(); 
			 }
		} else if (!ircstrcasecmp (argv[0], "!start")) {
			if (!ircstrcasecmp (currentwargamestatus, "stopped")) {
				startcountdowntimer(cmdparams->source->name);
				ns_free (argv);
				return 1;
			} else if (currentwarplayercount < 10) {
				irc_privmsg (ws_bot, cmdparams->source, "\0034A game has already started \0037%s\0034, Type '\2\003Join\2\0034' To Join in.", cmdparams->source->name);
				ns_free (argv);
				return 1;
			} else {
				irc_privmsg (ws_bot, cmdparams->source, "\0034A game has already started \0037%s\0034 and all spots are taken. Please try the next game.", cmdparams->source->name);
				ns_free (argv);
				return 1;
			}
		} else if (!ircstrcasecmp (argv[0], "join") || !ircstrcasecmp (argv[0], "j")) {
			if (!ircstrcasecmp (currentwargamestatus, "starting")) {
				joinwar(cmdparams->source->name);
				ns_free (argv);
				return 1;
			}
		} else if (!ircstrcasecmp (argv[0], "!Remove")) {
			if (!ircstrcasecmp (currentwargamestatus, "starting") || !ircstrcasecmp (currentwargamestatus, "started")) {
				removewar(cmdparams->source->name);
				ns_free (argv);
				return 1;
			}
		} else if (!ircstrcasecmp (argv[0], "!players")) {
			if (!ircstrcasecmp (currentwargamestatus, "started")) {
				irc_chanprivmsg (ws_bot, warroom, "\0039Current Players are\0038 :\0037 %s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
				ns_free (argv);
				return 1;
			}
		} else if (!ircstrcasecmp (argv[0], "!turn")) {
			if (!ircstrcasecmp (currentwargamestatus, "started")) {
				if (warinprogress == 1) {
					irc_chanprivmsg (ws_bot, warroom, "\0039The Current Player is \0037%s\0039 holding\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
				} else {
					irc_chanprivmsg (ws_bot, warroom, "\0039The Current Player is \0037%s\0039 currently holding\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
				}
				ns_free (argv);
				return 1;
			}
		}
	} else if (argc > 1) {
		if ((!ircstrcasecmp (argv[0], "play") || !ircstrcasecmp (argv[0], "p")) && !ircstrcasecmp (cmdparams->source->name,wplayernick[currentplayer]) && !ircstrcasecmp (currentwargamestatus, "started")) {
			if (warinprogress == 1) {
				if (argc == 4) {
					playwarcards(argv[1], argv[2], argv[3]);
				}
			} else {
				if (argc == 2) {
					playcard(argv[1]);
				}
			}
			ns_free (argv);
			return 1;
		} else if (!ircstrcasecmp (argv[0], "!Remove")) {
			if (!ircstrcasecmp (currentwargamestatus, "starting") || !ircstrcasecmp (currentwargamestatus, "started")) {
				removewarother(cmdparams->source->name, argv[1]);
				ns_free (argv);
			}
		}
	}
	ns_free (argv);
	return 1;
}

/*
 * private message processing
*/
int BotMessage (CmdParams* cmdparams)
{
	char *bufchal;
	static char line[512];
	int argc;
	char **argv;

	strlcpy (line, cmdparams->param, 512);
	argc = split_buf (line, &argv, 0);

	if (argc >= 1) {
		if (!ircstrcasecmp (argv[0], "help")) {
			irc_prefmsg (ws_bot, cmdparams->source, "Currently available commands are all public commands");
			irc_prefmsg (ws_bot, cmdparams->source, "To see currently available public commands");
			irc_prefmsg (ws_bot, cmdparams->source, "type !whelp in the main channel ( %s )", warroom);
			if (UserLevel(cmdparams->source) >= NS_ULEVEL_OPER) {
				irc_prefmsg (ws_bot, cmdparams->source, "\2CHAN <channel>\2 - Swap WarGame Channel to <channel>");
			}
			ns_free (argv);
			return 1;
		} else if (!ircstrcasecmp (argv[0], "CHAN") && (UserLevel(cmdparams->source) >= NS_ULEVEL_OPER)) {
			irc_chanprivmsg (ws_bot, warroom, "%s has moved the Game room to %s, Please Go there now to continue the game", cmdparams->source->name, argv[1]);
			irc_chanalert (ws_bot, "%s moved the game to %s", cmdparams->source->name, argv[1]);
			irc_part( ws_bot, warroom, NULL );
			strlcpy (warroom, argv[1], MAXCHANLEN);
			irc_join (ws_bot, warroom, NULL);
			irc_cmode (ws_bot, warroom, "+o", ws_bot->name);
			DBAStoreConfigStr ("WarRoom", warroom, MAXCHANLEN);
			ns_free (argv);
			return NS_SUCCESS;
		} else {
			irc_prefmsg (ws_bot, cmdparams->source, "Invalid Command. /msg %s help for more info", ws_bot->name);
		}
	}
		
	irc_prefmsg (ws_bot, cmdparams->source, "'/msg %s help' to list commands", ws_bot->name);
	bufchal = joinbuf(argv, argc, 0);
	irc_chanalert (ws_bot, "\0038Recieved Private Message from\0037 %s\0038 :\003 %s", cmdparams->source->name, bufchal);
	ns_free(bufchal);
	ns_free (argv);
	return 1;
}

/** BotInfo */
static BotInfo ws_botinfo = 
{
	"WarServ", 
	"WarServ1", 
	"WS", 
	BOT_COMMON_HOST, 
	"War Game Service",
	BOT_FLAG_SERVICEBOT,
	NULL, 
	NULL,
};

/*
 * Online event processing
*/
int ModSynch (void)
{
	/* Introduce a bot onto the network */
	ws_bot = AddBot (&ws_botinfo);	
	if (!ws_bot) {
		return NS_FAILURE;
	}
	srand((unsigned int)me.now);
	/* channel to play game in */
	if (DBAFetchConfigStr ("WarRoom", warroom, MAXCHANLEN) != NS_SUCCESS) {
		strlcpy (warroom, "#Games", MAXCHANLEN);
	}
	irc_chanalert (ws_bot, "Game will start in %s", warroom);
	irc_join (ws_bot, warroom, NULL);
	irc_cmode (ws_bot, warroom, "+o", ws_bot->name);
	return NS_SUCCESS;
};

/*
 * Nick Change Check
*/
int PlayerNickChange (CmdParams* cmdparams)
{
	if (currentwarplayercount < 1) {
		return NS_SUCCESS;
	}
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		if (!ircstrcasecmp (wplayernick[wpln], cmdparams->param)) {
			strlcpy (wplayernick[wpln], cmdparams->source->name, MAXNICK);
		}
	}
	return NS_SUCCESS;
}

/*
 * Init module
*/
int ModInit (Module *mod_ptr)
{
	stopwar();
	return NS_SUCCESS;
}

/*
 * Exit module
*/
void ModFini (void)
{
	DelTimer ("startwar");
}

/*
 * Deal Cards
*/

int wardealcards() {
	int cd;
	int cnd;
	int cndn;
	int ptdt = 0;
	for (wpln = 0; wpln < 52; wpln++) {
		wstackcards[wpln]= wpln;
	}
	for (cd = 52; cd > 0; cd--) {
		cnd= rand() % cd;
		cndn= wstackcards[cnd];
		for (wpln = cnd; wpln < (cd - 1); wpln++) {
			wstackcards[wpln]= wstackcards[(wpln + 1)];
		}
		wstackcards[(cd - 1)]= 0;
		wplayercardsinhand[ptdt][wplayercardstotal[ptdt]]= cndn;
		wplayercardstotal[ptdt]++;
		ptdt++;
		if (ptdt == currentwarplayercount) {
			ptdt= 0;
		}
	}
}

/*
 * Shuffle Players Cards
*/

int playershufflecards() {
	int tcs;
	int tcsp;
	int tcsps;
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		for (wplnh = 0; wplnh < wplayercardstotal[wpln]; wplnh++) {
			tcsps= 0;
			tcsp= rand() % wplayercardstotal[wpln];
			if (tcsp == wplnh) {
				tcsps= 1;
			}
			if (tcsps == 0) {
				tcs= wplayercardsinhand[wpln][tcsp];
				wplayercardsinhand[wpln][tcsp] = wplayercardsinhand[wpln][wplnh];
				wplayercardsinhand[wpln][wplnh] = tcs;
			}
		}
	}
}

/*
 * Stop Game
 * resets everything to default as if no game running
*/

int stopwar() {
	DelTimer ("startwar");
	for (wpln = 0; wpln < 10; wpln++) {
		strlcpy (wplayernick[wpln], " ", MAXNICK);
		wplayercardstotal[wpln]= 0;
		wplayercardplayed[wpln]= 0;
		wplayeratwar[wpln]= 0;
		wplayerwarcarddisplayed[wpln]= 0;
		for (wplnh = 0; wplnh < 52; wplnh++) {
			wplayercardsinhand[wpln][wplnh]= 0;
		}
		for (wplnh = 0; wplnh < 3; wplnh++) {
			wplayerwarcardsplayed[wpln][wplnh]= 0;
		}
	}
	for (wpln = 0; wpln < 52; wpln++) {
		wstackcards[wpln]= 0;
	}
	warinprogress= 0;
	wstackcardscurrent= 0;
	currentwarplayercount = 0;
	strlcpy (currentwargamestatus, "Stopped", 10);
	currentplayer= 0;
	return 1;
}

/*
 * Start Game Countdown
 * Starts a timer to allow 30 seconds for people to join the game
*/

int startcountdowntimer(char *nic) {
	strlcpy (currentwargamestatus, "starting", 10);
	irc_chanprivmsg (ws_bot, warroom, "\0037A new game of \0034WAR\0037 has been started by %s. Game will start in 30 seconds, type '\2\003Join\2\0037' to play.", nic);
	AddTimer (TIMER_TYPE_INTERVAL, startwar, "startwar", 30);
	return 1;
}

/*
 * Start Game
 * Initializes variables and starts game
*/

int startwar(void) 
{
	DelTimer ("startwar");
	if (!ircstrcasecmp (currentwargamestatus, "starting")) {
		if (currentwarplayercount < 1) {
			irc_chanprivmsg (ws_bot, warroom, "\0034No Players joined to current Game, Exiting");
			stopwar();
			return 0;
		}
		if (currentwarplayercount < 10) {
			joinwar(ws_bot->name);
		}
		irc_chanprivmsg (ws_bot, warroom, "\0034WAR\00310 is now starting, current players are \0037%s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
		irc_chanprivmsg (ws_bot, warroom, "\0039Shuffling Deck and Dealing Cards");
		currentplayer= 0;
		strlcpy (currentwargamestatus, "started", 10);
		wardealcards();
		wstackcardscurrent= 0;
		askplaycard();
	}
	return 0;
}

/*
 * Join Game
 * adds new player to game during countdown
*/

int joinwar(char *nic) {
	if (currentwarplayercount < 10) {
		for (wpln = 0; wpln < 10; wpln++) {
			if (!ircstrcasecmp (wplayernick[wpln], nic)) {
				return 1;		
			}
		}
		strlcpy (wplayernick[currentwarplayercount], nic, MAXNICK);
		currentwarplayercount++;
		irc_chanprivmsg (ws_bot, warroom, "\0038Welcome to \0034WAR \0037%s", nic);
	} else {
		irc_chanprivmsg (ws_bot, warroom, "\0034Sorry all places are filled \0037%s\0034, your welcome to try the next game though", nic);
	}
}

/*
 * Remove Nick ( Not Online )
 * allows removal of a player by anyone if player no longer connected to ircd
*/

int removewarother(char *nic, char *ntr) {
	Client *u;
	u = FindUser (ntr);
	if (!u) {
		removewar(ntr);
	}
}

/*
 * Remove Player
 * removes player from game and adds any cards they have to the stack
*/

int removewar(char *nic) {
	int tfrpacp;
	int tfrpacpn;
	int wplnht;
	int cpfp;
	tfrpacp= 0;
	tfrpacpn= 0;
	if (currentwarplayercount < 1) {
		return 1;
	}
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		if (!ircstrcasecmp (wplayernick[wpln], nic)) {
			irc_chanprivmsg (ws_bot, warroom, "\0037%s \0038Removed from the current game of \0034War", nic);
			if (wpln == currentplayer) {
				tfrpacp= 1;
			} else if (wpln < currentplayer) {
				tfrpacp= 2;
				tfrpacpn= wpln;
			}
			for (wplnht = 0; wplnht < wplayercardstotal[wpln]; wplnht++) {
				wstackcards[wstackcardscurrent]= wplayercardsinhand[wpln][wplnht];
				wstackcardscurrent++;
			}
			for (wplnh = wpln; wplnh < currentwarplayercount; wplnh++) {
				cpfp= (wplnh + 1);
				if (cpfp == currentwarplayercount) {
					strlcpy (wplayernick[wplnh], " ", MAXNICK);
					for (wplnht = 0; wplnht < wplayercardstotal[wplnh]; wplnht++) {
						wplayercardsinhand[wplnh][wplnht]= 0;
					}
					wplayercardstotal[wplnh]= 0;
				} else {
					strlcpy (wplayernick[wplnh], wplayernick[cpfp], MAXNICK);
					for (wplnht = 0; wplnht < 52; wplnht++) {
						wplayercardsinhand[wplnh][wplnht]= wplayercardsinhand[cpfp][wplnht];
					}
					wplayercardstotal[wplnh]= wplayercardstotal[cpfp];
				}
			}
			currentwarplayercount--;
		}
	}
	if (currentwarplayercount < 2) {
		irc_chanprivmsg (ws_bot, warroom, "\0039Stopping Current Game , \0037%s\0039 wins.", wplayernick[0]);
		stopwar();
	} else if (tfrpacp == 1) {
		if (currentplayer > (currentwarplayercount - 1)) {
			currentplayer= 0;
		}
		if (ircstrcasecmp (currentwargamestatus, "starting")) {
			askplaycard();
		}
	} else if (tfrpacp == 2) {
		if (currentplayer > tfrpacpn) {
			currentplayer--;
		}
	}
}

/*
 * asks each player for card to play
 * or check war results if in war
 * or removes player if no cards left
*/

int askplaycard() {
	int trn;
	int wspa[5];
	char wspas[5][3];
	int nwp = (currentwarplayercount + 1);
	if (wplayercardstotal[currentplayer] == 0) {
		removewar(wplayernick[currentplayer]);
		return 1;
	}
	if (wplayercardstotal[currentplayer] == 52) {
		currentplayer++;
		removewar(wplayernick[currentplayer]);
		return 1;
	}
	if (warinprogress == 1) {
		if (currentplayer < currentwarplayercount) {
			for (wpln = currentplayer; wpln < currentwarplayercount; wpln++) {
				if (wplayeratwar[wpln] == 1) {
					nwp= wpln;
					wpln= currentwarplayercount;
				}
			}
			currentplayer = nwp;
		}
		if (nwp > currentwarplayercount) {
			checkwarwinner();
		} else {
			if (!ircstrcasecmp (wplayernick[currentplayer], ws_bot->name)) {
				trn= wplayercardstotal[currentplayer];
				wspa[2]= ((rand() % trn) + 1);
				trn--;
				wspa[3]= ((rand() % trn) + 1);
				if (wspa[3] == wspa[2]) {
					wspa[3]++;
				}
				trn--;
				wspa[4]= ((rand() % trn) + 1);
				if ((wspa[4] == wspa[2]) || (wspa[4] == wspa[3])) {
					wspa[4]++;
				}
				if ((wspa[4] == wspa[2]) || (wspa[4] == wspa[3])) {
					wspa[4]++;
				}
				for (wpln = 2; wpln < 5; wpln++) {
					snprintf(wspas[wpln], 3, "%d", wspa[wpln]);
				}
				playwarcards(wspas[2], wspas[3], wspas[4]);
			} else {
				irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 you hold\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
			}
		}
	} else {
		if (!ircstrcasecmp (wplayernick[currentplayer], ws_bot->name)) {
			snprintf(wspas[0], 3, "%d", ((rand() % wplayercardstotal[currentplayer]) + 1));
			playcard(wspas[0]);
		} else {
			irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 you currently hold\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
		}
	}
}

/*
 * Player War Play Card
*/

int playwarcards(char *cnps1, char *cnps2, char *cnps3) {
	int cnp[3];
	cnp[0] = atoi(cnps1);
	cnp[1] = atoi(cnps2);
	cnp[2] = atoi(cnps3);
	for (wpln = 0; wpln < 3; wpln++) {
		if ((cnp[wpln] < 1) || (cnp[wpln] > wplayercardstotal[currentplayer])) {
			return -1;
		}
	}
	if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 13) {
		strlcpy (csuitcolour, "\0034", 10);
		strlcpy (csuit, "Hearts", 10);
	} else if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 26) {
		strlcpy (csuitcolour, "\0037", 10);
		strlcpy (csuit, "Diamonds", 10);
	} else if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 39) {
		strlcpy (csuitcolour, "\00314", 10);
		strlcpy (csuit, "Clubs", 10);
	} else {
		strlcpy (csuitcolour, "\00315", 10);
		strlcpy (csuit, "Spades", 10);
	}
	switch ((wplayercardsinhand[currentplayer][(cnp[2] - 1)] % 13)) {
		case 12:
			strlcpy (csuitcard, "Ace", 10);
			break;
		case 11:
			strlcpy (csuitcard, "King", 10);
			break;
		case 10:
			strlcpy (csuitcard, "Queen", 10);
			break;
		case 9:
			strlcpy (csuitcard, "Jack", 10);
			break;
		default:
			snprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp[2] - 1)] % 13) + 2));
			break;
	}
	irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 played Cards\00311 %d , %d , %d \0039(%s %s of %s \0039)", wplayernick[currentplayer], cnp[0], cnp[1], cnp[2], csuitcolour, csuitcard, csuit);
	for (wpln = 0; wpln < 3; wpln++) {
		cnp[wpln]--;
		wstackcards[wstackcardscurrent]= wplayercardsinhand[currentplayer][cnp[wpln]];
		wstackcardscurrent++;
		wplayerwarcardsplayed[currentplayer][wpln]= wplayercardsinhand[currentplayer][cnp[wpln]];
	}
	if (cnp[2] > cnp[0])
		cnp[2]--;
	if (cnp[2] > cnp[1])
		cnp[2]--;
	if (cnp[1] > cnp[0])
		cnp[1]--;
	for (wpln = 0; wpln < 3; wpln++) {
		wplayercardstotal[currentplayer]--;
		for (wplnh = cnp[wpln]; wplnh < wplayercardstotal[currentplayer]; wplnh++) {
			wplayercardsinhand[currentplayer][wplnh]= wplayercardsinhand[currentplayer][(wplnh +1)];
		}
		wplayercardsinhand[currentplayer][wplayercardstotal[currentplayer]]= 0;
	}
	currentplayer++;
	if (currentplayer < currentwarplayercount) {
		askplaycard();
	} else {
		checkwarwinner();
	}
}

/*
 * Player Plays Card
*/

int playcard(char *cnps) {
	int cnp;
	cnp = atoi(cnps);
	if ((cnp > 0) && (cnp < (wplayercardstotal[currentplayer] + 1))){
		if (wplayercardsinhand[currentplayer][(cnp - 1)] < 13) {
			strlcpy (csuitcolour, "\0034", 10);
			strlcpy (csuit, "Hearts", 10);
		} else if (wplayercardsinhand[currentplayer][(cnp - 1)] < 26) {
			strlcpy (csuitcolour, "\0037", 10);
			strlcpy (csuit, "Diamonds", 10);
		} else if (wplayercardsinhand[currentplayer][(cnp - 1)] < 39) {
			strlcpy (csuitcolour, "\00314", 10);
			strlcpy (csuit, "Clubs", 10);
		} else {
			strlcpy (csuitcolour, "\00315", 10);
			strlcpy (csuit, "Spades", 10);
		}
		switch ((wplayercardsinhand[currentplayer][(cnp - 1)] % 13)) {
			case 12:
				strlcpy (csuitcard, "Ace", 10);
				break;
			case 11:
				strlcpy (csuitcard, "King", 10);
				break;
			case 10:
				strlcpy (csuitcard, "Queen", 10);
				break;
			case 9:
				strlcpy (csuitcard, "Jack", 10);
				break;
			default:
				snprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp - 1)] % 13) + 2));
				break;
		}
		irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 played Card\00311 %d \0039(%s %s of %s \0039)", wplayernick[currentplayer], cnp, csuitcolour, csuitcard, csuit);
		cnp--;
		wstackcards[wstackcardscurrent]= wplayercardsinhand[currentplayer][cnp];
		wstackcardscurrent++;
		wplayercardplayed[currentplayer]= wplayercardsinhand[currentplayer][cnp];
		wplayercardstotal[currentplayer]--;
		for (wpln = cnp; wpln < wplayercardstotal[currentplayer]; wpln++) {
			wplayercardsinhand[currentplayer][wpln]= wplayercardsinhand[currentplayer][(wpln +1)];
		}
		wplayercardsinhand[currentplayer][wplayercardstotal[currentplayer]]= 0;
		currentplayer++;
		if (currentplayer < currentwarplayercount) {
			askplaycard();
		} else {
			checkhandwinner();
		}
	}
}

/*
 * Checks standard hand for a winner (not a war hand)
*/

int checkhandwinner() {
	int hcnp = 0;
	int hcnpt = 0;
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		if (warinprogress == wplayeratwar[wpln]) {
			if ((wplayercardplayed[wpln] % 13) > hcnp) {
				hcnp = (wplayercardplayed[wpln] % 13);
				hcnpt = 1;
			} else if ((wplayercardplayed[wpln] % 13) == hcnp) {
				hcnpt++;
			}
		}
	}
	if (hcnpt == 1) {
		for (wpln = 0; wpln < currentwarplayercount; wpln++) {
			if (warinprogress == wplayeratwar[wpln]) {
				if ((wplayercardplayed[wpln] % 13) == hcnp) {
					if (warinprogress == 1) {
						irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 wins the \0034WAR\0039.", wplayernick[wpln]);
					} else {
						irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 takes the hand.", wplayernick[wpln]);
					}
					for (wplnh = 0; wplnh < wstackcardscurrent; wplnh++) {
						wplayercardsinhand[wpln][wplayercardstotal[wpln]]= wstackcards[wplnh];
						wplayercardstotal[wpln]++;
					}
					clearstack();
					wpln= currentwarplayercount;
				}
			}
		}
		for (wpln = 0; wpln < currentwarplayercount; wpln++) {
			wplayeratwar[wpln]= 0;
		}
		warinprogress= 0;
	} else {
		irc_chanprivmsg (ws_bot, warroom, "\0034WAR DECLARED");
		for (wpln = 0; wpln < currentwarplayercount; wpln++) {
			if (warinprogress == wplayeratwar[wpln]) {
				wplayeratwar[wpln] = 0;
				if ((wplayercardplayed[wpln] % 13) == hcnp) {
					wplayeratwar[wpln]= 1;
					if (wplayercardstotal[wpln] < 3) {
						irc_chanprivmsg (ws_bot, warroom, "\0037%s\0038 Surrenders\0039 (Insufficient Cards)", wplayernick[wpln]);
						hcnpt--;
						removewar(wplayernick[wpln]);
						if (currentwarplayercount < 2) {
							wpln= currentwarplayercount;
							return 1;
						}
					}
				}
			}
		}
		warinprogress= 1;
	}
	currentplayer= 0;
	playershufflecards();
	askplaycard();
}

/*
 * copy war hand to normal for checking
*/

int checkwarwinner() {
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		if (wplayeratwar[wpln] == 1) {
			wplayercardplayed[wpln]= wplayerwarcardsplayed[wpln][2];
		} else {
			wplayercardplayed[wpln]= 0;
		}
		for (wplnh = 0; wplnh < 3; wplnh++) {
			wplayerwarcardsplayed[wpln][wplnh]= 0;
		}
	}
	checkhandwinner();
}

/*
 * clears the stack
*/

int clearstack() {
	for (wpln = 0; wpln < 52; wpln++) {
		wstackcards[wpln]= 0;
	}
	wstackcardscurrent= 0;
}

#ifdef WIN32 /* temp */

int main (int argc, char **argv)
{
	return 0;
}
#endif
