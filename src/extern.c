/*
 * global variable initializaton
 *
 * @(#)extern.c 4.82 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

bool after;             /* True if we want after daemons */
bool again;             /* Repeating the last command */
int  noscore;               /* Was a wizard sometime */
bool seenstairs;            /* Have seen the stairs (for lsd) */
bool amulet = FALSE;            /* He found the amulet */
bool door_stop = FALSE;         /* Stop running when we pass a door */
bool fight_flush = FALSE;       /* True if toilet input */
bool firstmove = FALSE;         /* First move after setting door_stop */
bool got_ltc = FALSE;           /* We have gotten the local tty chars */
bool has_hit = FALSE;           /* Has a "hit" message pending in msg */
bool in_shell = FALSE;          /* True if executing a shell */
bool inv_describe = TRUE;       /* Say which way items are being used */
bool jump = FALSE;          /* Show running as series of jumps */
bool kamikaze = FALSE;          /* to_death really to DEATH */
bool lower_msg = FALSE;         /* Messages should start w/lower case */
bool move_on = FALSE;           /* Next move shouldn't pick up items */
bool msg_esc = FALSE;           /* Check for ESC from msg's --More-- */
bool passgo = TRUE;            /* Follow passages */
bool playing = TRUE;            /* True until he quits */
bool q_comm = FALSE;            /* Are we executing a 'Q' command? */
bool running = FALSE;           /* True if player is running */
bool save_msg = TRUE;           /* Remember last msg */
bool see_floor = TRUE;          /* Show the lamp illuminated floor */
bool stat_msg = FALSE;          /* Should status() print as a msg() */
bool terse = FALSE;         /* True if we should be short */
bool to_death = FALSE;          /* Fighting is to the death! */
bool tombstone = TRUE;          /* Print out tombstone at end */
#ifdef MASTER
int wizard = FALSE;         /* True if allows wizard commands */
#endif
bool pack_used[26] =            /* Is the character used in the pack? */
{
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
    FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
};

char dir_ch;                /* Direction from last get_dir() call */
char file_name[MAXSTR];         /* Save file name */
char huh[MAXSTR];           /* The last message printed */
char *p_colors[MAXPOTIONS];     /* Colors of the potions */
char prbuf[2 * MAXSTR];         /* buffer for sprintfs */
char *r_stones[MAXRINGS];       /* Stone settings of the rings */
char runch;             /* Direction player is running */
char *s_names[MAXSCROLLS];      /* Names of the scrolls */
char take;              /* Thing she is taking */
char whoami[MAXSTR];            /* Name of player */
char *ws_made[MAXSTICKS];       /* What sticks are made of */
char *ws_type[MAXSTICKS];       /* Is it a wand or a staff */
int  orig_dsusp;            /* Original dsusp char */
char fruit[MAXSTR] =            /* Favorite fruit */
{ 's', 'l', 'i', 'm', 'e', '-', 'm', 'o', 'l', 'd', '\0' };
char home[MAXSTR] = { '\0' };       /* User's home directory */
char *inv_t_name[] =
{
    "Overwrite",
    "Slow",
    "Clear"
};
char l_last_comm = '\0';        /* Last last_comm */
char l_last_dir = '\0';         /* Last last_dir */
char last_comm = '\0';          /* Last command typed */
char last_dir = '\0';           /* Last direction given */
char *tr_name[] =           /* Names of the traps */
{
    "a trapdoor",
    "an arrow trap",
    "a sleeping gas trap",
    "a beartrap",
    "a teleport trap",
    "a poison dart trap",
    "a rust trap",
    "a mysterious trap"
};


int n_objs;             /* # items listed in inventory() call */
int ntraps;             /* Number of traps on this floor */
int hungry_state = 0;           /* How hungry is he */
int equip_state = 0;          /* What is he wearing */
int inpack = 0;             /* Number of things in pack */
int inv_type = 0;           /* Type of inventory to use */
int cur_floor = 1;              /* What floor she is on */
int max_hit;                /* Max damage done to her in to_death */
int max_floor;              /* Deepest player has gone */
int mpos = 0;               /* Where cursor is on top line */
int no_food = 0;            /* Number of levels without food */
int a_class[MAXARMORS] =        /* Armor class for each armor type */
{
    8,  /* LEATHER */
    7,  /* RING_MAIL */
    7,  /* STUDDED_LEATHER */
    6,  /* SCALE_MAIL */
    5,  /* CHAIN_MAIL */
    4,  /* SPLINT_MAIL */
    4,  /* BANDED_MAIL */
    3,  /* PLATE_MAIL */
};

int count = 0;              /* Number of times to repeat command */
FILE *scoreboard = NULL;    /* File descriptor for score file */
int food_left;              /* Amount of food in hero's stomach */
int lastscore = -1;         /* Score before this turn */
int no_command = 0;         /* Number of turns asleep */
int no_move = 0;            /* Number of turns held in place */
int purse = 0;              /* How much gold he has */
int quiet = 0;              /* Number of quiet turns */
int vf_hit = 0;             /* Number of time flytrap has hit */

int dnum;               /* Dungeon number */
int seed;               /* Random number seed */
int e_levels[] =        /* Experience required to level up */
/* This should be modified for balancing */
{
    10L,      /* Level 2  */
    50L,      /* Level 3  */
    125L,     /* Level 4  */
    250L,     /* Level 5  */ // perk?
    480L,     /* Level 6  */
    800L,     /* Level 7  */
    1220L,    /* Level 8  */
    1740L,    /* Level 9  */
    2500L,    /* Level 10 */ // perk?
    3400L,    /* Level 11 */
    4500L,    /* Level 12 */
    5900L,    /* Level 13 */
    7400L,    /* Level 14 */
    9000L,    /* Level 15 */ // perk?
    11000L,   /* Level 16 */
    13000L,   /* Level 17 */
    15000L,   /* Level 18 */
    17000L,   /* Level 19 */
    20000L,   /* Level 20 */ // perk?
    0L
};

coord delta;                /* Change indicated to get_dir() */
coord oldpos;               /* Position before last look() call */
coord stairs;               /* Location of staircase */

PLACE places[MAXLINES * MAXCOLS];       /* floor map */

THING *cur_armor;           /* What he is wearing */
THING *cur_ring[2];         /* Which rings are being worn */
THING *cur_weapon;          /* Which weapon he is weilding */
THING *l_last_pick = NULL;      /* Last last_pick */
THING *last_pick = NULL;        /* Last object picked in get_item() */
THING *lvl_obj = NULL;          /* List of objects on this floor */
THING *mlist = NULL;            /* List of monsters on the floor */
THING player;               /* His stats */
/* restart of game */

WINDOW *hw = NULL;          /* used as a scratch window */

#define INIT_STATS { 16, 0, 1, 10, 12, "1x4", 12 }

struct stats max_stats = INIT_STATS;    /* The maximum for the player */

struct room *oldrp;         /* Roomin(&oldpos) */
struct room rooms[MAXROOMS];        /* One for each room -- A floor */
struct room passages[MAXPASS] =     /* One for each passage */
{
    {   {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}}
    },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} },
    { {0, 0}, {0, 0}, {0, 0}, 0, ISGONE | ISDARK, 0, {{0, 0}} }
};

#define ___ 1
#define XX 10
struct monster monsters[26] =
{
    /* NOTE: the damage is %%% on venus flytrap so that xstr won't merge this */
    /* string with others, since it is written on in the program */
    /* NOTE: HIGHER armor means easier to hit for some reason */
    /* Name        CARRY      FLAG             str,  exp, lvl, amr, hpt, dmg(NDICExSIDES), maxhp, mvspd, mvctr */
    { "aquator",       0,   ISMEAN,           { XX,   20,   5,   3, ___, "0x0/0x0",        ___,     1,     0} },
    { "bat",           0,   ISFLY,            { XX,    1,   1,   5, ___, "1x2",            ___,     0,     0} },
    { "centaur",      25,   0,                { XX,   30,   5,   6, ___, "2x2/2x2/2x4",    ___,     0,     0} }, /* boss 1, floor 5  */
    { "dragon",      100,   0,                { XX,  250,  18,   3, ___, "4x8/4x8/12x4",   ___,     2,     0} }, /* boss 5, floor 26 */
    { "emu",           0,   ISMEAN,           { XX,    2,   1,   8, ___, "1x2",            ___,     1,     0} },
    { "venus flytrap", 0,   ISMEAN,           { XX,   80,   8,   4, ___, "%%%x0",          ___,     0,     0} },
    { "griffin",     100,   ISFLY,            { XX,  100,  12,   5, ___, "4x2/3x4",        ___,     0,     0} }, /* boss 3, floor 15 */
    { "hobgoblin",     0,   ISMEAN,           { XX,    3,   1,   6, ___, "1x8",            ___,     1,     0} },
    { "ice monster",   0,   0,                { XX,    5,   1,  10, ___, "1x2",            ___,     1,     0} },
    { "jabberwock",  100,   0,                { XX,  160,  15,   7, ___, "2x6/2x6/1x20",   ___,     0,     0} }, /* boss 4, floor 20 */
    { "kestrel",       0,   ISMEAN | ISFLY,   { XX,    2,   1,   8, ___, "1x2",            ___,     0,     0} },
    { "leprechaun",    0,   0,                { XX,   10,   3,   9, ___, "1x1",            ___,     0,     0} },
    { "medusa",       40,   ISMEAN,           { XX,  200,   8,   3, ___, "4x4/4x6",        ___,     1,     0} },
    { "nymph",       100,   0,                { XX,   37,   3,  10, ___, "0x0",            ___,     0,     0} },
    { "orc",          15,   0,                { XX,   15,   1,   7, ___, "1x8/2x8",        ___,     1,     0} },
    { "phantom",       0,   ISINVIS,          { XX,  120,   8,   4, ___, "4x4",            ___,     1,     0} },
    { "quagga",        0,   ISMEAN,           { XX,   25,   4,   6, ___, "3x2",            ___,     0,     0} },
    { "rattlesnake",   0,   ISMEAN,           { XX,   10,   2,   5, ___, "2x6",            ___,     1,     0} },
    { "snake",         0,   ISMEAN,           { XX,    2,   1,   6, ___, "1x4",            ___,     1,     0} },
    { "troll",        15,   ISREGEN | ISMEAN, { XX,   50,   8,   5, ___, "3x8/3x8/5x6",    ___,     2,     0} }, /* boss 2, floor 10 */
    { "black unicorn", 0,   ISMEAN,           { XX,  200,   7,  -1, ___, "2x8/2x10",       ___,     0,     0} },
    { "vampire",      20,   ISREGEN | ISMEAN, { XX,  100,   8,   2, ___, "2x6/2x10",       ___,     0,     0} },
    { "wraith",        0,   0,                { XX,   75,   5,   5, ___, "1x6",            ___,     0,     0} },
    { "xeroc",        30,   0,                { XX,  100,   7,   8, ___, "4x8/6x8",        ___,     1,     0} },
    { "yeti",         30,   0,                { XX,   50,   4,   7, ___, "2x6",            ___,     0,     0} },
    { "zombie",        0,   ISMEAN,           { XX,   20,   3,   9, ___, "2x8/3x6",        ___,     1,     0} }
};
#undef ___
#undef XX

int boss_floors[NUMBOSSES] = 
{
    5, 10, 15, 20, 26
};

char boss_order[NUMBOSSES + 1] = 
{
    'C', 'T', 'G', 'J', 'D', '\0'
};

/* monster types that can only be spawned as bosses */
char only_boss[] = 
{
    'G', 'J', 'D', '\0' 
};

struct obj_info things[NUMTHINGS] =
{
    { 0,            23 },   /* potion */
    { 0,            38 },   /* scroll */
    { 0,            14 },   /* food */
    { 0,             7 },   /* weapon */
    { 0,             7 },   /* armor */
    { 0,             1 },   /* ring */
    { 0,            10 },   /* stick */
};

struct obj_info arm_info[MAXARMORS] =
{
    { "leather armor",           20,     20, NULL, FALSE },
    { "ring mail",               15,     25, NULL, FALSE },
    { "studded leather armor",   15,     20, NULL, FALSE },
    { "scale mail",              13,     30, NULL, FALSE },
    { "chain mail",              12,     75, NULL, FALSE },
    { "splint mail",             10,     80, NULL, FALSE },
    { "banded mail",             10,     90, NULL, FALSE },
    { "plate mail",               5,    150, NULL, FALSE },
};
struct obj_info pot_info[MAXPOTIONS] =
{
    { "confusion",           7,   5, NULL, FALSE },
    { "hallucination",       8,   5, NULL, FALSE },
    { "poison",              8,   5, NULL, FALSE },
    { "gain strength",      13, 150, NULL, FALSE },
    { "see invisible",       3, 100, NULL, FALSE },
    { "healing",            13, 130, NULL, FALSE },
    { "monster detection",   6, 130, NULL, FALSE },
    { "magic detection",     6, 105, NULL, FALSE },
    { "raise level",         2, 250, NULL, FALSE },
    { "extra healing",       5, 200, NULL, FALSE },
    { "haste self",          5, 190, NULL, FALSE },
    { "restore strength",   13, 130, NULL, FALSE },
    { "blindness",           5,   5, NULL, FALSE },
    { "levitation",          6,  75, NULL, FALSE },
};
struct obj_info ring_info[MAXRINGS] =
{
    { "protection",           9, 400, NULL, FALSE },
    { "add strength",         9, 400, NULL, FALSE },
    { "sustain strength",     5, 280, NULL, FALSE },
    { "searching",           10, 420, NULL, FALSE },
    { "see invisible",       10, 310, NULL, FALSE },
    { "adornment",            1,  10, NULL, FALSE },
    { "aggravate monster",   10,  10, NULL, FALSE },
    { "dexterity",            8, 440, NULL, FALSE },
    { "increase damage",      8, 400, NULL, FALSE },
    { "regeneration",         4, 460, NULL, FALSE },
    { "slow digestion",       9, 240, NULL, FALSE },
    { "teleportation",        5,  30, NULL, FALSE },
    { "stealth",              7, 470, NULL, FALSE },
    { "maintain armor",       5, 380, NULL, FALSE },
};
struct obj_info scr_info[MAXSCROLLS] =
{
    { "monster confusion",  6, 140, NULL, FALSE },
    { "magic mapping",      4, 150, NULL, FALSE },
    { "hold monster",       5, 180, NULL, FALSE },
    { "sleep",              5,   5, NULL, FALSE },
    { "enchant armor",      7, 160, NULL, FALSE },
    { "identify",          15,  80, NULL, FALSE }, 
    { "scare monster",      3, 200, NULL, FALSE },
    { "food detection",     2,  60, NULL, FALSE },
    { "teleportation",      5, 165, NULL, FALSE },
    { "enchant weapon",     8, 150, NULL, FALSE },
    { "create monster",     4,  75, NULL, FALSE },
    { "remove curse",      10, 105, NULL, FALSE },
    { "aggravate monsters", 7,  20, NULL, FALSE },
    { "protect armor",      2, 250, NULL, FALSE },
};
struct obj_info weap_info[MAXWEAPONS + 1] =
{
    { "mace",               11,   8, NULL, FALSE },
    { "long sword",         11,  15, NULL, FALSE },
    { "short bow",          12,  15, NULL, FALSE },
    { "arrow",              12,   1, NULL, FALSE },
    { "dagger",              8,   3, NULL, FALSE },
    { "two handed sword",   10,  75, NULL, FALSE },
    { "dart",               12,   2, NULL, FALSE },
    { "shuriken",           12,   5, NULL, FALSE },
    { "spear",              12,   5, NULL, FALSE },
    { NULL, 0 },    /* DO NOT REMOVE: fake entry for dragon's breath */
};
struct obj_info ws_info[MAXSTICKS] =
{
    { "light",          20, 250, NULL, FALSE },
    { "invisibility",    6,   5, NULL, FALSE },
    { "lightning",       3, 330, NULL, FALSE },
    { "fire",            3, 330, NULL, FALSE },
    { "cold",            3, 330, NULL, FALSE },
    { "polymorph",       7, 310, NULL, FALSE },
    { "magic missile",  10, 170, NULL, FALSE },
    { "haste monster",  10,   5, NULL, FALSE },
    { "slow monster",   11, 350, NULL, FALSE },
    { "drain life",      9, 300, NULL, FALSE },
    { "nothing",         1,   5, NULL, FALSE },
    { "teleport away",   6, 340, NULL, FALSE },
    { "teleport to",     6,  50, NULL, FALSE },
    { "cancellation",    5, 280, NULL, FALSE },
};
/* ("You zap the wand, %s", stick_descs[stick_id]) */
char* stick_descs[MAXSTICKS] = 
{
    "a bright light emits",                                 /* WS_LIGHT */
    "it's so clear in here now",                            /* WS_INVIS */
    "a bolt of lightning shoots out",                       /* WS_ELECT */
    "a fireball shoots out",                                /* WS_FIRE */
    "a blast of cold shoots out",                           /* WS_COLD */
    "it smells like smoke in here",                         /* WS_POLYMORPH */
    "a magic missle shoots out",                            /* WS_MISSILE */
    "the breeze speeds up",                                 /* WS_HASTE_M */
    "the breeze slows down",                                /* WS_SLOW_M */
    "you feel your life flowing through it",                /* WS_DRAIN */
    "nothing happens",                                      /* WS_NOP */
    "the space in front of you has been teleported away",   /* WS_TELAWAY */
    "space folds violently inward toward you",              /* WS_TELTO */
    "the air now seems very ordinary"                       /* WS_CANCEL */
};

struct h_list helpstr[] =
{
    {'?',           "    prints help",                         TRUE},
    {'w',           "    up",                                  TRUE},
    {'a',           "    left",                                TRUE},
    {'s',           "    down",                                TRUE},
    {'d',           "    right",                               TRUE},
    {'\0',          "    <SHIFT><dir>: run that way",          TRUE},
    {'k',           "    rest and search for secrets",         TRUE},
    {'e',           "    use an item",                         TRUE},
    {'q',           "    drop an item",                        TRUE},
    // {'t',             throw something",                     TRUE},
    {'j',           "    fight till death or near death",      TRUE},
    {'>',           "    go down a staircase (%)",             TRUE},
    {'<',           "    go up a staircase (%)",               TRUE},
    // {'c',        "    call object",                         TRUE}, /* I lowkey want to add this back */
    // {'o',        "    examine/set options",                 TRUE}, /* TODO: add back in after options are functional */
    {CTRL ('P'),    "   repeat last message",                 TRUE},
    {ESCAPE,        "   (escape) cancel command",             TRUE},
    {CTRL ('S'),    "   save game",                           TRUE},
    {'Q',           "    quit",                                TRUE},
    {'v',           "    print version number",                TRUE},
    {0,     NULL }
};
