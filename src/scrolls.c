/*
 * Read a scroll and let it happen
 *
 * @(#)scrolls.c    4.44 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/*
 * read_scroll:
 *  Read a scroll from the pack and do the appropriate thing
 */

void read_scroll(THING* obj)
{
    PLACE *pp;
    int y, x;
    char ch;
    int i;
    bool discardit = FALSE;
    struct room *cur_room;
    THING *orig_obj;
    static coord mp;


    /* I think these two checks are unnecessary */
    if (obj == NULL)
    {
        return;
    }

    if (obj->o_type != SCROLL)
    {
        if (!terse)
        {
            msg ("there is nothing on it to read");
        }
        else
        {
            msg ("nothing to read");
        }

        return;
    }

    /*
     * Calculate the effect it has on the poor guy.
     */
    if (obj == cur_weapon)
    {
        cur_weapon = NULL;
    }

    /*
     * Get rid of the thing
     */
    discardit = (bool) (obj->o_count == 1);
    leave_pack (obj, FALSE, FALSE);
    orig_obj = obj;

    switch (obj->o_which)
    {
    case S_CONFUSE:
        /*
         * Scroll of monster confusion.  Give him that power.
         */
        player.t_flags |= CANHUH;
        msg ("your hands begin to glow %s", pick_color ("red"));
        break;
    case S_ARMOR:

        if (cur_armor != NULL)
        {
            cur_armor->o_arm--;
            cur_armor->o_flags &= ~ISCURSED;
            msg ("your armor glows %s for a moment", pick_color ("silver"));
        }

        break;
    case S_HOLD:
        /*
         * Hold monster scroll.  Stop all monsters within two spaces
         * from chasing after the hero.
         */

        ch = 0;

        for (x = hero.x - 2; x <= hero.x + 2; x++)
            if (x >= 0 && x < NUMCOLS)
                for (y = hero.y - 2; y <= hero.y + 2; y++)
                    if (y >= 0 && y <= NUMLINES - 1)
                        if ((obj = moat (y, x)) != NULL && on (*obj, ISRUN))
                        {
                            obj->t_flags &= ~ISRUN;
                            obj->t_flags |= ISHELD;
                            ch++;
                        }

        msg ("the monsters around you freeze");
        scr_info[S_HOLD].oi_know = TRUE;

        break;
    case S_SLEEP:
        /*
         * Scroll which makes you fall asleep
         */
        scr_info[S_SLEEP].oi_know = TRUE;
        no_command += rnd (SLEEPTIME) + 4;
        player.t_flags &= ~ISRUN;
        msg ("you fall asleep");
        break;
    case S_CREATE:
        /*
         * Create a monster:
         * First look in a circle around him, next try his room
         * otherwise give up
         */
        i = 0;

        for (y = hero.y - 1; y <= hero.y + 1; y++)
            for (x = hero.x - 1; x <= hero.x + 1; x++)

                /*
                 * Don't put a monster in top of the player.
                 */
                if (y == hero.y && x == hero.x)
                {
                    continue;
                }
        /*
         * Or anything else nasty
         */
                else if (step_ok (ch = winat (y, x)))
                {
                    if (ch == SCROLL
                            && find_obj (y, x)->o_which == S_SCARE)
                    {
                        continue;
                    }
                    else if (rnd (++i) == 0)
                    {
                        mp.y = y;
                        mp.x = x;
                    }
                }

        if (i == 0)
        {
            msg ("you hear a faint cry of anguish in the distance");
        }
        else
        {
            obj = new_item();
            // new_monster (obj, randmonster (FALSE), &mp, FALSE);
            spawn_monster (obj, FALSE, &mp, FALSE);
            msg("a monster appears");
        }

        break;
    
    case S_ID:
        {
            /*
             * Identify, let him figure something out
             */
            scr_info[obj->o_which].oi_know = TRUE;
            msg ("this scroll is an identify scroll");
            // wait for user to hit enter or space
            char ch = 0;
            while (ch != '\n' && ch != ' ' && ch != '\r')
            {
                ch = readchar();
            }
            whatis (TRUE, 0);
        }

        break;
    case S_MAP:
        /*
         * Scroll of magic mapping.
         */
        scr_info[S_MAP].oi_know = TRUE;
        msg ("oh, now this scroll has a map on it");

        /*
         * take all the things we want to keep hidden out of the window
         */
        for (y = 1; y < NUMLINES - 1; y++)
            for (x = 0; x < NUMCOLS; x++)
            {
                pp = INDEX (y, x);

                switch (ch = pp->p_ch)
                {
                case DOOR:
                case STAIRS:
                    break;

                case '-':
                case '|':
                    if (! (pp->p_flags & F_REAL))
                    {
                        ch = pp->p_ch = DOOR;
                        pp->p_flags |= F_REAL;
                    }

                    break;

                case ' ':
                    if (pp->p_flags & F_REAL)
                    {
                        goto def;
                    }

                    pp->p_flags |= F_REAL;
                    ch = pp->p_ch = PASSAGE;

                /* FALLTHROUGH */

                case PASSAGE:
                pass:
                    if (! (pp->p_flags & F_REAL))
                    {
                        pp->p_ch = PASSAGE;
                    }

                    pp->p_flags |= (F_SEEN | F_REAL);
                    ch = PASSAGE;
                    break;

                case FLOOR:
                    if (pp->p_flags & F_REAL)
                    {
                        ch = ' ';
                    }
                    else
                    {
                        ch = TRAP;
                        pp->p_ch = TRAP;
                        pp->p_flags |= (F_SEEN | F_REAL);
                    }

                    break;

                default:
                def:
                    if (pp->p_flags & F_PASS)
                    {
                        goto pass;
                    }

                    ch = ' ';
                    break;
                }

                if (ch != ' ')
                {
                    if ((obj = pp->p_monst) != NULL)
                    {
                        obj->t_oldch = ch;
                    }

                    if (obj == NULL || !on (player, SEEMONST))
                    {
                        mvaddch (y, x, ch);
                    }
                }
            }

        break;
    case S_FDET:
        /*
         * Potion of food detection
         */
        ch = FALSE;
        wclear (hw);

        for (obj = lvl_obj; obj != NULL; obj = next (obj))
            if (obj->o_type == FOOD)
            {
                ch = TRUE;
                wmove (hw, obj->o_pos.y, obj->o_pos.x);
                waddch (hw, FOOD);
            }

        if (ch)
        {
            scr_info[S_FDET].oi_know = TRUE;
            show_win ("Your nose tingles and you smell food.--More--");
        }
        else
        {
            msg ("your nose tingles, you smell no food");
        }

        break;
    case S_TELEP:
        /*
         * Scroll of teleportation:
         * Make him dissapear and reappear
         */
        {
            cur_room = proom;
            teleport();

            if (cur_room != proom)
            {
                scr_info[S_TELEP].oi_know = TRUE;
            }
            msg("you've been teleported!");
        }
        break;
    case S_ENCH:

        if (cur_weapon == NULL || cur_weapon->o_type != WEAPON)
        {
            msg ("your weapon would have glowed %s, but none is equipped", pick_color ("blue"));
        }
        else
        {
            cur_weapon->o_flags &= ~ISCURSED;

            if (rnd (2) == 0)
            {
                cur_weapon->o_hplus++;
            }
            else
            {
                cur_weapon->o_dplus++;
            }

            msg ("your %s glows %s for a moment",
                 weap_info[cur_weapon->o_which].oi_name, pick_color ("blue"));
        }

        break;
    case S_SCARE:
        /*
         * Reading it is a mistake and produces laughter at her
         * poor boo boo.
         */
        msg ("you hear maniacal laughter in the distance");
        break;
    case S_REMOVE:
        uncurse (cur_armor);
        uncurse (cur_weapon);
        uncurse (cur_ring[LEFT]);
        uncurse (cur_ring[RIGHT]);
        msg (choose_str ("you feel in touch with the Universal Onenes",
                         "you feel all curses lifting"));
        break;
    case S_AGGR:
        /*
         * This scroll aggravates all the monsters on the current
         * floor and sets them running towards the hero
         */
        aggravate();
        msg ("you hear footsteps rapidly approaching");
        break;
    case S_PROTECT:

        if (cur_armor != NULL)
        {
            cur_armor->o_flags |= ISPROT;
            msg ("your armor is covered by a shimmering %s shield",
                 pick_color ("gold"));
        }
        else
        {
            msg ("your armor would have shimmered %s, but none is equipped", pick_color ("gold"));
        }
        break;
    
    default:
        msg("what a puzzling scroll");
        break;
        
#ifdef MASTER
    otherwise:
        msg ("what a puzzling scroll!");
        return;
#endif
    }

    obj = orig_obj;
    look (TRUE);    /* put the result of the scroll on the screen */
    status();

    // call_it (&scr_info[obj->o_which]);
    /* Just let the poor guy know for next time */
    scr_info[obj->o_which].oi_know = TRUE;

    if (discardit)
    {
        discard (obj);
    }
}

/*
 * uncurse:
 *  Uncurse an item
 */

void uncurse (THING *obj)
{
    if (obj != NULL)
    {
        obj->o_flags &= ~ISCURSED;
    }
}
