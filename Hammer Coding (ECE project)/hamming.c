/*
 * Hamming Helpers: a friendly Hamming(7,4) encoder / error-fixer
 *
 * Build:  gcc -std=c99 -Wall -o hamming hamming.c
 * Run:    ./hamming                  (interactive menu)
 *         ./hamming encode 1011      (quick encode)
 *         ./hamming correct 0110001  (quick fix)
 *         ./hamming demo | story     (guided demo / the story)
 *
 * Tip: set NO_COLOR=1 if your terminal shows strange symbols.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

#define RST "\033[0m"
#define BLD "\033[1m"
#define DIM "\033[2m"
#define RED "\033[91m"
#define GRN "\033[92m"
#define YEL "\033[93m"
#define BLU "\033[94m"
#define CYN "\033[96m"

static int color_on = 1;
static int interactive = 0;
static int legend_shown = 0;
#define K(x) (color_on ? (x) : "")

/* Three helper "buddies". Buddy g owns seat (1<<g) and watches 4 seats. */
static const int GROUP[3][4] = {{1, 3, 5, 7}, {2, 3, 6, 7}, {4, 5, 6, 7}};
static const char *BNAME[3] = {"Ruby", "Bluey", "Leafy"};
static const char *BCOL[3] = {RED, BLU, GRN};

/* ---------- small helpers ---------- */

static int is_parity_seat(int s) { return (s & (s - 1)) == 0; }
static int buddy_of_seat(int s) { return s == 1 ? 0 : s == 2 ? 1 : 2; }

static int is_bits(const char *s, size_t n) {
    if (strlen(s) != n) return 0;
    for (size_t i = 0; i < n; i++)
        if (s[i] != '0' && s[i] != '1') return 0;
    return 1;
}

static int ask(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, (int)size, stdin)) return -1;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
}

static void pause_for_enter(void) {
    char tmp[16];
    if (!interactive) return;
    ask("\n  (press Enter to continue) ", tmp, sizeof tmp);
}

static void center(const char *s, int w) {
    int l = (int)strlen(s), left = (w - l) / 2;
    printf("%*s%s%*s", left, "", s, w - l - left, "");
}

static void heading(const char *text) {
    printf("\n  %s%s== %s ==%s\n", K(BLD), K(CYN), text, K(RST));
}

static void buddy_label(int g) {
    printf("%s%s%s%s", K(BLD), K(BCOL[g]), BNAME[g], K(RST));
}

/* ---------- the math (same as the original program) ---------- */

static void encode(const int d[4], int b[8]) {
    b[0] = 0;
    b[3] = d[0]; b[5] = d[1]; b[6] = d[2]; b[7] = d[3];
    b[1] = (b[3] + b[5] + b[7]) % 2;
    b[2] = (b[3] + b[6] + b[7]) % 2;
    b[4] = (b[5] + b[6] + b[7]) % 2;
}

/* ---------- drawing ---------- */

/* Draws the 7 seats as bulbs. mark = seat to point at (0 = none). */
static void show_row(const int b[8], int mark) {
    printf("\n");
    for (int line = 0; line < 7; line++) {
        printf("  ");
        for (int i = 1; i <= 7; i++) {
            int par = is_parity_seat(i), lit = b[i];
            const char *bc = par ? CYN : DIM;
            char buf[16];
            switch (line) {
            case 0:
                printf("%s%s%s", K(bc), par ? "╔═════╗" : "┌─────┐", K(RST));
                break;
            case 1:
                printf("%s%s%s  %s%s%s  %s%s%s", K(bc), par ? "║" : "│", K(RST),
                       K(lit ? YEL : DIM), lit ? "●" : "○", K(RST),
                       K(bc), par ? "║" : "│", K(RST));
                break;
            case 2:
                printf("%s%s%s", K(bc), par ? "╚═════╝" : "└─────┘", K(RST));
                break;
            case 3:
                snprintf(buf, sizeof buf, "seat %d", i);
                printf("%s", K(BLD)); center(buf, 7); printf("%s", K(RST));
                break;
            case 4:
                if (par) {
                    int g = buddy_of_seat(i);
                    printf("%s", K(BCOL[g])); center(BNAME[g], 7); printf("%s", K(RST));
                } else {
                    printf("%s", K(DIM)); center("you", 7); printf("%s", K(RST));
                }
                break;
            case 5:
                printf("  ");
                for (int g = 0; g < 3; g++) {
                    int member = 0;
                    for (int k = 0; k < 4; k++) if (GROUP[g][k] == i) member = 1;
                    if (member) printf("%s●%s", K(BCOL[g]), K(RST));
                    else printf("%s·%s", K(DIM), K(RST));
                }
                printf("  ");
                break;
            case 6:
                if (mark == i) printf("%s%s  ▲▲▲  %s", K(BLD), K(RED), K(RST));
                else printf("       ");
                break;
            }
            printf(" ");
        }
        printf("\n");
    }
    if (!legend_shown) {
        legend_shown = 1;
        printf("  %sBright bulb = ON, dim circle = OFF. Double border = helper bulb. Single border = your bulb.%s\n", K(DIM), K(RST));
        printf("  %sColored dots under a seat show which buddies watch it.%s\n", K(DIM), K(RST));
    }
}

static void show_codeword(const int b[8]) {
    printf("  ");
    for (int i = 1; i <= 7; i++) {
        if (is_parity_seat(i)) printf("%s%s%d%s", K(BLD), K(BCOL[buddy_of_seat(i)]), b[i], K(RST));
        else printf("%s%d%s", K(BLD), b[i], K(RST));
    }
    printf("   %s(colored digits = helper bulbs, white digits = your message)%s\n", K(DIM), K(RST));
}

/* ---------- features ---------- */

static void do_story(void) {
    heading("The story");
    printf("\n  Imagine you send 4 light bulbs to a friend through a long tunnel.\n"
           "  Each bulb is ON (1) or OFF (0). That is your secret message.\n\n"
           "  But a naughty gremlin lives in the tunnel. Sometimes he flips ONE bulb:\n"
           "  ON becomes OFF, or OFF becomes ON. Your friend cannot call you to ask\n"
           "  \"what did you mean?\".\n\n"
           "  The trick: you add 3 helper bulbs, one for each buddy:\n\n");
    for (int g = 0; g < 3; g++) {
        printf("    "); buddy_label(g);
        printf(" watches seats %d %d %d %d\n", GROUP[g][0], GROUP[g][1], GROUP[g][2], GROUP[g][3]);
    }
    printf("\n  Every buddy has ONE rule: \"the lit bulbs I watch must come in pairs\".\n"
           "  Pairs means an EVEN number of lit bulbs (0, 2 or 4).\n\n"
           "  If the gremlin flips a bulb, the buddies who watch it shout \"someone is\n"
           "  lonely!\". Each seat is watched by a different mix of buddies, so the\n"
           "  shouting pattern points at exactly the broken seat. Flip it back. Fixed!\n");
    pause_for_enter();
}

static int do_encode(const char *arg) {
    char buf[64];
    int d[4], b[8];
    if (arg) {
        if (!is_bits(arg, 4)) { printf("Error: provide exactly 4 binary bits (e.g. 1011)\n"); return 1; }
        snprintf(buf, sizeof buf, "%s", arg);
    } else {
        for (;;) {
            if (ask("\n  Type 4 bulbs, 1 = ON and 0 = OFF (example 1011): ", buf, sizeof buf)) return 1;
            if (is_bits(buf, 4)) break;
            printf("  %sOops! I need exactly 4 characters, each a 0 or a 1.%s\n", K(RED), K(RST));
        }
    }
    for (int k = 0; k < 4; k++) d[k] = buf[k] - '0';
    encode(d, b);

    heading("Step 1: your message");
    printf("\n  Your 4 bulbs: %s%s%s\n", K(BLD), buf, K(RST));
    printf("  They go in seats 3, 5, 6 and 7. Seats 1, 2 and 4 are left for the buddies.\n");

    heading("Step 2: the buddies count");
    printf("\n  Each buddy counts the lit bulbs of YOUR message that they watch.\n"
           "  If the count is odd, one bulb has no partner, so the buddy lights\n"
           "  their own bulb to make a pair.\n\n");
    for (int g = 0; g < 3; g++) {
        int cnt = 0, own = 1 << g;
        printf("  "); buddy_label(g); printf(" watches seats");
        for (int k = 0; k < 4; k++) {
            int s = GROUP[g][k];
            if (s == own) continue;
            printf(" %d", s);
            cnt += b[s];
        }
        printf(" -> lit: %d (%s) -> own bulb in seat %d is %s%s%s\n",
               cnt, cnt % 2 ? "odd" : "even", own,
               K(BLD), cnt % 2 ? "ON" : "OFF", K(RST));
    }

    heading("Step 3: ready to send");
    show_row(b, 0);
    printf("\n  Send this to your friend:\n");
    show_codeword(b);
    printf("\n  %sTry it: ./hamming correct %d%d%d%d%d%d%d%s\n", K(DIM), b[1], b[2], b[3], b[4], b[5], b[6], b[7], K(RST));
    pause_for_enter();
    return 0;
}

/* Detective work. orig = original 4 data bits (or NULL if unknown). */
static void run_correct(const int rx[8], const int *orig) {
    int b[8], p[3], syn;
    memcpy(b, rx, sizeof b);

    heading("What your friend received");
    show_row(b, 0);

    heading("The buddies check their groups");
    printf("\n  Each buddy counts ALL lit bulbs they watch, including their own.\n"
           "  Even = all paired up (happy). Odd = someone is lonely (problem!).\n\n");
    for (int g = 0; g < 3; g++) {
        int cnt = 0;
        printf("  "); buddy_label(g); printf(" watches seats");
        for (int k = 0; k < 4; k++) { printf(" %d", GROUP[g][k]); cnt += b[GROUP[g][k]]; }
        p[g] = cnt % 2;
        printf(" -> lit: %d (%s) %s%s%s\n", cnt, cnt % 2 ? "odd" : "even",
               K(p[g] ? RED : GRN), p[g] ? "PROBLEM" : "ok", K(RST));
    }

    syn = p[2] * 4 + p[1] * 2 + p[0];
    heading("Reading the answers as a number");
    printf("\n  Problem = 1, ok = 0. Read them in this order: Leafy, Bluey, Ruby.\n\n"
           "  Leafy=%d  Bluey=%d  Ruby=%d   ->   binary %d%d%d   ->   %sseat %d%s\n",
           p[2], p[1], p[0], p[2], p[1], p[0], K(BLD), syn, K(RST));

    if (syn == 0) {
        printf("\n  %sNobody complained. No gremlin was here!%s\n", K(GRN), K(RST));
    } else {
        printf("\n  %sThe gremlin flipped seat %d!%s Showing where:\n", K(RED), syn, K(RST));
        show_row(b, syn);
        b[syn] ^= 1;
        heading("Flip it back");
        show_row(b, 0);
    }

    printf("\n  %sYour message is: %d%d%d%d%s", K(BLD), b[3], b[5], b[6], b[7], K(RST));
    if (orig) {
        int same = b[3] == orig[0] && b[5] == orig[1] && b[6] == orig[2] && b[7] == orig[3];
        printf("  %s%s%s", K(same ? GRN : RED), same ? "(same as what was sent!)" : "(different, the tunnel did too much damage)", K(RST));
    }
    printf("\n  %sRemember: this fixes ONE flipped bulb. Two flips can fool the buddies.%s\n", K(DIM), K(RST));
    pause_for_enter();
}

static int do_correct(const char *arg) {
    char buf[64];
    int b[8];
    if (arg) {
        if (!is_bits(arg, 7)) { printf("Error: provide exactly 7 binary bits (e.g. 0110011)\n"); return 1; }
        snprintf(buf, sizeof buf, "%s", arg);
    } else {
        for (;;) {
            if (ask("\n  Type the 7 bulbs you received (example 0110001): ", buf, sizeof buf)) return 1;
            if (is_bits(buf, 7)) break;
            printf("  %sOops! I need exactly 7 characters, each a 0 or a 1.%s\n", K(RED), K(RST));
        }
    }
    b[0] = 0;
    for (int i = 1; i <= 7; i++) b[i] = buf[i - 1] - '0';
    run_correct(b, NULL);
    return 0;
}

static void do_demo(void) {
    char buf[64];
    int d[4], b[8], rx[8], seat;

    heading("Play: send a message past the gremlin");
    for (;;) {
        if (ask("\n  Type 4 bulbs (or press Enter for 1011): ", buf, sizeof buf)) return;
        if (buf[0] == '\0') strcpy(buf, "1011");
        if (is_bits(buf, 4)) break;
        printf("  %sOops! I need exactly 4 characters, each a 0 or a 1.%s\n", K(RED), K(RST));
    }
    for (int k = 0; k < 4; k++) d[k] = buf[k] - '0';
    encode(d, b);
    printf("\n  The buddies add their helper bulbs. Ready to send:\n");
    show_row(b, 0);
    pause_for_enter();

    for (;;) {
        char *end;
        if (ask("\n  Which seat should the gremlin flip? (1-7, 0 = none, Enter = surprise me): ", buf, sizeof buf)) return;
        if (buf[0] == '\0') { seat = 1 + rand() % 7; break; }
        seat = (int)strtol(buf, &end, 10);
        if (*end == '\0' && seat >= 0 && seat <= 7) break;
        printf("  %sPlease type a number from 0 to 7.%s\n", K(RED), K(RST));
    }
    memcpy(rx, b, sizeof rx);
    if (seat) {
        rx[seat] ^= 1;
        printf("\n  %sThe gremlin sneaks in and flips seat %d... your friend does not know which!%s\n", K(RED), seat, K(RST));
    } else {
        printf("\n  The gremlin stayed home.\n");
    }
    run_correct(rx, d);
}

static void banner(void) {
    printf("\n  %s%s╭──────────────────────────────────────────────╮%s\n", K(BLD), K(YEL), K(RST));
    printf("  %s%s│            HAMMING  HELPERS                  │%s\n", K(BLD), K(YEL), K(RST));
    printf("  %s%s│   How computers fix their own mistakes       │%s\n", K(BLD), K(YEL), K(RST));
    printf("  %s%s╰──────────────────────────────────────────────╯%s\n", K(BLD), K(YEL), K(RST));
}

static void usage(const char *prog) {
    banner();
    printf("\n  Just run  %s%s%s  for the friendly menu. Shortcuts:\n\n", K(BLD), prog, K(RST));
    printf("    %s encode  <4 bits>   e.g. 1011\n", prog);
    printf("    %s correct <7 bits>   e.g. 0110001\n", prog);
    printf("    %s demo               play with the gremlin\n", prog);
    printf("    %s story              the story in plain words\n\n", prog);
}

int main(int argc, char *argv[]) {
    char buf[32];
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    SetConsoleOutputCP(65001);
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004);
#endif
    srand((unsigned)time(NULL));
    if (getenv("NO_COLOR") || !ISATTY(FILENO(stdout))) color_on = 0;

    if (argc == 3 && strcmp(argv[1], "encode") == 0) return do_encode(argv[2]);
    if (argc == 3 && strcmp(argv[1], "correct") == 0) return do_correct(argv[2]);
    if (argc == 2 && strcmp(argv[1], "story") == 0) { do_story(); return 0; }
    if (argc == 2 && strcmp(argv[1], "demo") == 0) { interactive = 1; do_demo(); return 0; }
    if (argc != 1) { usage(argv[0]); return 1; }

    interactive = 1;
    banner();
    for (;;) {
        printf("\n  What would you like to do?\n\n"
               "    %s1%s  Learn the story (1 minute)\n"
               "    %s2%s  Encode: turn 4 bulbs into 7 (add the helpers)\n"
               "    %s3%s  Fix: find and repair a broken bulb\n"
               "    %s4%s  Play: send a message past the gremlin\n"
               "    %s0%s  Quit\n",
               K(BLD), K(RST), K(BLD), K(RST), K(BLD), K(RST), K(BLD), K(RST), K(BLD), K(RST));
        if (ask("\n  Your choice: ", buf, sizeof buf)) break;
        switch (buf[0]) {
        case '1': do_story(); break;
        case '2': do_encode(NULL); break;
        case '3': do_correct(NULL); break;
        case '4': do_demo(); break;
        case '0': case 'q': case 'Q': printf("\n  Bye! Keep those bulbs shining.\n\n"); return 0;
        default: printf("  %sPlease type 1, 2, 3, 4 or 0.%s\n", K(RED), K(RST));
        }
    }
    printf("\n");
    return 0;
}
