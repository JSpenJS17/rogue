/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c    4.57 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <curses.h>
#include <curl/curl.h>
#include "rogue.h"
#include "score.h"
#include "cJSON.h"

/* Structs for global leaderboard storage */
struct ResponseBuffer {
    char *data;
    size_t size;
};

typedef struct TopScore {
	char name[64];
	int score;
	char reason[64];
	int floor;
} TopScore;

int placement = 0; /* The hero's placement against the world */
int num_topscores = 0; /* The number of the top 10 scores that are available */

/*
 * parse:
 *  Parse through the JSON returned by the score database
 */
TopScore* parse(char *str) 
{
    /* Parse through a JSON of the format: 
    {
        "placement": 1,
        "top_scores": {
        "1": {
            "name": "pierce",
            "score": 1000,
            "reason": "died on floor 3 by a kestrel."
        },
        "2": {
            "name": "billy",
            "score": 900,
            "reason": "died on floor 2 by a hobgoblin"
        }
        }
    }
    Returns a list of the top ten TopScore structs in order
    */

    /* Allocate memory for the top scores */
    TopScore *top_scores = malloc(10 * sizeof(TopScore));
    if (!top_scores) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    memset(top_scores, 0, 10 * sizeof(TopScore));

    /* Parse the JSON */
    cJSON *root = cJSON_Parse(str);
    if (!root) {
        fprintf(stderr, "JSON parsing error: %s\n", cJSON_GetErrorPtr());
        free(top_scores);
        return NULL;
    }

    /* Get the hero's placement */
    cJSON *placement_dict = cJSON_GetObjectItem(root, "placement");
    if (!placement_dict || !cJSON_IsNumber(placement_dict)) {
        fprintf(stderr, "placement missing or not a number\n");
        cJSON_Delete(root);
        free(placement_dict);
        return NULL;
    }

    if (cJSON_IsNumber(placement_dict)) {
        placement = placement_dict->valueint;
    } else {
        fprintf(stderr, "placement is not a number\n");
        cJSON_Delete(root);
        free(top_scores);
        return NULL;
    }

    /* Get the top scores object */
    cJSON *score_dict = cJSON_GetObjectItem(root, "top_scores");
    if (!score_dict || !cJSON_IsObject(score_dict)) {
        fprintf(stderr, "top_scores missing or not an object\n");
        cJSON_Delete(root);
        free(top_scores);
        return NULL;
    }

    cJSON *entry = NULL;

    /* Iterate through each top score and add it to the list as a TopScore struct */
    cJSON_ArrayForEach(entry, score_dict) {
        if (num_topscores >= 10)
        {
            break;
        }

        cJSON *name = cJSON_GetObjectItem(entry, "name");
        cJSON *score = cJSON_GetObjectItem(entry, "score");
        cJSON *reason = cJSON_GetObjectItem(entry, "reason");

        if (name && cJSON_IsString(name) &&
            score && cJSON_IsNumber(score) &&
            reason && cJSON_IsString(reason)) {

            strncpy(top_scores[num_topscores].name, name->valuestring, sizeof(top_scores[num_topscores].name) - 1);
            top_scores[num_topscores].score = score->valueint;
            strncpy(top_scores[num_topscores].reason, reason->valuestring, sizeof(top_scores[num_topscores].reason) - 1);
            num_topscores++;
        }
    }

    cJSON_Delete(root);
    return top_scores;
}

/* 
 * write_callback:
 *  used as a callback for CURL to write to a buffer instead of stdout
 */
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    struct ResponseBuffer *buf = (struct ResponseBuffer *)userdata;

    char *new_data = realloc(buf->data, buf->size + total_size + 1); // +1 for null terminator
    if (new_data == NULL) {
        return 0; // out of memory
    }

    buf->data = new_data;
    memcpy(&(buf->data[buf->size]), ptr, total_size);
    buf->size += total_size;
    buf->data[buf->size] = '\0'; // null terminate

    return total_size;
}

static char *rip[] =
{
    "                       __________\n",
    "                      /          \\\n",
    "                     /    REST    \\\n",
    "                    /      IN      \\\n",
    "                   /     PEACE      \\\n",
    "                  /                  \\\n",
    "                  |                  |\n",
    "                  |                  |\n",
    "                  |   killed by a    |\n",
    "                  |                  |\n",
    "                  |       1980       |\n",
    "                 *|     *  *  *      | *\n",
    "         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
    0
};

/*
 * score:
 *  Figure score and post it.
 */
/* VARARGS2 */

void score (int amount, int flags, char monst)
{
    SCORE *scp;
    int i;
    SCORE *sc2;
    SCORE *top_ten, *endp;
# ifdef MASTER
    int prflags = 0;
# endif
    void (*fp) (int);
    unsigned int uid;
    static char *reason[] =
    {
        "killed",
        "quit",
        "A total winner",
        "killed with Amulet"
    };

    start_score();

    if (flags >= 0
#ifdef MASTER
            || wizard
#endif
       )
    {
        mvaddstr (LINES - 1, 0, "[Press return to continue]");
        refresh();
        wgetnstr (stdscr, prbuf, 80);
        endwin();
        printf ("\n");
        resetltchars();
        /*
         * free up space to "guarantee" there is space for the top_ten
         */
        delwin (stdscr);
        delwin (curscr);

        if (hw != NULL)
        {
            delwin (hw);
        }
    }

    top_ten = (SCORE *) malloc (numscores * sizeof (SCORE));
    endp = &top_ten[numscores];

    for (scp = top_ten; scp < endp; scp++)
    {
        scp->sc_score = 0;

        for (i = 0; i < MAXSTR; i++)
        {
            scp->sc_name[i] = (unsigned char) rnd (255);
        }

        scp->sc_flags = RN;
        scp->sc_level = RN;
        scp->sc_monster = (unsigned short) RN;
        scp->sc_uid = RN;
    }

    signal (SIGINT, SIG_DFL);

#ifdef MASTER

    if (wizard)
        if (strcmp (prbuf, "names") == 0)
        {
            prflags = 1;
        }
        else if (strcmp (prbuf, "edit") == 0)
        {
            prflags = 2;
        }

#endif
    rd_score (top_ten);
    /*
     * Insert her in list if need be
     */
    sc2 = NULL;

    if (!noscore)
    {
        uid = md_getuid();

        for (scp = top_ten; scp < endp; scp++)
            if (amount > scp->sc_score)
            {
                break;
            }
            else if (!allscore &&   /* only one score per nowin uid */
                     flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
            {
                scp = endp;
            }

        if (scp < endp)
        {
            if (flags != 2 && !allscore)
            {
                for (sc2 = scp; sc2 < endp; sc2++)
                {
                    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
                    {
                        break;
                    }
                }

                if (sc2 >= endp)
                {
                    sc2 = endp - 1;
                }
            }
            else
            {
                sc2 = endp - 1;
            }

            while (sc2 > scp)
            {
                *sc2 = sc2[-1];
                sc2--;
            }

            scp->sc_score = amount;
            strncpy (scp->sc_name, whoami, MAXSTR);
            scp->sc_flags = flags;

            if (flags == 2)
            {
                scp->sc_level = max_floor;
            }
            else
            {
                scp->sc_level = cur_floor;
            }

            scp->sc_monster = monst;
            scp->sc_uid = uid;
            sc2 = scp;
        }
    }

    /*
     * Print the list
     */
    if (flags != -1)
    {
        putchar ('\n');
    }

    printf ("Top %s Local %s:\n", Numname, allscore ? "Scores" : "Rogueists");
    printf ("   Score Name\n");

    for (scp = top_ten; scp < endp; scp++)
    {
        if (scp->sc_score)
        {
            if (sc2 == scp)
            {
                md_raw_standout();
            }

            printf ("%2d %5d %s: %s on floor %d", (int) (scp - top_ten + 1),
                    scp->sc_score, scp->sc_name, reason[scp->sc_flags],
                    scp->sc_level);

            if (scp->sc_flags == 0 || scp->sc_flags == 3)
            {
                printf (" by %s", killname ((char) scp->sc_monster, TRUE));
            }

#ifdef MASTER

            if (prflags == 1)
            {
                printf (" (%s)", md_getrealname (scp->sc_uid));
            }
            else if (prflags == 2)
            {
                fflush (stdout);
                (void) fgets (prbuf, 10, stdin);

                if (prbuf[0] == 'd')
                {
                    for (sc2 = scp; sc2 < endp - 1; sc2++)
                    {
                        *sc2 = * (sc2 + 1);
                    }

                    sc2 = endp - 1;
                    sc2->sc_score = 0;

                    for (i = 0; i < MAXSTR; i++)
                    {
                        sc2->sc_name[i] = (char) rnd (255);
                    }

                    sc2->sc_flags = RN;
                    sc2->sc_level = RN;
                    sc2->sc_monster = (unsigned short) RN;
                    scp--;
                }
            }
            else
#endif /* MASTER */
                printf (".");

            if (sc2 == scp)
            {
                md_raw_standend();
            }

            putchar ('\n');
        }
        else
        {
            break;
        }
    }

    /*
     * Send her to the server (http post using curl.h to piercelane2.ddns.net) if she was the top score
     */
    if (sc2 == top_ten) 
    {
        // Initialize a curl handle
        CURLcode res;
        CURL* curl;
        char json_data[256];
        char rsn[64];
        struct curl_slist *headers = NULL;

        /* Set up CURL response buffer */
        struct ResponseBuffer response;
        response.data = malloc(1); // initial empty buffer
        response.size = 0;

        curl = curl_easy_init();

        if (curl) 
        {
            /*
             * Send up our score data
             * JSON Format:
             *  {
             *    "name": [score, end_reason]
             *  }
             */
            if (sc2->sc_flags == 0 || sc2->sc_flags == 3)
            {
                sprintf (rsn, "%s on floor %d by %s.", 
                    reason[sc2->sc_flags], sc2->sc_level,
                    killname ((char) sc2->sc_monster, TRUE));
            } 
            else 
            {
                sprintf (rsn, "%s on floor %d.", 
                    reason[sc2->sc_flags], sc2->sc_level);
            }
            

            sprintf(json_data, "{\"%s\": [%d, \"%s\"]}", 
                sc2->sc_name, sc2->sc_score, rsn);

            /* Set up CURL */
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_URL, "http://piercelane2.ddns.net:5000/");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            /* Set up the write function and buffer */
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

            /* Do the deed */
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK) 
            {
                fprintf(stderr, "Failed to communicate to scoreboard server: %s\n", curl_easy_strerror(res));
            } 
            else
            {
                TopScore* top_ten = parse(response.data);
                if (top_ten) 
                {
                    printf("\n\nTop Ten Global Scores:\n");
                    printf("   Score Name\n");
                    for (int i = 0; i < num_topscores; i++)
                    {
                        printf("%2d %5d %s: %s\n", i + 1, top_ten[i].score, top_ten[i].name, top_ten[i].reason);
                    }
                    printf("\nYou placed number %d globally.\n\n", placement);
                    free(top_ten);
                } 
                else 
                {
                    printf("Failed to parse scores.\n");
                }
            }

            // Cleanup
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            free(response.data);
        }
    }

    /*
     * Update the list file
     */
    if (sc2 != NULL)
    {
        if (lock_sc())
        {
            fp = signal (SIGINT, SIG_IGN);
            wr_score (top_ten);
            unlock_sc();
            signal (SIGINT, fp);
        }
    }
}

/*
 * death:
 *  Do something really fun when he dies
 */

void death (char monst)
{
    char **dp, *killer;
    struct tm *lt;
    static time_t date;
    struct tm *localtime();

    signal (SIGINT, SIG_IGN);
    purse -= purse / 10;
    signal (SIGINT, leave);
    clear();
    killer = killname (monst, FALSE);

    if (!tombstone)
    {
        mvprintw (LINES - 2, 0, "Killed by ");
        killer = killname (monst, FALSE);

        if (monst != 's' && monst != 'h')
        {
            printw ("a%s ", vowelstr (killer));
        }

        printw ("%s with %d gold", killer, purse);
    }
    else
    {
        time (&date);
        lt = localtime (&date);
        move (8, 0);
        dp = rip;

        while (*dp)
        {
            addstr (*dp++);
        }

        mvaddstr (17, center (killer), killer);

        if (monst == 's' || monst == 'h')
        {
            mvaddch (16, 32, ' ');
        }
        else
        {
            mvaddstr (16, 33, vowelstr (killer));
        }

        mvaddstr (14, center (whoami), whoami);
        sprintf (prbuf, "%d Au", purse);
        move (15, center (prbuf));
        addstr (prbuf);
        sprintf (prbuf, "%4d", 1900 + lt->tm_year);
        mvaddstr (18, 26, prbuf);
    }

    move (LINES - 1, 0);
    refresh();
    score (purse, amulet ? 3 : 0, monst);
    printf ("[Press return to continue]");
    fflush (stdout);
    if (fgets (prbuf, 10, stdin) == NULL) {
        prbuf[0] = '\0';
    }
    my_exit (0);
}

/*
 * center:
 *  Return the index to center the given string
 */
int center (char *str)
{
    return 28 - (((int) strlen (str) + 1) / 2);
}

/*
 * total_winner:
 *  Code for a winner
 */

void total_winner()
{
    THING *obj;
    struct obj_info *op;
    int worth = 0;
    int oldpurse;

    clear();
    standout();
    addstr ("                                                               \n");
    addstr ("  @   @               @   @           @          @@@  @     @  \n");
    addstr ("  @   @               @@ @@           @           @   @     @  \n");
    addstr ("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr ("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr ("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr ("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr ("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr ("                                                               \n");
    addstr ("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr ("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr ("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr ("a great profit and are admitted to the Fighters' Guild.\n");
    mvaddstr (LINES - 1, 0, "--Press space to continue--");
    refresh();
    wait_for (' ');
    clear();
    mvaddstr (0, 0, "   Worth  Item\n");
    oldpurse = purse;

    for (obj = pack; obj != NULL; obj = next (obj))
    {
        switch (obj->o_type)
        {
        case FOOD:
            worth = 2 * obj->o_count;
            break;
        case WEAPON:
            worth = weap_info[obj->o_which].oi_worth;
            worth *= 3 * (obj->o_hplus + obj->o_dplus) + obj->o_count;
            obj->o_flags |= ISKNOW;
            break;
        case ARMOR:
            worth = arm_info[obj->o_which].oi_worth;
            worth += (9 - obj->o_arm) * 100;
            worth += (10 * (a_class[obj->o_which] - obj->o_arm));
            obj->o_flags |= ISKNOW;
            break;
        case SCROLL:
            worth = scr_info[obj->o_which].oi_worth;
            worth *= obj->o_count;
            op = &scr_info[obj->o_which];

            if (!op->oi_know)
            {
                worth /= 2;
            }

            op->oi_know = TRUE;
            break;
        case POTION:
            worth = pot_info[obj->o_which].oi_worth;
            worth *= obj->o_count;
            op = &pot_info[obj->o_which];

            if (!op->oi_know)
            {
                worth /= 2;
            }

            op->oi_know = TRUE;
            break;
        case RING:
            op = &ring_info[obj->o_which];
            worth = op->oi_worth;

            if (obj->o_which == R_ADDSTR || obj->o_which == R_ADDDAM ||
                    obj->o_which == R_PROTECT || obj->o_which == R_ADDHIT)
            {
                if (obj->o_arm > 0)
                {
                    worth += obj->o_arm * 100;
                }
                else
                {
                    worth = 10;
                }
            }

            if (! (obj->o_flags & ISKNOW))
            {
                worth /= 2;
            }

            obj->o_flags |= ISKNOW;
            op->oi_know = TRUE;
            break;
        case STICK:
            op = &ws_info[obj->o_which];
            worth = op->oi_worth;
            worth += 20 * obj->o_charges;

            if (! (obj->o_flags & ISKNOW))
            {
                worth /= 2;
            }

            obj->o_flags |= ISKNOW;
            op->oi_know = TRUE;
            break;
        case AMULET:
            worth = 1000;
        }

        if (worth < 0)
        {
            worth = 0;
        }

        printw ("%c) %5d  %s\n", obj->o_packch, worth, inv_name (obj, FALSE));
        purse += worth;
    }

    printw ("   %5d  Gold Pieces          ", oldpurse);
    refresh();
    score (purse, 2, ' ');
    my_exit (0);
}

/*
 * killname:
 *  Convert a code to a monster name
 */
char *killname (char monst, bool doart)
{
    struct h_list *hp;
    char *sp;
    bool article;
    static struct h_list nlist[] =
    {
        {'a',   "arrow",        TRUE},
        {'b',   "bolt",         TRUE},
        {'d',   "dart",         TRUE},
        {'h',   "hypothermia",      FALSE},
        {'s',   "starvation",       FALSE},
        {'\0'}
    };

    if (isupper (monst))
    {
        sp = monsters[monst - 'A'].m_name;
        article = TRUE;
    }
    else
    {
        sp = "Wally the Wonder Badger";
        article = FALSE;

        for (hp = nlist; hp->h_ch; hp++)
            if (hp->h_ch == monst)
            {
                sp = hp->h_desc;
                article = hp->h_print;
                break;
            }
    }

    if (doart && article)
    {
        sprintf (prbuf, "a%s ", vowelstr (sp));
    }
    else
    {
        prbuf[0] = '\0';
    }

    strcat (prbuf, sp);
    return prbuf;
}

/*
 * death_monst:
 *  Return a monster appropriate for a random death.
 */
char death_monst()
{
    static char poss[] =
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'h', 'd', 's',
        ' ' /* This is provided to generate the "Wally the Wonder Badger"
           message for killer */
    };

    return poss[rnd (sizeof poss / sizeof (char))];
}
