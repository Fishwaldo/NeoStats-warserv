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


#include <stdio.h>
#include "dl.h"       /* Required for module */
#include "stats.h"    /* Required for bot support */
#include "log.h"      /* Log systems support */
#include "conf.h"

char s_module_bot_name[MAXNICK];
char warroom[CHANLEN];
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

/** Module Info definition 
 * version information about our module
 * This structure is required for your module to load and run on NeoStats
 */
ModuleInfo __module_info = {
	"WarServ",
	"War Card Game Module For NeoStats",
	"1.2",
	__DATE__,
	__TIME__
};

/** printf version information
 * respond to the /VERSION command on IRC with this text
 * This is recommended for your module to load and run on NeoStats
 */
int new_m_version(char *origin, char **av, int ac)
{
	snumeric_cmd(351, origin, "Module WarServ Loaded, Version: %s %s %s",
		__module_info.module_version, __module_info.module_build_date,
		__module_info.module_build_time);
	return 0;
}

/** Module function list
 * A list of IRCd (server) commands that we will respond to
 * e.g. VERSION
 * This table is required for your module to load and run on NeoStats
 * but you do not have to have any functions in it
 */
Functions __module_functions[] = {
	{MSG_VERSION, new_m_version, 1},
#ifdef HAVE_TOKEN_SUP
	{TOK_VERSION, new_m_version, 1},
#endif
	{NULL, NULL, 0}
};

/** Channel message processing
 * What do we do with messages in channels
 * This is required if you want your module to respond to channel messages
 */
int __ChanMessage(char *origin, char **argv, int argc)
{
	char *tsptr;
	User *u;
	char *buf;
	char *bufchal;
	if (argc < 1) {
		return -1;
	}
	if (strcasecmp(argv[0], warroom)) {
		return -1;
	}
	u = finduser(origin);
	if (!u) {
		return -1;
	}
	if (argc == 2) {
		if (!strcasecmp(argv[1], "!rules")) {
			prefmsg(u->nick, s_module_bot_name, "Welcome To War (The Card Game) Ver 1.2 %s", u->nick);
			prefmsg(u->nick, s_module_bot_name, "Written By DeadNotBuried");
			prefmsg(u->nick, s_module_bot_name, " ");
			prefmsg(u->nick, s_module_bot_name, "All cards are Dealt out evenly when the game starts.");
			prefmsg(u->nick, s_module_bot_name, "The Object of the game is to hold ALL the cards.");
			prefmsg(u->nick, s_module_bot_name, "Each Player plays a card from their hand, and the");
			prefmsg(u->nick, s_module_bot_name, "highest card wins all cards played that turn.");
			prefmsg(u->nick, s_module_bot_name, " ");
			prefmsg(u->nick, s_module_bot_name, "If the played cards are equal, there is a War.");
			prefmsg(u->nick, s_module_bot_name, "all played cards stay out, and each player involved");
			prefmsg(u->nick, s_module_bot_name, "in the war plays 3 more cards. the War continues the");
			prefmsg(u->nick, s_module_bot_name, "same way, untill someone wins all the played cards.");
			prefmsg(u->nick, s_module_bot_name, " ");
			prefmsg(u->nick, s_module_bot_name, "If you don't have enough cards to play, your cards are");
			prefmsg(u->nick, s_module_bot_name, "automatically put into the center, and you surrender.");
			prefmsg(u->nick, s_module_bot_name, " ");
			prefmsg(u->nick, s_module_bot_name, "NOTE: Game can't be joined to after play has started.");
		} else if (!strcasecmp(argv[1], "!whelp")) {
			prefmsg(u->nick, s_module_bot_name, "Currently available public WarServ commands");
			prefmsg(u->nick, s_module_bot_name, "===========================================");
			prefmsg(u->nick, s_module_bot_name, "!whelp      -- Displays this help");
			prefmsg(u->nick, s_module_bot_name, "!rules      -- displays war rules");
			prefmsg(u->nick, s_module_bot_name, "!about      -- displays about information");
			prefmsg(u->nick, s_module_bot_name, "!start      -- Starts a new game");
			prefmsg(u->nick, s_module_bot_name, "!stop       -- Stops the current game");
			prefmsg(u->nick, s_module_bot_name, "!players    -- Show current players");
			prefmsg(u->nick, s_module_bot_name, "!turn       -- Show who's turn it is");
			prefmsg(u->nick, s_module_bot_name, "join        -- Joins the current game");
			prefmsg(u->nick, s_module_bot_name, "play # # #  -- Plays the card(s) number");
			prefmsg(u->nick, s_module_bot_name, "!remove     -- Removes you from the current game");
			return 1;
		} else if (!strcasecmp(argv[1], "!about")) {
			prefmsg(u->nick, s_module_bot_name, "Welcome To War (The Card Game) v1.0 by DeadNotBuried");
			return 1;
		} else if (!strcasecmp(argv[1], "!stop")) {
			 if (strcasecmp(currentwargamestatus, "stopped")) {
				 privmsg(warroom, s_module_bot_name, "\0039Stopping Current Game");
				 stopwar(); 
			 }
		} else if (!strcasecmp(argv[1], "!start")) {
			if (!strcasecmp(currentwargamestatus, "stopped")) {
				startcountdowntimer(u->nick);
				return 1;
			} else if (currentwarplayercount < 10) {
				privmsg(argv[0], s_module_bot_name, "\0034A game has already started \0037%s\0034, Type '\2\003Join\2\0034' To Join in.", u->nick);
				return 1;
			} else {
				privmsg(argv[0], s_module_bot_name, "\0034A game has already started \0037%s\0034 and all spots are taken. Please try the next game.", u->nick);
				return 1;
			}
		} else if (!strcasecmp(argv[1], "join") || !strcasecmp(argv[1], "j")) {
			if (!strcasecmp(currentwargamestatus, "starting")) {
				joinwar(u->nick);
				return 1;
			}
		} else if (!strcasecmp(argv[1], "!Remove")) {
			if (!strcasecmp(currentwargamestatus, "starting") || !strcasecmp(currentwargamestatus, "started")) {
				removewar(u->nick);
				return 1;
			}
		} else if (!strcasecmp(argv[1], "!players")) {
			if (!strcasecmp(currentwargamestatus, "started")) {
				privmsg(warroom, s_module_bot_name, "\0039Current Players are\0038 :\0037 %s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
				return 1;
			}
		} else if (!strcasecmp(argv[1], "!turn")) {
			if (!strcasecmp(currentwargamestatus, "started")) {
				if (warinprogress == 1) {
					privmsg(warroom, s_module_bot_name, "\0039The Current Player is \0037%s\0039 holding\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
				} else {
					privmsg(warroom, s_module_bot_name, "\0039The Current Player is \0037%s\0039 currently holding\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
				}
				return 1;
			}
		}
	} else if (argc > 2) {
		if ((!strcasecmp(argv[1], "play") || !strcasecmp(argv[1], "p")) && !strcasecmp(u->nick,wplayernick[currentplayer]) && !strcasecmp(currentwargamestatus, "started")) {
			if (warinprogress == 1) {
				if (argc == 5) {
					playwarcards(argv[2], argv[3], argv[4]);
				}
			} else {
				if (argc == 3) {
					playcard(argv[2]);
				}
			}
			return 1;
		} else if (!strcasecmp(argv[1], "!Remove")) {
			if (!strcasecmp(currentwargamestatus, "starting") || !strcasecmp(currentwargamestatus, "started")) {
				removewarother(u->nick, argv[2]);
				return 1;
			}
		}
	}
	return 1;
}

/** Bot message processing
 * What do we do with messages sent to our bot with /mag
 * This is required if you want your module to respond to /msg
 */
int __BotMessage(char *origin, char **argv, int argc)
{
	User *u;
	char *bufchal;
	u = finduser(origin);
	if (!u) {
		return -1;
	}
	if (strcasecmp(argv[0], s_module_bot_name)) {
		return -1;
	}
	if (argc >= 2) {
		if (!strcasecmp(argv[1], "help")) {
			prefmsg(u->nick, s_module_bot_name, "Currently available commands are all public commands");
			prefmsg(u->nick, s_module_bot_name, "To see currently available public commands");
			prefmsg(u->nick, s_module_bot_name, "type !whelp in the main channel ( %s )", warroom);
			if (UserLevel(u) >= NS_ULEVEL_OPER) {
				prefmsg(u->nick, s_module_bot_name, "\2CHAN <channel>\2 - Swap WarGame Channel to <channel>");
			}
			return 1;
		} else if (!strcasecmp(argv[1], "CHAN") && (UserLevel(u) >= NS_ULEVEL_OPER)) {
			privmsg(warroom, s_module_bot_name, "%s has moved the Game room to %s, Please Go there now to continue the game", u->nick, argv[2]);
			chanalert(s_module_bot_name, "%s moved the game to %s", u->nick, argv[2]);
			spart_cmd(s_module_bot_name, warroom);
			strlcpy(warroom, argv[2], CHANLEN);
			sjoin_cmd(s_module_bot_name, warroom);
			schmode_cmd(s_module_bot_name, warroom, "+o", s_module_bot_name);
			SET_SEGV_INMODULE("WarServ");
			SetConf((void *)warroom, CFGSTR, "WarRoom");
			return NS_SUCCESS;
		} else {
			prefmsg(u->nick, s_module_bot_name, "Invalid Command. /msg %s help for more info", s_module_bot_name);
		}
	}
		
	prefmsg(u->nick, s_module_bot_name, "'/msg %s help' to list commands", s_module_bot_name);
	bufchal = joinbuf(argv, argc, 1);
	chanalert(s_module_bot_name, "\0038Recieved Private Message from\0037 %s\0038 :\003 %s", u->nick, bufchal);
	free(bufchal);
	return 1;
}

/** Online event processing
 * What we do when we first come online
 */
int Online(char **av, int ac)
{
	char *tmp;
	/* Introduce a bot onto the network */
	if (init_bot(s_module_bot_name, "WarServ", me.host, "War Game Service", "+oSq",
		__module_info.module_name) == -1) {
			/* nick was in use */
			return 0;
	}
	srand((unsigned int)time(NULL));
	/* channel to play game in */
	if (GetConf((void *)&tmp, CFGSTR, "WarRoom") <= 0) {
		strlcpy(warroom, "#Games", CHANLEN);
	} else {
		strlcpy(warroom, tmp, CHANLEN);
	}
	chanalert(s_module_bot_name, "Game will start in %s", warroom);
	sjoin_cmd(s_module_bot_name, warroom);
	schmode_cmd(s_module_bot_name, warroom, "+o", s_module_bot_name);
	return 1;
};

/*
 * Nick Change Check
 */
int PlayerNickChange(char **av, int ac) {
	if (currentwarplayercount < 1) {
		return 1;
	}
	for (wpln = 0; wpln < currentwarplayercount; wpln++) {
		if (!strcasecmp(wplayernick[wpln], av[0])) {
			strncpy(wplayernick[wpln], av[1], MAXNICK);
		}
	}
}

/** Module event list
 * What events we will act on
 * This is required if you want your module to respond to events on IRC
 * see modules.txt for a list of all events available
 */
EventFnList __module_events[] = {
	{EVENT_ONLINE, Online},
	{EVENT_NICKCHANGE, PlayerNickChange},
	{NULL, NULL}
};

/** Init module
 * This is required if you need to do initialisation of your module when
 * first loaded
 */
int __ModInit(int modnum, int apiver)
{
	strncpy(s_module_bot_name, "WarServ", MAXNICK);
	stopwar();
	return 1;
}

/** Exit module
 * This is required if you need to do cleanup of your module when it ends
 */
void __ModFini()
{
	del_mod_timer("wartimer");
};

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
	del_mod_timer("wartimer");
	for (wpln = 0; wpln < 10; wpln++) {
		strncpy(wplayernick[wpln], " ", MAXNICK);
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
	strncpy(currentwargamestatus, "Stopped", 10);
	currentplayer= 0;
	return 1;
}

/*
 * Start Game Countdown
 * Starts a timer to allow 30 seconds for people to join the game
*/

int startcountdowntimer(char *nic) {
	strncpy(currentwargamestatus, "starting", 10);
	privmsg(warroom, s_module_bot_name, "\0037A new game of \0034WAR\0037 has been started by %s. Game will start in 30 seconds, type '\2\003Join\2\0037' to play.", nic);
	add_mod_timer("startwar", "wartimer", __module_info.module_name, 30);
	return 1;
}

/*
 * Start Game
 * Initializes variables and starts game
*/

void startwar() {
	del_mod_timer("wartimer");
	if (!strcasecmp(currentwargamestatus, "starting")) {
		if (currentwarplayercount < 1) {
			privmsg(warroom, s_module_bot_name, "\0034No Players joined to current Game, Exiting");
			stopwar();
			return;
		}
		if (currentwarplayercount < 10) {
			joinwar(s_module_bot_name);
		}
		privmsg(warroom, s_module_bot_name, "\0034WAR\00310 is now starting, current players are \0037%s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
		privmsg(warroom, s_module_bot_name, "\0039Shuffling Deck and Dealing Cards");
		currentplayer= 0;
		strncpy(currentwargamestatus, "started", 10);
		wardealcards();
		wstackcardscurrent= 0;
		askplaycard();
	}
	return;
}

/*
 * Join Game
 * adds new player to game during countdown
*/

int joinwar(char *nic) {
	if (currentwarplayercount < 10) {
		for (wpln = 0; wpln < 10; wpln++) {
			if (!strcasecmp(wplayernick[wpln], nic)) {
				return 1;		
			}
		}
		strncpy(wplayernick[currentwarplayercount], nic, MAXNICK);
		currentwarplayercount++;
		privmsg(warroom, s_module_bot_name, "\0038Welcome to \0034WAR \0037%s", nic);
	} else {
		privmsg(warroom, s_module_bot_name, "\0034Sorry all places are filled \0037%s\0034, your welcome to try the next game though", nic);
	}
}

/*
 * Remove Nick ( Not Online )
 * allows removal of a player by anyone if player no longer connected to ircd
*/

int removewarother(char *nic, char *ntr) {
	User *u;
	u = finduser(ntr);
	if (!u) {
		removewar(ntr);
	}
}

/*
 * Remove PLayer
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
		if (!strcasecmp(wplayernick[wpln], nic)) {
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
					strncpy(wplayernick[wplnh], " ", MAXNICK);
					for (wplnht = 0; wplnht < wplayercardstotal[wplnh]; wplnht++) {
						wplayercardsinhand[wplnh][wplnht]= 0;
					}
					wplayercardstotal[wplnh]= 0;
				} else {
					strncpy(wplayernick[wplnh], wplayernick[cpfp], MAXNICK);
					for (wplnht = 0; wplnht < 52; wplnht++) {
						wplayercardsinhand[wplnh][wplnht]= wplayercardsinhand[cpfp][wplnht];
					}
					wplayercardstotal[wplnh]= wplayercardstotal[cpfp];
				}
			}
			currentwarplayercount--;
			privmsg(warroom, s_module_bot_name, "\0037%s \0038Removed from the current game of \0034War", nic);
		}
	}
	if (currentwarplayercount < 2) {
		privmsg(warroom, s_module_bot_name, "\0039Stopping Current Game , \0037%s\0039 wins.", wplayernick[0]);
		stopwar();
	} else if (tfrpacp == 1) {
		if (currentplayer > (currentwarplayercount - 1)) {
			currentplayer= 0;
		}
		if (strcasecmp(currentwargamestatus, "starting")) {
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
		}
		if (nwp > currentwarplayercount) {
			checkwarwinner();
		} else {
			if (!strcasecmp(wplayernick[currentplayer], s_module_bot_name)) {
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
				privmsg(warroom, s_module_bot_name, "\0037%s\0039 you hold\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
			}
		}
	} else {
		if (!strcasecmp(wplayernick[currentplayer], s_module_bot_name)) {
			snprintf(wspas[0], 3, "%d", ((rand() % wplayercardstotal[currentplayer]) + 1));
			playcard(wspas[0]);
		} else {
			privmsg(warroom, s_module_bot_name, "\0037%s\0039 you currently hold\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
		}
	}
}

/*
 * Player War Play Card
*/

int playwarcards(char *cnps1, char *cnps2, char *cnps3) {
	int cnp[3];
	char *tempint;
	cnp[0] = atoi(cnps1);
	cnp[1] = atoi(cnps2);
	cnp[2] = atoi(cnps3);
	for (wpln = 0; wpln < 3; wpln++) {
		if ((cnp[wpln] < 1) || (cnp[wpln] > wplayercardstotal[currentplayer])) {
			return -1;
		}
	}
	if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 13) {
		strncpy(csuitcolour, "\0034", 10);
		strncpy(csuit, "Hearts", 10);
	} else if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 26) {
		strncpy(csuitcolour, "\0037", 10);
		strncpy(csuit, "Diamonds", 10);
	} else if (wplayercardsinhand[currentplayer][(cnp[2] - 1)] < 39) {
		strncpy(csuitcolour, "\00314", 10);
		strncpy(csuit, "Clubs", 10);
	} else {
		strncpy(csuitcolour, "\00315", 10);
		strncpy(csuit, "Spades", 10);
	}
	switch ((wplayercardsinhand[currentplayer][(cnp[2] - 1)] % 13)) {
		case 12:
			strncpy(csuitcard, "Ace", 10);
			break;
		case 11:
			strncpy(csuitcard, "King", 10);
			break;
		case 10:
			strncpy(csuitcard, "Queen", 10);
			break;
		case 9:
			strncpy(csuitcard, "Jack", 10);
			break;
		default:
			snprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp[2] - 1)] % 13) + 2));
			break;
	}
	privmsg(warroom, s_module_bot_name, "\0037%s\0039 played Cards\00311 %d , %d , %d \0039(%s %s of %s \0039)", wplayernick[currentplayer], cnp[0], cnp[1], cnp[2], csuitcolour, csuitcard, csuit);
	for (wpln = 0; wpln < 3; wpln++) {
		cnp[wpln]--;
		wstackcards[wstackcardscurrent]= wplayercardsinhand[currentplayer][cnp[wpln]];
		wstackcardscurrent++;
		wplayerwarcardsplayed[currentplayer][wpln]= wplayercardsinhand[currentplayer][cnp[wpln]];
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
			strncpy(csuitcolour, "\0034", 10);
			strncpy(csuit, "Hearts", 10);
		} else if (wplayercardsinhand[currentplayer][(cnp - 1)] < 26) {
			strncpy(csuitcolour, "\0037", 10);
			strncpy(csuit, "Diamonds", 10);
		} else if (wplayercardsinhand[currentplayer][(cnp - 1)] < 39) {
			strncpy(csuitcolour, "\00314", 10);
			strncpy(csuit, "Clubs", 10);
		} else {
			strncpy(csuitcolour, "\00315", 10);
			strncpy(csuit, "Spades", 10);
		}
		switch ((wplayercardsinhand[currentplayer][(cnp - 1)] % 13)) {
			case 12:
				strncpy(csuitcard, "Ace", 10);
				break;
			case 11:
				strncpy(csuitcard, "King", 10);
				break;
			case 10:
				strncpy(csuitcard, "Queen", 10);
				break;
			case 9:
				strncpy(csuitcard, "Jack", 10);
				break;
			default:
				snprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp - 1)] % 13) + 2));
				break;
		}
		privmsg(warroom, s_module_bot_name, "\0037%s\0039 played Card\00311 %d \0039(%s %s of %s \0039)", wplayernick[currentplayer], cnp, csuitcolour, csuitcard, csuit);
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
						privmsg(warroom, s_module_bot_name, "\0037%s\0039 wins the \0034WAR\0039.", wplayernick[wpln]);
					} else {
						privmsg(warroom, s_module_bot_name, "\0037%s\0039 takes the hand.", wplayernick[wpln]);
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
		privmsg(warroom, s_module_bot_name, "\0034WAR DECLARED");
		for (wpln = 0; wpln < currentwarplayercount; wpln++) {
			if ((wplayercardplayed[wpln] % 13) == hcnp) {
				if (wplayercardstotal[wpln] < 3) {
					privmsg(warroom, s_module_bot_name, "\0037%s\0038 Surrenders\0039 (Insufficient Cards)", wplayernick[wpln]);
					hcnpt--;
					removewar(wplayernick[wpln]);
					if (currentwarplayercount < 2) {
						wpln= currentwarplayercount;
						return 1;
					}
				} else {
					wplayeratwar[wpln]= 1;
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
			for (wplnh = 0; wplnh < 3; wplnh++) {
				wplayerwarcardsplayed[wpln][wplnh]= 0;
			}
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
