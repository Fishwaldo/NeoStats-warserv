/* WarServ - War Card Game Service - NeoStats Addon Module
** Copyright (c) 2004-2005 Justin Hammond, Mark Hetherington, DeadNotBuried
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

Bot *ws_bot;

/*
 * WarServ Module Help
*/
extern const char *ws_help_set_chan[];
extern const char ws_help_start_oneline[];
extern const char ws_help_stop_oneline[];
extern const char ws_help_join_oneline[];
extern const char ws_help_remove_oneline[];
extern const char ws_help_players_oneline[];
extern const char ws_help_turn_oneline[];
extern const char ws_help_play_oneline[];
extern const char *ws_help_start[];
extern const char *ws_help_stop[];
extern const char *ws_help_join[];
extern const char *ws_help_remove[];
extern const char *ws_help_players[];
extern const char *ws_help_turn[];
extern const char *ws_help_play[];

/*
 * Defines
*/
#define WS_GAME_STOPPED		0x00000001	/* Game Not Running */
#define WS_GAME_STARTING	0x00000002	/* Game Starting */
#define WS_GAME_PLAYING		0x00000003	/* Game Running */
#define WS_GAME_STOPPING	0x00000004	/* Game Running */

/*
 * Variables
*/
char warroom[MAXCHANLEN];
int currentwargamestatus;
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

/*
 * Procedures
*/
int PlayerNickChange (CmdParams* cmdparams);
int StartWarGame (CmdParams* cmdparams);
int StopWarGame (CmdParams* cmdparams);
int JoinWarGame (CmdParams* cmdparams);
int RemoveWarGame (CmdParams* cmdparams);
int ShowPlayersWarGame (CmdParams* cmdparams);
int ShowTurnWarGame (CmdParams* cmdparams);
int PlayCardsWarGame (CmdParams* cmdparams);
static int ws_cmd_set_chan (CmdParams *cmdparams, SET_REASON reason);
void wardealcards(void);
void playershufflecards(void);
void stopwar(void);
void startcountdowntimer(char *nic);
int startwar(void);
void joinwar(char *nic);
void removewarother(char *nic, char *ntr);
void removewar(char *nic);
void askplaycard(void);
void playwarcards(char *cnps1, char *cnps2, char *cnps3);
void playcard(char *cnps);
void checkhandwinner(void);
void checkwarwinner(void);
void clearstack(void);
