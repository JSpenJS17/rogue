/*
 * Read and execute the user commands
 *
 * @(#)command.c    4.73 (Berkeley) 08/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include "rogue.h"

/* 
 * use:
 *  The player wants to use something
 */
void use()
{
    // inventory (pack, 0);
    THING* obj = get_item("use", 0);
    if (obj == NULL) // we hit esc
    {
        return;
    }

    switch (obj->o_type)
    {
        case POTION:
            quaff(obj);
            break;
        case SCROLL:
            read_scroll(obj);
            break;
        case RING:
            if (obj == cur_ring[LEFT])
            {
                ring_off(LEFT);
            }
            else if (obj == cur_ring[RIGHT])
            {
                ring_off(RIGHT);
            }
            else
            {
                ring_on(obj);
            }
            break;
        case STICK:
            if (get_dir())
            {
                do_zap(obj);
            }
            else
            {
                after = FALSE;
            }
            break;
        case FOOD:
            eat(obj);
            break;
        case WEAPON:
            switch (obj->o_which)
            {
                case ARROW:
                    /* 
                     * If the player doesn't have a bow equiped, we can't throw arrows
                     */
                    if (cur_weapon == NULL || cur_weapon->o_which != BOW)
                    {
                        msg("wield a bow to shoot arrows");
                        after = FALSE;
                        return;
                    }
                /* FALLTHROUGH */
                case DART:
                case SHIRAKEN:
                    if (!get_dir())
                    {
                        after = FALSE;
                    }
                    else
                    {
                        missile (obj, delta.y, delta.x);
                    }
                    break;

                /* Non-throwing weapon */
                default:
                    wield(obj);
            }
            break;
        case ARMOR:
            if (obj == cur_armor)
            {
                take_off();
            }
            else
            {
                if (cur_armor != NULL)
                {
                    take_off();
                }
                wear(obj);
            }
            break;
        default:
            msg("you can't use that");
            return;
    }
}

/*
 * command:
 *  Process the user commands
 */
void command()
{
    register char ch;
    register int ntimes = 1;            /* Number of player moves */
    char *fp;
    THING *mp;
    static char countch, direction, newcount = FALSE;

    if (on (player, ISHASTE))
    {
        ntimes++;
    }

    /*
     * Let the daemons start up
     */
    do_daemons (BEFORE);
    do_fuses (BEFORE);

    while (ntimes--)
    {
        again = FALSE;

        if (has_hit)
        {
            endmsg();
            has_hit = FALSE;
        }

        /*
         * these are illegal things for the player to be, so if any are
         * set, someone's been poking in memeory
         */
        if (on (player, ISSLOW | ISBOSS | ISINVIS | ISREGEN | ISTARGET))
        {
            exit (1);
        }

        look (TRUE);

        if (!running)
        {
            door_stop = FALSE;
        }

        status();
        lastscore = purse;
        move (hero.y, hero.x);

        if (! ((running || count) && jump))
        {
            refresh();    /* Draw screen */
        }

        take = 0;
        after = TRUE;
        /*
         * Read command or continue run
         */
#ifdef MASTER

        if (wizard)
        {
            noscore = TRUE;
        }

#endif

        if (!no_command)
        {
            if (running || to_death)
            {
                ch = runch;
            }
            else if (count)
            {
                ch = countch;
            }
            else
            {
                ch = readchar();
                move_on = FALSE;

                if (mpos != 0)      /* Erase message if its there */
                {
                    msg ("");
                }
            }
        }
        else
        {
            ch = '.';
        }

        if (no_command)
        {
            if (--no_command == 0)
            {
                player.t_flags |= ISRUN;
                msg ("you can move again");
            }
        }
        else
        {
            /*
             * check for prefixes
             */
            newcount = FALSE;

            if (isdigit (ch))
            {
                count = 0;
                newcount = TRUE;

                while (isdigit (ch))
                {
                    count = count * 10 + (ch - '0');

                    if (count > 255)
                    {
                        count = 255;
                    }

                    ch = readchar();
                }

                countch = ch;

                /*
                 * turn off count for commands which don't make sense
                 * to repeat
                 */
                switch (ch)
                {
                case CTRL ('W') :
                case CTRL ('A') :
                case CTRL ('S') :
                case CTRL ('D') :
                case 'w':
                case 'a':
                case 's':
                case 'd':
                case 'W':
                case 'A':
                case 'S':
                case 'D':
                
                case 'm':
                case 'q':
                case 'r':
                case 't':
                case 'z':
                case 'C':

#ifdef MASTER
                // case CTRL ('D') :
                // case CTRL ('A') :
#endif
                    break;

                default:
                    count = 0;
                }
            }

            /*
             * execute a command
             */
            if (count && !running)
            {
                count--;
            }

            if (ch != ESCAPE && ! (running || count || to_death))
            {
                l_last_comm = last_comm;
                l_last_dir = last_dir;
                l_last_pick = last_pick;
                last_comm = ch;
                last_dir = '\0';
                last_pick = NULL;
            }

        over:

            switch (ch)
            {
            // case ',':
            //     {
            //         THING *obj = NULL;
            //         int found = 0;

            //         for (obj = lvl_obj; obj != NULL; obj = next (obj))
            //         {
            //             if (obj->o_pos.y == hero.y && obj->o_pos.x == hero.x)
            //             {
            //                 found = 1;
            //                 break;
            //             }
            //         }

            //         if (found)
            //         {
            //             if (levit_check())
            //                 ;
            //             else
            //             {
            //                 pick_up ((char) obj->o_type);
            //             }
            //         }
            //         else
            //         {
            //             if (!terse)
            //             {
            //                 addmsg ("there is ");
            //             }

            //             addmsg ("nothing here");

            //             if (!terse)
            //             {
            //                 addmsg (" to pick up");
            //             }

            //             endmsg();
            //         }
            //     }

            //     break;
            // case '!':
            //     shell();
            //     break;
            case 'a':
                do_move (0, -1);
                break;
            case 's':
                do_move (1, 0);
                break;
            case 'w':
                do_move (-1, 0);
                break;
            case 'd':
                do_move (0, 1);
                break;
            // case 'A':
            //     do_run ('a');
            //     break;
            // case 'S':
            //     do_run ('s');
            //     break;
            // case 'W':
            //     do_run ('w');
            //     break;
            // case 'D':
            //     do_run ('d');
            //     break;

            case 'W':
            case 'A':
            case 'S':
            case 'D':
            {

                if (!on(player, ISBLIND))
                {
                    door_stop = TRUE;
                    firstmove = TRUE;
                }

                if (count && !newcount) {
                    ch = direction;
                }
                else
                {
                    ch = tolower(ch);
                    direction = ch;
                    do_run(ch);
                }
                goto over;
            }
            case 'j':
                if (!get_dir())
                {
                    after = FALSE;
                    break;
                }

                delta.y += hero.y;
                delta.x += hero.x;

                if (((mp = moat (delta.y, delta.x)) == NULL)
                        || ((!see_monst (mp)) && !on (player, SEEMONST)))
                {
                    if (!terse)
                    {
                        addmsg ("I see ");
                    }

                    msg ("no monster there");
                    after = FALSE;
                }
                else if (diag_ok (&hero, &delta))
                {
                    to_death = TRUE;
                    max_hit = 0;
                    mp->t_flags |= ISTARGET;
                    runch = ch = dir_ch;
                    goto over;
                }

                break;
            // case 't':

            //     if (!get_dir())
            //     {
            //         after = FALSE;
            //     }
            //     else
            //     {
            //         missile (delta.y, delta.x);
            //     }

            //     break;
            case 'q':
                drop();
                break;
            // case 'i':
            //     after = FALSE;
            //     inventory (pack, 0, "--Press space to continue--");
            //     break;

            case 'e':  /* new general use item command */
                use();
                break;
            case 'o':
                option();
                after = FALSE;
                break;
            case '>':
                after = FALSE;
                d_level();
                break;
            case '<':
                after = FALSE;
                u_level();
                break;
            case '?':
                after = FALSE;
                help();
                break;
            case 'k':
                search();
                break;
            case CTRL ('P') :
                after = FALSE;
                msg (huh);
                break;
            case CTRL ('R') :
                after = FALSE;
                clearok (curscr, TRUE);
                wrefresh (curscr);
                break;
            case 'v':
                after = FALSE;
                msg ("version %s. (pierce was here)", release);
                break;
            case CTRL ('S'):
                after = FALSE;
                save_game();
                break;
            case ' ':
                after = FALSE;  /* "Legal" illegal command */
                break;
            // case '^':
            //     after = FALSE;

            //     if (get_dir())
            //     {
            //         delta.y += hero.y;
            //         delta.x += hero.x;
            //         fp = &flat (delta.y, delta.x);

            //         if (!terse)
            //         {
            //             addmsg ("You have found ");
            //         }

            //         if (chat (delta.y, delta.x) != TRAP)
            //         {
            //             msg ("no trap there");
            //         }
            //         else if (on (player, ISHALU))
            //         {
            //             msg (tr_name[rnd (NTRAPS)]);
            //         }
            //         else
            //         {
            //             msg (tr_name[*fp & F_TMASK]);
            //             *fp |= F_SEEN;
            //         }
            //     }

#ifdef MASTER
            when '+':
                after = FALSE;

                if (wizard)
                {
                    wizard = FALSE;
                    turn_see (TRUE);
                    msg ("not wizard any more");
                }
                else
                {
                    wizard = passwd();

                    if (wizard)
                    {
                        noscore = TRUE;
                        turn_see (FALSE);
                        msg ("you are suddenly as smart as Ken Arnold in dungeon #%d", dnum);
                    }
                    else
                    {
                        msg ("sorry");
                    }
                }

#endif
                break;
            case ESCAPE:    /* Escape */
                door_stop = FALSE;
                count = 0;
                after = FALSE;
                again = FALSE;
                break;
            // case ')':
            //     current (cur_weapon, "wielding", NULL);
            //     break;
            // case ']':
            //     current (cur_armor, "wearing", NULL);
            //     break;
            // case '=':
            //     current (cur_ring[LEFT], "wearing",
            //              terse ? "(L)" : "on left hand");
            //     current (cur_ring[RIGHT], "wearing",
            //              terse ? "(R)" : "on right hand");
            //     break;
            // case '@':
            //     stat_msg = TRUE;
            //     status();
            //     stat_msg = FALSE;
            //     after = FALSE;
            default:
                 after = FALSE;
                 illcom (ch);
            }

            /*
             * turn off flags if no longer needed
             */
            if (!running)
            {
                door_stop = FALSE;
            }
        }

        /*
         * If he ran into something to take, let him pick it up.
         */
        if (take != 0)
        {
            pick_up (take);
        }

        if (!running)
        {
            door_stop = FALSE;
        }

        if (!after)
        {
            ntimes++;
        }
    }

    do_daemons (AFTER);
    do_fuses (AFTER);

    if (ISRING (LEFT, R_SEARCH))
    {
        search();
    }
    else if (ISRING (LEFT, R_TELEPORT) && rnd (50) == 0)
    {
        teleport();
    }

    if (ISRING (RIGHT, R_SEARCH))
    {
        search();
    }
    else if (ISRING (RIGHT, R_TELEPORT) && rnd (50) == 0)
    {
        teleport();
    }
}

/*
 * illcom:
 *  What to do with an illegal command
 */
void illcom (int ch)
{
    save_msg = FALSE;
    count = 0;
    msg ("illegal command '%s'", unctrl (ch));
    save_msg = TRUE;
}

/*
 * search:
 *  player gropes about him to find hidden things.
 */
void search()
{
    register int y, x;
    register char *fp;
    register int ey, ex;
    int probinc;
    bool found;

    ey = hero.y + 1;
    ex = hero.x + 1;
    probinc = (on (player, ISHALU) ? 3 : 0);
    probinc += (on (player, ISBLIND) ? 2 : 0);
    found = FALSE;

    for (y = hero.y - 1; y <= ey; y++)
        for (x = hero.x - 1; x <= ex; x++)
        {
            if (y == hero.y && x == hero.x)
            {
                continue;
            }

            fp = &flat (y, x);

            if (! (*fp & F_REAL))
                switch (chat (y, x))
                {
                case '|':
                case '-':
                    if (rnd (5 + probinc) != 0)
                    {
                        break;
                    }

                    chat (y, x) = DOOR;
                    msg ("a secret door");
                foundone:
                    found = TRUE;
                    *fp |= F_REAL;
                    count = FALSE;
                    running = FALSE;
                    break;

                case FLOOR:
                    if (rnd (2 + probinc) != 0)
                    {
                        break;
                    }

                    chat (y, x) = TRAP;

                    if (!terse)
                    {
                        addmsg ("you found ");
                    }

                    if (on (player, ISHALU))
                    {
                        msg (tr_name[rnd (NTRAPS)]);
                    }
                    else
                    {
                        msg (tr_name[*fp & F_TMASK]);
                        *fp |= F_SEEN;
                    }

                    goto foundone;
                    break;

                case ' ':
                    if (rnd (3 + probinc) != 0)
                    {
                        break;
                    }

                    chat (y, x) = PASSAGE;
                    goto foundone;
                }
        }

    if (found)
    {
        look (FALSE);
    }
}

/*
 * help:
 *  Give single character help, or the whole mess if he wants it
 */
void help()
{
    register struct h_list *strp;
    register int numprint, cnt;

    mpos = 0;

    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    numprint = 0;

    for (strp = helpstr; strp->h_desc != NULL; strp++)
        if (strp->h_print)
        {
            numprint++;
        }

    if (numprint & 01)      /* round odd numbers up */
    {
        numprint++;
    }

    numprint /= 2;

    if (numprint > LINES - 1)
    {
        numprint = LINES - 1;
    }

    wclear (hw);
    cnt = 0;

    for (strp = helpstr; strp->h_desc != NULL; strp++)
        if (strp->h_print)
        {
            wmove (hw, cnt % numprint, cnt >= numprint ? COLS / 2 : 0);

            if (strp->h_ch)
            {
                waddstr (hw, unctrl (strp->h_ch));
            }

            waddstr (hw, strp->h_desc);

            if (++cnt >= numprint * 2)
            {
                break;
            }
        }

    wmove (hw, LINES - 1, 0);
    waddstr (hw, "--Press space to continue--");
    wrefresh (hw);
    wait_for (' ');
    clearok (stdscr, TRUE);
    /*
        refresh();
    */
    msg ("");
    touchwin (stdscr);
    wrefresh (stdscr);
}

/*
 * identify:
 *  Tell the player what a certain thing is.
 */
void identify()
{
    register int ch;
    register struct h_list *hp;
    register char *str;
    static struct h_list ident_list[] =
    {
        {'|',       "wall of a room",       FALSE},
        {'-',       "wall of a room",       FALSE},
        {GOLD,      "gold",             FALSE},
        {STAIRS,    "a staircase",          FALSE},
        {DOOR,      "door",             FALSE},
        {FLOOR,     "room floor",           FALSE},
        {PLAYER,    "you",              FALSE},
        {PASSAGE,   "passage",          FALSE},
        {TRAP,      "trap",             FALSE},
        {POTION,    "potion",           FALSE},
        {SCROLL,    "scroll",           FALSE},
        {FOOD,      "food",             FALSE},
        {WEAPON,    "weapon",           FALSE},
        {' ',       "solid rock",           FALSE},
        {ARMOR,     "armor",            FALSE},
        {AMULET,    "the Amulet of Yendor",     FALSE},
        {RING,      "ring",             FALSE},
        {STICK,     "wand or staff",        FALSE},
        {'\0'}
    };

    msg ("what do you want identified? ");
    ch = readchar();
    mpos = 0;

    if (ch == ESCAPE)
    {
        msg ("");
        return;
    }

    if (isupper (ch))
    {
        str = monsters[ch - 'A'].m_name;
    }
    else
    {
        str = "unknown character";

        for (hp = ident_list; hp->h_ch != '\0'; hp++)
            if (hp->h_ch == ch)
            {
                str = hp->h_desc;
                break;
            }
    }

    msg ("'%s': %s", unctrl (ch), str);
}

/*
 * d_level:
 *  He wants to go down a floor
 */
void d_level()
{
    if (levit_check())
    {
        return;
    }

    if (chat (hero.y, hero.x) != STAIRS)
    {
        msg ("I see no way down");
    }
    else
    {
        // loop through monster list to see if the boss is still alive
        THING* tp = NULL;
        for (tp = mlist; tp != NULL; tp = next (tp))
        {
            if (tp->t_flags & ISBOSS)
            {
                msg ("The boss magically blocks your way down!");
                return;
            }
        }
        cur_floor++;
        seenstairs = FALSE;
        new_level();
    }
}

/*
 * u_level:
 *  He wants to go up a floor
 */
void u_level()
{
    if (levit_check())
    {
        return;
    }

    if (chat (hero.y, hero.x) == STAIRS)
        if (amulet)
        {
            cur_floor--;

            if (cur_floor == 0)
            {
                total_winner();
            }

            new_level();
            msg ("you feel a wrenching sensation in your gut");
        }
        else
        {
            msg ("your way is magically blocked");
        }
    else
    {
        msg ("I see no way up");
    }
}

/*
 * levit_check:
 *  Check to see if she's levitating, and if she is, print an
 *  appropriate message.
 */
bool levit_check()
{
    if (!on (player, ISLEVIT))
    {
        return FALSE;
    }

    msg ("You can't.  You're floating off the ground!");
    return TRUE;
}

/*
 * call:
 *  Allow a user to call a potion, scroll, or ring something
 */
void call()
{
    register THING *obj;
    register struct obj_info *op = NULL;
    register char **guess, *elsewise = NULL;
    register bool *know;

    obj = get_item ("call", CALLABLE);

    /*
     * Make certain that it is somethings that we want to wear
     */
    if (obj == NULL)
    {
        return;
    }

    switch (obj->o_type)
    {
    case RING:
        op = &ring_info[obj->o_which];
        elsewise = r_stones[obj->o_which];
        goto norm;
        break;
    case POTION:
        op = &pot_info[obj->o_which];
        elsewise = p_colors[obj->o_which];
        goto norm;
        break;
    case SCROLL:
        op = &scr_info[obj->o_which];
        elsewise = s_names[obj->o_which];
        goto norm;
        break;
    case STICK:
        op = &ws_info[obj->o_which];
        elsewise = ws_made[obj->o_which];
    norm:
        know = &op->oi_know;
        guess = &op->oi_guess;

        if (*guess != NULL)
        {
            elsewise = *guess;
        }

        break;
    case FOOD:
        msg ("you can't call that anything");
        return;
    breakdefault:
        guess = &obj->o_label;
        know = NULL;
        elsewise = obj->o_label;
    }

    if (know != NULL && *know)
    {
        msg ("that has already been identified");
        return;
    }

    if (elsewise != NULL && elsewise == *guess)
    {
        if (!terse)
        {
            addmsg ("Was ");
        }

        msg ("called \"%s\"", elsewise);
    }

    if (terse)
    {
        msg ("call it: ");
    }
    else
    {
        msg ("what do you want to call it? ");
    }

    if (elsewise == NULL)
    {
        strcpy (prbuf, "");
    }
    else
    {
        strcpy (prbuf, elsewise);
    }

    if (get_str (prbuf, stdscr) == NORM)
    {
        if (*guess != NULL)
        {
            free (*guess);
        }

        *guess = malloc ((unsigned int) strlen (prbuf) + 1);
        strcpy (*guess, prbuf);
    }
}

/*
 * current:
 *  Print the current weapon/armor
 */
void current (THING *cur, char *how, char *where)
{
    after = FALSE;

    if (cur != NULL)
    {
        if (!terse)
        {
            addmsg ("you are %s (", how);
        }

        inv_describe = FALSE;
        addmsg ("%c) %s", cur->o_packch, inv_name (cur, TRUE));
        inv_describe = TRUE;

        if (where)
        {
            addmsg (" %s", where);
        }

        endmsg();
    }
    else
    {
        if (!terse)
        {
            addmsg ("you are ");
        }

        addmsg ("%s nothing", how);

        if (where)
        {
            addmsg (" %s", where);
        }

        endmsg();
    }
}
