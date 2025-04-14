/*
 * Routines dealing specifically with rings
 *
 * @(#)rings.c  4.19 (Berkeley) 05/29/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include "rogue.h"

/*
 * ring_on:
 *  Put a ring on a hand
 */

void ring_on(THING* obj)
{
    int ring;

    /*
     * Make certain that it is somethings that we want to wear
     */
    if (obj == NULL)
    {
        return;
    }

    if (obj->o_type != RING)
    {
        if (!terse)
        {
            msg ("it would be difficult to wrap that around a finger");
        }
        else
        {
            msg ("not a ring");
        }

        return;
    }

    /*
     * find out which hand to put it on
     */
    if (is_current (obj))
    {
        return;
    }

    if (cur_ring[LEFT] == NULL && cur_ring[RIGHT] == NULL)
    {
        if ((ring = gethand()) < 0)
        {
            return;
        }
    }
    else if (cur_ring[LEFT] == NULL)
    {
        ring = LEFT;
    }
    else if (cur_ring[RIGHT] == NULL)
    {
        ring = RIGHT;
    }
    else
    {
        if (!terse)
        {
            msg ("you already have a ring on each hand");
        }
        else
        {
            msg ("wearing two");
        }

        return;
    }

    cur_ring[ring] = obj;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which)
    {
    case R_ADDSTR:
        chg_str (obj->o_arm);
        break;

    case R_SEEINVIS:
        invis_on();
        break;

    case R_AGGR:
        aggravate();
        break;
    }

    if (!terse)
    {
        addmsg ("you are now wearing ");
    }

    msg ("%s (%c)", inv_name (obj, TRUE), obj->o_packch);
}

/*
 * ring_off:
 *  take off the ring on a given hand
 */

void ring_off(int hand)
{
    int ring;
    THING *obj;

    if (hand != LEFT && hand != RIGHT)
    {
        /* whoops */
        return;
    }
    
    mpos = 0;
    obj = cur_ring[hand];

    if (obj == NULL)
    {
        msg ("not wearing such a ring");
        return;
    }

    if (dropcheck (obj))
    {
        msg ("was wearing %s(%c)", inv_name (obj, TRUE), obj->o_packch);
    }
}

/*
 * gethand:
 *  Which hand is the hero interested in?
 */
int gethand()
{
    int c;

    for (;;)
    {
        if (terse)
        {
            msg ("left or right ring? ");
        }
        else
        {
            msg ("left hand or right hand? ");
        }

        if ((c = readchar()) == ESCAPE)
        {
            return -1;
        }

        mpos = 0;

        if (c == 'l' || c == 'L')
        {
            return LEFT;
        }
        else if (c == 'r' || c == 'R')
        {
            return RIGHT;
        }

        if (terse)
        {
            msg ("L or R");
        }
        else
        {
            msg ("please type L or R");
        }
    }
}

/*
 * ring_eat:
 *  How much food does this ring use up?
 */
int ring_eat (int hand)
{
    THING *ring;
    int eat;
    
    /* 
     * Determines how much food is used by a ring
     *   Positive values mean it uses food
     *   0 values are neutral, no food is used or given
     *   Negative values have probability of using food (1/-n chance)
     *     Except R_DIGEST, which will give food equal to -n
     */
    static int uses[] =
    {
        -3, /* R_PROTECT */     -3, /* R_ADDSTR */
         0, /* R_SUSTSTR */     -4, /* R_SEARCH */
         0, /* R_SEEINVIS */     0, /* R_NOP */
         0, /* R_AGGR */        -4, /* R_ADDHIT */
        -3, /* R_ADDDAM */      -2, /* R_REGEN */
        -2, /* R_DIGEST */       0, /* R_TELEPORT */
        -4, /* R_STEALTH */     -3  /* R_SUSTARM */
    };

    if ((ring = cur_ring[hand]) == NULL)
    {
        return 0;
    }

    if ((eat = uses[ring->o_which]) < 0)
    {
        eat = (rnd (-eat) == 0);
    }

    if (ring->o_which == R_DIGEST) // special case, slow digest ring will actually feed you
    {
        eat = -eat;
    }

    return eat;
}

/*
 * ring_num:
 *  Print ring bonuses
 */
char *ring_num (THING *obj)
{
    static char buf[10];

    if (! (obj->o_flags & ISKNOW))
    {
        return "";
    }

    switch (obj->o_which)
    {
    case R_PROTECT:
    case R_ADDSTR:
    case R_ADDDAM:
    case R_ADDHIT:
        sprintf (buf, " [%s]", num (obj->o_arm, 0, RING));
        break;
    default:
        return "";
    }

    return buf;
}
