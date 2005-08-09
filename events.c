/* WarServ - War Card Game Service - NeoStats Addon Module
** Copyright (c) 2004-2005 Justin Hammond, Mark Hetherington, Jeff Lang
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
 * Start War Game
*/
int StartWarGame (CmdParams* cmdparams)
{
	if (currentwargamestatus == WS_GAME_STOPPED) {
		startcountdowntimer(cmdparams->source->name);
	} else if (currentwarplayercount < 10) {
		irc_privmsg (ws_bot, cmdparams->source, "\0034A game has already started \0037%s\0034, Type '\2\003Join\2\0034' in \2\003%s\2\0034 To Join in.", cmdparams->source->name, warroom);
	} else {
		irc_privmsg (ws_bot, cmdparams->source, "\0034A game has already started \0037%s\0034 and all spots are taken. Please try the next game.", cmdparams->source->name);
	}
	return NS_SUCCESS;
}

/*
 * Stop War Game
*/
int StopWarGame (CmdParams* cmdparams)
{
	if (currentwargamestatus != WS_GAME_STOPPED) {
		if (cmdparams->source->user->ulevel >= NS_ULEVEL_OPER) {
			irc_chanprivmsg (ws_bot, warroom, "\0039Stopping Current Game (\0037%s\0039)", cmdparams->source->name);
			stopwar();
			wpln = currentwarplayercount;
			return NS_SUCCESS;
		} else {
			for (wpln = 0; wpln < currentwarplayercount; wpln++) {
				if (!ircstrcasecmp (wplayernick[wpln], cmdparams->source->name)) {
					irc_chanprivmsg (ws_bot, warroom, "\0039Stopping Current Game (\0037%s\0039)", cmdparams->source->name);
					stopwar();
					wpln = currentwarplayercount;
					return NS_SUCCESS;
				}
			}
		}
	}
	return NS_SUCCESS;
}

/*
 * Join War Game
*/
int JoinWarGame (CmdParams* cmdparams)
{
	if (currentwargamestatus == WS_GAME_STARTING) {
		joinwar(cmdparams->source->name);
	}
	return NS_SUCCESS;
}

/*
 * Remove From War Game
*/
int RemoveWarGame (CmdParams* cmdparams)
{
	Client *u;

	if (currentwargamestatus != WS_GAME_STOPPED) {
		if ( cmdparams->ac > 0 ) {
			/* allows removal of a player by anyone
			 * if player no longer connected to ircd */
			u = FindUser(cmdparams->av[0]);
			if (!u || cmdparams->source->user->ulevel >= NS_ULEVEL_OPER) {
				removewar(u->name);
			}		
		} else {
			removewar(cmdparams->source->name);
		}
	}
	return NS_SUCCESS;
}

/*
 * Display Current Players
*/
int ShowPlayersWarGame (CmdParams* cmdparams)
{
	if (currentwargamestatus == WS_GAME_PLAYING) {
		irc_chanprivmsg (ws_bot, warroom, "\0039Current Players are\0038 :\0037 %s %s %s %s %s %s %s %s %s %s", wplayernick[0], wplayernick[1], wplayernick[2], wplayernick[3], wplayernick[4], wplayernick[5], wplayernick[6], wplayernick[7], wplayernick[8], wplayernick[9]);
	}
	return NS_SUCCESS;
}

/*
 * Display Current Player Turn
*/
int ShowTurnWarGame (CmdParams* cmdparams)
{
	if (currentwargamestatus == WS_GAME_PLAYING) {
		if (warinprogress == 1) {
			irc_chanprivmsg (ws_bot, warroom, "\0039The Current Player is \0037%s\0039 holding\00311 %d\0039 cards, and are currently at \0034WAR\0039 which three would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
		} else {
			irc_chanprivmsg (ws_bot, warroom, "\0039The Current Player is \0037%s\0039 currently holding\00311 %d\0039 cards, which would you like to play ?", wplayernick[currentplayer], wplayercardstotal[currentplayer]);
		}
	}
	return NS_SUCCESS;
}

/*
 * Play Cards
*/
int PlayCardsWarGame (CmdParams* cmdparams)
{
	if (!ircstrcasecmp (cmdparams->source->name,wplayernick[currentplayer]) && currentwargamestatus == WS_GAME_PLAYING) {
		if (warinprogress == 1) {
			if (cmdparams->ac == 3) {
				playwarcards(cmdparams->av[0], cmdparams->av[1], cmdparams->av[2]);
			}
		} else {
			if (cmdparams->ac == 1) {
				playcard(cmdparams->av[0]);
			}
		}
	}
	return NS_SUCCESS;
}
