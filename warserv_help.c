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

const char *ws_help_set_chan[] = {
	"\2CHAN <#Channel>\2 - Set Channel War Games Play in",
	NULL
};

/*
 * Help Text
*/
const char ws_help_start_oneline[] = "Start a Game in Channel";
const char ws_help_stop_oneline[] = "Stops Game";
const char ws_help_join_oneline[] = "Join a game currently Starting";
const char ws_help_remove_oneline[] = "Remove From Game";
const char ws_help_players_oneline[] = "Display Player List In Channel";
const char ws_help_turn_oneline[] = "Display Current Player In Channel";
const char ws_help_play_oneline[] = "Play Card(s)";

const char *ws_help_start[] = {
	"Syntax: \2START\2",
	"",
	"Starts War Game in configured Channel.",
	NULL
};

const char *ws_help_stop[] = {
	"Syntax: \2STOP\2",
	"",
	"Stops Currently Running War Game.",
	"Only Available to a Current PLayer.",
	NULL
};

const char *ws_help_join[] = {
	"Syntax: \2JOIN\2",
	"",
	"Joins you to the Currently Starting Game.",
	"NOTE: Once Game Starts no New Joins Allowed.",
	NULL
};

const char *ws_help_remove[] = {
	"Syntax: \2REMOVE <nick>\2",
	"",
	"With No Nick specified, removes you from the current game.",
	"",
	"With Nick specified, removes Nick from the curent Game, as",
	"as Nick is no longer connected to the network.",
	NULL
};

const char *ws_help_players[] = {
	"Syntax: \2PLAYERS\2",
	"",
	"Displays List of Current Players in channel.",
	NULL
};

const char *ws_help_turn[] = {
	"Syntax: \2TURN\2",
	"",
	"Shows Current Player waiting to play Cards.",
	NULL
};

const char *ws_help_play[] = {
	"Syntax: \2PLAY\2 # <#> <#>",
	"",
	"During Normal Round one card number must be entered.",
	"During a WAR round, 3 card numbers Must be entered.",
	NULL
};
