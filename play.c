/* WarServ - War Card Game Service - NeoStats Addon Module
** Copyright (c) 2004-2005 DeadNotBuried
** Portions Copyright (c) 1999-2005, NeoStats
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
** WarServ CVS Identification
** $Id$
*/

#include "neostats.h"    /* Required for bot support */
#include "warserv.h"

/*
 * Stop Game
 * resets everything to default as if no game running
*/

void stopwar() {
	if (currentwargamestatus == WS_GAME_STARTING) {
		DelTimer ("startwar");
	}
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
	currentwargamestatus = WS_GAME_STOPPED;
	currentplayer= 0;
	return;
}

/*
 * Start Game Countdown
 * Starts a timer to allow 30 seconds for people to join the game
*/

void startcountdowntimer(char *nic) {
	currentwargamestatus = WS_GAME_STARTING;
	irc_chanprivmsg (ws_bot, warroom, "\0037A new game of \0034WAR\0037 has been started by %s. Game will start in 30 seconds, type '\2\003Join\2\0037' to play.", nic);
	AddTimer (TIMER_TYPE_COUNTDOWN, startwar, "startwar", 30);
	return;
}

/*
 * Start Game
 * Initializes variables and starts game
*/

int startwar(void) 
{
  /*	DelTimer ("startwar"); */
	if (currentwargamestatus == WS_GAME_STARTING) {
		if (currentwarplayercount < 1) {
			irc_chanprivmsg (ws_bot, warroom, "\0034No Players joined to current Game, Exiting");
			currentwargamestatus = WS_GAME_STOPPING;
			stopwar();
			return;
		}
		if (currentwarplayercount < 10) {
			joinwar(ws_bot->name);
		}
		irc_chanprivmsg (ws_bot, warroom, "\0034WAR\00310 is now starting, current players are \0037%s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
		irc_chanprivmsg (ws_bot, warroom, "\0039Shuffling Deck and Dealing Cards");
		currentplayer= 0;
		currentwargamestatus = WS_GAME_PLAYING;
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

void joinwar(char *nic) {
	if (currentwarplayercount < 10) {
		for (wpln = 0; wpln < 10; wpln++) {
			if (!ircstrcasecmp (wplayernick[wpln], nic)) {
				return;
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
 * Remove Player
 * removes player from game and adds any cards they have to the stack
*/

void removewar(char *nic) {
	int tfrpacp;
	int tfrpacpn;
	int wplnht;
	int cpfp;
	tfrpacp= 0;
	tfrpacpn= 0;
	if (currentwarplayercount < 1) {
		return;
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
		if (currentwargamestatus == WS_GAME_STARTING) {
			askplaycard();
		}
	} else if (tfrpacp == 2) {
		if (currentplayer > tfrpacpn) {
			currentplayer--;
		}
	}
}

/*
 * Deal Cards
*/
void wardealcards() {
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

void playershufflecards() {
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
 * asks each player for card to play
 * or check war results if in war
 * or removes player if no cards left
*/
void askplaycard() {
	int trn;
	int wspa[5];
	char wspas[5][3];
	int nwp = (currentwarplayercount + 1);
	if (wplayercardstotal[currentplayer] == 0) {
		removewar(wplayernick[currentplayer]);
	}
	if (wplayercardstotal[currentplayer] == 52) {
		currentplayer++;
		removewar(wplayernick[currentplayer]);
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
					ircsnprintf(wspas[wpln], 3, "%d", wspa[wpln]);				}
				playwarcards(wspas[2], wspas[3], wspas[4]);
			} else {
				irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 you hold\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
			}
		}
	} else {
		if (!ircstrcasecmp (wplayernick[currentplayer], ws_bot->name)) {
			ircsnprintf(wspas[0], 3, "%d", ((rand() % wplayercardstotal[currentplayer]) + 1));
			playcard(wspas[0]);
		} else {
			irc_chanprivmsg (ws_bot, warroom, "\0037%s\0039 you currently hold\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
		}
	}
}

/*
 * Player War Play Card
*/
void playwarcards(char *cnps1, char *cnps2, char *cnps3) {
	int cnp[3];
	cnp[0] = atoi(cnps1);
	cnp[1] = atoi(cnps2);
	cnp[2] = atoi(cnps3);
	for (wpln = 0; wpln < 3; wpln++) {
		if ((cnp[wpln] < 1) || (cnp[wpln] > wplayercardstotal[currentplayer])) {
			return;
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
			ircsnprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp[2] - 1)] % 13) + 2));
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
void playcard(char *cnps) {
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
				ircsnprintf(csuitcard, 10, "%d", ((wplayercardsinhand[currentplayer][(cnp - 1)] % 13) + 2));
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
void checkhandwinner() {
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
							return;
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
void checkwarwinner() {
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
void clearstack() {
	for (wpln = 0; wpln < 52; wpln++) {
		wstackcards[wpln]= 0;
	}
	wstackcardscurrent= 0;
}
