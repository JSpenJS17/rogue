/*
 * File with various monster functions in it
 *
 * @(#)monsters.c   4.46 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include "rogue.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
static char lvl_mons[] =
{
    'K', 'E', 'B', 'S', 'H', 'I', 'R', 'O', 'Z', 'L', 'F', 'Q', 'A',
    'N', 'Y', 'C', 'W', 'P', 'T', 'X', 'M', 'V', 'U', 'G', 'J', 'D'
};

static char wand_mons[] =
{
    'K', 'E', 'B', 'S', 'H',   0, 'R', 'O', 'Z',   0,  0, 'Q', 'A',
    0,   'Y',   0, 'W', 'P', 'T',   0, 'U', 'M', 'V',  0,   0,   0
};

/*
 * randmonster:
 *  Pick a monster to show up.  The lower the floor,
 *  the meaner the monster.
 *    monster index: cur_floor + [-6, 3]
 *      -- I might lower the variance on this, 10 unique enemies/floor is a lot
 */
char randmonster (bool wander)
{
    int d;
    char *mons;

    mons = (wander ? wand_mons : lvl_mons);

    do
    {
        d = cur_floor + (rnd (10) - 6);

        if (d < 0)
        {
            d = rnd (5);
        }

        if (d > 25)
        {
            d = rnd (5) + 21;
        }
    }
    while (mons[d] == 0); // never return 0

    return mons[d];
}

/*
 * new_monster:
 *  Pick a new monster and add it to the list
 *  Returns true if spawn is successful, false if spawn fails 
 */

bool new_monster (THING *tp, char type, coord *cp, bool boss)
{
    struct monster *mp;
    int lev_add;

    if ((lev_add = cur_floor - AMULETLEVEL) < 0)
    {
        lev_add = 0;
    }

    attach (mlist, tp);
    tp->t_type = type;
    tp->t_disguise = type;
    tp->t_pos = *cp;
    move (cp->y, cp->x);
    tp->t_oldch = CCHAR (inch());
    tp->t_room = roomin (cp);
    moat (cp->y, cp->x) = tp;
    mp = &monsters[tp->t_type - 'A'];
    
    tp->t_stats.s_lvl = mp->m_stats.s_lvl + lev_add;
    tp->t_stats.s_maxhp = tp->t_stats.s_hpt = roll (tp->t_stats.s_lvl, 4) + (tp->t_stats.s_lvl * 4);
    tp->t_stats.s_arm = mp->m_stats.s_arm - lev_add;
    tp->t_stats.s_movespd = mp->m_stats.s_movespd;
    tp->t_stats.s_movectr = mp->m_stats.s_movectr;

    strcpy (tp->t_stats.s_dmg, mp->m_stats.s_dmg);
    tp->t_stats.s_str = mp->m_stats.s_str;
    tp->t_stats.s_exp = mp->m_stats.s_exp + lev_add * 10 + exp_add (tp);
    tp->t_flags = mp->m_flags;

    // add boss status to bosses
    if (boss) 
    {
        tp->t_flags |= ISBOSS;
        tp->t_flags &= ~ISMEAN;
        tp->t_stats.s_exp *= 5;
    }

    if (cur_floor > 29)
    {
        tp->t_flags |= ISHASTE;
    }

    tp->t_turn = TRUE;
    tp->t_pack = NULL;

    if (ISWEARING (R_AGGR))
    {
        runto (cp);
    }

    if (type == 'X')
    {
        tp->t_disguise = rnd_thing();
    }

    return TRUE;
}

/*
 * spawn_monster:
 *  Spawn a random new monster using new_monster -- new_monster adds it to list
 */
void spawn_monster (THING *tp, bool wander, coord *cp, bool boss)
{
    char monst;

    /*
     * Skip only_boss monsters if we are not spawning a boss
     */
    do {
        monst = randmonster (wander);
    } while (!boss && strchr (only_boss, monst));

    new_monster(tp, monst, cp, boss);
}

/*
 * expadd:
 *  Experience to add for this monster's level/hit points
 */
int exp_add (THING *tp)
{
    int mod;

    if (tp->t_stats.s_lvl == 1)
    {
        mod = tp->t_stats.s_maxhp / 8;
    }
    else
    {
        mod = tp->t_stats.s_maxhp / 6;
    }

    if (tp->t_stats.s_lvl > 9)
    {
        mod *= 20;
    }
    else if (tp->t_stats.s_lvl > 6)
    {
        mod *= 4;
    }

    return mod;
}

/*
 * wanderer:
 *  Create a new wandering monster and aim it at the player
 */

void wanderer()
{
    THING *tp;
    static coord cp;

    tp = new_item();

    do
    {
        find_floor ((struct room *) NULL, &cp, FALSE, TRUE);
    }
    while (roomin (&cp) == proom);

    // new_monster (tp, randmonster (TRUE), &cp, FALSE);
    spawn_monster (tp, TRUE, &cp, FALSE);

    if (on (player, SEEMONST))
    {
        standout();

        if (!on (player, ISHALU))
        {
            addch (tp->t_type);
        }
        else
        {
            addch (rnd (26) + 'A');
        }

        standend();
    }

    runto (&tp->t_pos);
#ifdef MASTER

    if (wizard)
    {
        msg ("started a wandering %s", monsters[tp->t_type - 'A'].m_name);
    }

#endif
}

/*
 * wake_monster:
 *  What to do when the hero steps next to a monster
 */
THING *wake_monster (int y, int x)
{
    THING *tp;
    struct room *rp;
    char ch, *mname;

#ifdef MASTER

    if ((tp = moat (y, x)) == NULL)
    {
        msg ("can't find monster in wake_monster");
    }

#else
    tp = moat (y, x);

    if (tp == NULL)
    {
        endwin(), abort();
    }

#endif
    ch = tp->t_type;

    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (!on (*tp, ISRUN) && rnd (3) != 0 && on (*tp, ISMEAN) && !on (*tp, ISHELD)
            && !ISWEARING (R_STEALTH) && !on (player, ISLEVIT))
    {
        tp->t_dest = &hero;
        tp->t_flags |= ISRUN;
    }

    if (ch == 'M' && !on (player, ISBLIND) && !on (player, ISHALU)
            && !on (*tp, ISFOUND) && !on (*tp, ISCANC) && on (*tp, ISRUN))
    {
        rp = proom;

        if ((rp != NULL && ! (rp->r_flags & ISDARK))
                || dist (y, x, hero.y, hero.x) < LAMPDIST)
        {
            tp->t_flags |= ISFOUND;

            if (!save (VS_MAGIC))
            {
                if (on (player, ISHUH))
                {
                    lengthen (unconfuse, spread (HUHDURATION));
                }
                else
                {
                    fuse (unconfuse, 0, spread (HUHDURATION), AFTER);
                }

                player.t_flags |= ISHUH;
                mname = set_mname (tp);
                addmsg ("%s", mname);

                if (strcmp (mname, "it") != 0)
                {
                    addmsg ("'");
                }

                msg ("s gaze has confused you");
            }
        }
    }

    /*
     * Let greedy ones guard gold
     * Used to be a flag ISGREED, is now just orcs cause that was the only ISGREED monster
     */
    if (tp->t_type == 'O' && !on (*tp, ISRUN))
    {
        tp->t_flags |= ISRUN;

        if (proom->r_goldval)
        {
            tp->t_dest = &proom->r_gold;
        }
        else
        {
            tp->t_dest = &hero;
        }
    }

    return tp;
}

/*
 * give_pack:
 *  Give a pack to a monster if it deserves one
 */

void give_pack (THING *tp)
{
    if (cur_floor >= max_floor && rnd (100) < monsters[tp->t_type - 'A'].m_carry)
    {
        attach (tp->t_pack, new_thing(-1));
    }
}

/*
 * save_throw:
 *  See if a creature save against something
 */
int save_throw (int which, THING *tp)
{
    int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll (1, 20) >= need);
}

/*
 * save:
 *  See if he saves against various nasty things
 */
int save (int which)
{
    if (which == VS_MAGIC)
    {
        if (ISRING (LEFT, R_PROTECT))
        {
            which -= cur_ring[LEFT]->o_arm;
        }

        if (ISRING (RIGHT, R_PROTECT))
        {
            which -= cur_ring[RIGHT]->o_arm;
        }
    }

    return save_throw (which, &player);
}
