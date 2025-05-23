/*
 * Functions for dealing with problems brought about by weapons
 *
 * @(#)weapons.c    4.34 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"

#define NO_WEAPON -1

int group = 2;

static struct init_weaps
{
    char *iw_dam;   /* Damage when wielded */
    char *iw_hrl;   /* Damage when thrown */
    char iw_launch; /* Launching weapon */
    int iw_flags;   /* Miscellaneous flags */
} init_dam[MAXWEAPONS] =
{
    { "2x4",    "1x3",  NO_WEAPON,  0,      },  /* Mace */
    { "3x4",    "1x2",  NO_WEAPON,  0,      },  /* Long sword */
    { "1x1",    "1x1",  NO_WEAPON,  0,      },  /* Bow */
    { "1x1",    "2x3",  BOW,        ISMANY | ISMISL,    },  /* Arrow */
    { "1x4",    "1x4",  NO_WEAPON,  0,      },  /* Dagger */
    { "4x4",    "1x2",  NO_WEAPON,  0,      },  /* 2h sword */
    { "1x1",    "1x3",  NO_WEAPON,  ISMANY | ISMISL,    },  /* Dart */
    { "1x2",    "2x4",  NO_WEAPON,  ISMANY | ISMISL,    },  /* Shuriken */
    { "2x3",    "1x6",  NO_WEAPON,  0,      },  /* Spear */
};

/*
 * missile:
 *  Fire a weapon in a given direction
 */

void missile (THING* weapon, int ydelta, int xdelta)
{
    if (!dropcheck (weapon) || is_current (weapon))
    {
        return;
    }

    weapon = leave_pack (weapon, TRUE, FALSE);
    do_motion (weapon, ydelta, xdelta);

    /*
     * AHA! Here it has hit something.  If it is a wall or a door,
     * or if it misses (combat) the monster, put it on the floor
     */
    bool hit_wall = moat (weapon->o_pos.y, weapon->o_pos.x) == NULL;
    if (hit_wall || !hit_monster (unc (weapon->o_pos), weapon))
    {
        if (hit_wall)
        { 
            endmsg(); /* Specifically when we hit a wall, need to endmsg() here or it'll never get ended */
        }
        fall (weapon, TRUE);
    }
}

/*
 * do_motion:
 *  Do the actual motion on the screen done by an object traveling
 *  across the room
 */

void do_motion (THING *obj, int ydelta, int xdelta)
{
    int ch;

    /*
     * Come fly with us ...
     */
    obj->o_pos = hero;

    for (;;)
    {
        /*
         * Erase the old one
         */
        if (!ce (obj->o_pos, hero) && cansee (unc (obj->o_pos)) && !terse)
        {
            ch = chat (obj->o_pos.y, obj->o_pos.x);

            if (ch == FLOOR && !show_floor())
            {
                ch = ' ';
            }

            mvaddch (obj->o_pos.y, obj->o_pos.x, ch);
        }

        /*
         * Get the new position
         */
        obj->o_pos.y += ydelta;
        obj->o_pos.x += xdelta;

        if (step_ok (ch = winat (obj->o_pos.y, obj->o_pos.x)) && ch != DOOR)
        {
            /*
             * It hasn't hit anything yet, so display it
             * If it alright.
             */
            if (cansee (unc (obj->o_pos)) && !terse)
            {
                mvaddch (obj->o_pos.y, obj->o_pos.x, obj->o_type);
                refresh();
            }

            continue;
        }

        break;
    }
}

/*
 * fall:
 *  Drop an item someplace around here.
 */

void fall (THING *obj, bool pr)
{
    PLACE *pp;
    static coord fpos;

    if (fallpos (&obj->o_pos, &fpos))
    {
        pp = INDEX (fpos.y, fpos.x);
        pp->p_ch = (char) obj->o_type;
        obj->o_pos = fpos;

        if (cansee (fpos.y, fpos.x))
        {
            if (pp->p_monst != NULL)
            {
                pp->p_monst->t_oldch = (char) obj->o_type;
            }
            else
            {
                mvaddch (fpos.y, fpos.x, obj->o_type);
            }
        }

        attach (lvl_obj, obj);
        return;
    }

    if (pr)
    {
        if (has_hit)
        {
            endmsg();
            has_hit = FALSE;
        }

        msg ("the %s vanishes as it hits the ground",
             weap_info[obj->o_which].oi_name);
    }

    discard (obj);
}

/*
 * init_weapon:
 *  Set up the initial goodies for a weapon
 */

void init_weapon (THING *weap, int which)
{
    struct init_weaps *iwp;

    weap->o_type = WEAPON;
    weap->o_which = which;
    iwp = &init_dam[which];
    strncpy (weap->o_damage, iwp->iw_dam, sizeof (weap->o_damage));
    strncpy (weap->o_hurldmg, iwp->iw_hrl, sizeof (weap->o_hurldmg));
    weap->o_launch = iwp->iw_launch;
    weap->o_flags = iwp->iw_flags;
    weap->o_hplus = 0;
    weap->o_dplus = 0;

    if (weap->o_flags & ISMANY)
    {
        weap->o_count = rnd (8) + 8;
        weap->o_group = group++;
    }
    else
    {
        weap->o_count = 1;
        weap->o_group = 0;
    }
}

/*
 * hit_monster:
 *  Does the missile hit the monster?
 */
int hit_monster (int y, int x, THING *obj)
{
    static coord mp;

    mp.y = y;
    mp.x = x;
    return fight (&mp, obj, TRUE);
}

/*
 * num:
 *  Figure out the plus number for armor/weapons
 */
char *num (int n1, int n2, char type)
{
    static char numbuf[10];

    sprintf (numbuf, n1 < 0 ? "%d" : "+%d", n1);

    if (type == WEAPON)
    {
        sprintf (&numbuf[strlen (numbuf)], n2 < 0 ? ",%d" : ",+%d", n2);
    }

    return numbuf;
}

/*
 * wield:
 *  Pull out a certain weapon
 */

void wield(THING* obj)
{
    THING *oweapon;
    char *sp;

    oweapon = cur_weapon;

    if (!dropcheck (cur_weapon))
    {
        cur_weapon = oweapon;
        return;
    }

    cur_weapon = oweapon;

    if (obj == NULL || is_current (obj))
    {
        after = FALSE;
        return;
    }

    sp = inv_name (obj, TRUE);
    cur_weapon = obj;

    if (!terse)
    {
        addmsg ("you are now ");
    }

    msg ("wielding %s (%c)", sp, obj->o_packch);
}

/*
 * fallpos:
 *  Pick a random position around the give (y, x) coordinates
 */
bool fallpos (coord *pos, coord *newpos)
{
    int y, x, cnt, ch;

    cnt = 0;

    for (y = pos->y - 1; y <= pos->y + 1; y++)
        for (x = pos->x - 1; x <= pos->x + 1; x++)
        {
            /*
             * check to make certain the spot is empty, if it is,
             * put the object there, set it in the floor list
             * and re-draw the room if he can see it
             */
            if (y == hero.y && x == hero.x)
            {
                continue;
            }

            if (((ch = chat (y, x)) == FLOOR || ch == PASSAGE)
                    && rnd (++cnt) == 0)
            {
                newpos->y = y;
                newpos->x = x;
            }
        }

    return (bool) (cnt != 0);
}
