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

#ifdef WIN32
#include "modconfigwin32.h"
#else
#include "modconfig.h"
#endif
#include "neostats.h"    /* Required for bot support */
#include "warserv.h"

/** Copyright info */
const char *ws_copyright[] = {
	"Copyright (c) 2004-2005 DeadNotBuried",
	"Portions Copyright (c) 1999-2005, NeoStats",
	NULL
};

const char *ws_about[] = {
	"\2War Card Game Service\2",
	"",
	"All cards are Dealt out evenly when the game starts.",
	"The Object of the game is to hold ALL the cards.",
	"Each Player plays a card from their hand, and the",
	"highest card wins all cards played that turn.",
	" ",
	"If the played cards are equal, there is a War.",
	"all played cards stay out, and each player involved",
	"in the war plays 3 more cards. the War continues the",
	"same way, untill someone wins all the played cards.",
	"",
	"If you don't have enough cards to play, your cards are",
	"automatically put into the center, and you surrender.",
	"",
	"NOTE: Game can't be joined to after play has started.",
	NULL
};

/*
 * Commands and Settings
*/
static bot_cmd ws_commands[]=
{
	{"START",	StartWarGame,		0,	0,	ws_help_start,		ws_help_start_oneline},
	{"STOP",	StopWarGame,		0,	0,	ws_help_stop,		ws_help_stop_oneline},
	{"JOIN",	JoinWarGame,		0,	0,	ws_help_join,		ws_help_join_oneline},
	{"REMOVE",	RemoveWarGame,		0,	0,	ws_help_remove,		ws_help_remove_oneline},
	{"PLAYERS",	ShowPlayersWarGame,	0,	0,	ws_help_players,	ws_help_players_oneline},
	{"TURN",	ShowTurnWarGame,	0,	0,	ws_help_turn,		ws_help_turn_oneline},
	{"PLAY",	PlayCardsWarGame,	0,	0,	ws_help_play,		ws_help_play_oneline},
	{NULL,		NULL,			0, 	0,	NULL,			NULL}
};

static bot_setting ws_settings[]=
{
	{"CHAN",	&warroom,	SET_TYPE_CHANNEL,	0,	MAXCHANLEN,	NS_ULEVEL_ADMIN,	"warroom",	NULL,	ws_help_set_chan,	ws_cmd_set_chan,	(void *)"#Games_War" },
	{NULL,		NULL,		0,			0,	0,		0,			NULL,		NULL,	NULL, 			NULL },
};


/*
 * Module Info definition 
*/
ModuleInfo module_info = {
	"WarServ",
	"War Card Game Module For NeoStats",
	ws_copyright,
	ws_about,
	NEOSTATS_VERSION,
	MODULE_VERSION,
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
	{EVENT_NULL, NULL}
};

/** BotInfo */
static BotInfo ws_botinfo = 
{
	"WarServ", 
	"WarServ1", 
	"WarServ", 
	BOT_COMMON_HOST, 
	"War Game Service",
	BOT_FLAG_SERVICEBOT|BOT_FLAG_PERSIST,
	ws_commands,
	ws_settings, 
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
		strlcpy (warroom, "#Games_War", MAXCHANLEN);
	}
	irc_chanalert (ws_bot, "Game will start in %s", warroom);
	irc_join (ws_bot, warroom, "+o");
	return NS_SUCCESS;
};

/*
 * Init module
*/
int ModInit( void )
{
	stopwar();
	return NS_SUCCESS;
}

/*
 * Exit module
*/
int ModFini( void )
{
	if (currentwargamestatus == WS_GAME_STARTING) {
		DelTimer ("startwar");
	}
	return NS_SUCCESS;
}

/*
 * Set Overrides
*/
static int ws_cmd_set_chan(CmdParams *cmdparams, SET_REASON reason) 
{
	if (reason == SET_VALIDATE) {
		if (currentwargamestatus != WS_GAME_STOPPED) {
			irc_prefmsg (ws_bot, cmdparams->source, "Unable to change Game Channel while Game in Progress.");
			return NS_FAILURE;
		}
		irc_chanalert (ws_bot, "Game Channel Changing to %s , Parting %s (%s)", cmdparams->av[1], warroom, cmdparams->source->name);
		irc_chanprivmsg (ws_bot, warroom, "\0039%s has changed Channels, Game will now be available in %s", cmdparams->source->name, cmdparams->av[1]);
		irc_part (ws_bot, warroom, NULL);
		return NS_SUCCESS;
	}
	if (reason == SET_CHANGE) {
		irc_join (ws_bot, warroom, "+o");
		irc_chanalert (ws_bot, "Game Now Available In %s", warroom);
		return NS_SUCCESS;
	}
	return NS_SUCCESS;
}
