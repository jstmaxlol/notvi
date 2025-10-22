#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * NotVi, either a Vi "clone" or me trying ncurses out, you choose.
 * jstmax! / ceez2exzt / maxwasmailed@proton.me
*/

// Functions prototypes aka forward declarations
void StatusBar(const char *mode);
void Lines();
void PrintStatBarInfo(int line, int col, const char *file);
void StatusMsg(const char *msg);

// Global macros
#define MAX_LINES 1337
#define MAX_COLS 420

// Global variables
bool cmdmode = true;
char txtBuf[MAX_LINES][MAX_COLS];
int tabstop = 4;
bool number = false;
char filename[256]; // TODO: increase buf size?
bool first_insert = true;

int main(int argc, char **argv) {
    // Ncurses sets
	initscr();
    cbreak();
    noecho();
	keypad(stdscr, TRUE);
	// COLOR PAIRS
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	
	// Set background color
	bkgd(COLOR_PAIR(1));

	// PRINT SEQUENCE
	
	// Print title
	char titleMessage[] = "NotVi";
	move((LINES/2), (COLS/2) - (strlen(titleMessage)/2));
	printw("%s", titleMessage);

	refresh();

// COMMAND MODE LOOP
// TODO: something i would like to work on
// is the fact that right now there isn't
// a true Vi-like "normal mode" as per this
// implementation you have a state that is
// either COMMAND MODE or INSERT MODE
	while (true) {
		curs_set(1);
		noecho();
	
		// Print layout
		Lines();
	    StatusBar("0");

		refresh();

    	char buffno[64];
    	getstr(buffno);

		if (strcmp(buffno, "i") == 0 || strcmp(buffno, "insert") == 0) {
		// INSERT MODE
			if (first_insert == true) clear();
			first_insert = false;

			//
			curs_set(1);
			noecho();

			// Print mode layout
			Lines();
			StatusBar("INSERT");

			move(1, 4);
			refresh();
		
			int line = 1, col = 3;

			while (true) {
    			int typed_char = getch();

				// CHECK KEYS
    			if (typed_char == KEY_LEFT) --col;
				else if (typed_char == KEY_RIGHT) ++col;
				else if (typed_char == KEY_UP) --line;
				else if (typed_char == KEY_DOWN) ++line;
				else if (typed_char == KEY_BACKSPACE || typed_char == 127) {
					if (col > 3) --col;
					mvdelch(line, col);
					txtBuf[line][col] = '\0';
    			} else if (typed_char == KEY_CANCEL || typed_char == 30) mvdelch(line, col+1);
				else if (typed_char == '\n' || typed_char == '\r') {
        			txtBuf[line][col] = '\0';
					++line;
					col = 3;
    			} else if (typed_char == KEY_BTAB || typed_char == '\t') {
					for (int t = 0; t < tabstop; ++t) mvaddch(line, col++, ' ');
				} else if (typed_char == 27) {
        			curs_set(0);
					break;
    			} else if (typed_char >= 32 && typed_char <= 126) {
					mvaddch(line, col, typed_char);
					txtBuf[line][col] = typed_char;
					++col;
				}
				
				if (line < 1) line = 1;
				if (line >= LINES-2) line = LINES-3;
				if (col < 3) col = 3;
				if (col >= COLS) col = COLS-1;

				// Print lines and coloumns
				PrintStatBarInfo(line, col, filename);

    			move(line, col);
				txtBuf[line][col] = '\0';
    			refresh();
			}
		} else if (strcmp(buffno, "w") == 0 || strcmp(buffno, "write") == 0) {
			StatusMsg("Filename? ");
			getstr(filename);

			if (strlen(filename) > 0 && filename[0] != '/' && !strchr(filename, ' ')) {
				FILE *f = fopen(filename, "w");
				if (!f) {
					StatusMsg("ERROR: Unable to open file");
					continue;
				}

				for (int i = 1; i < MAX_LINES; ++i) {
					if (txtBuf[i][0] == '\0') {
						fputc('\n', f);
						continue;
					}
					fputs(txtBuf[i], f);
					fputc('\n', f);
				}

				fclose(f);
				StatusMsg("File written.");
			} else {
				StatusMsg("ERROR: Invalid filename");
			}
		} else if (strcmp(buffno, "q") == 0 || strcmp(buffno, "quit") == 0) {
			refresh();
			noecho();
			endwin();
			return 0;
		} else if (strcmp(buffno, "set") == 0) {
			char buffno_2[64];
			getstr(buffno_2);
			refresh();
			if (strcmp(buffno_2, "number") == 0 || strcmp(buffno_2, "nu") == 0) {
				number = !number;
				refresh();
			} else if (strcmp(buffno_2, "tabstop") == 0 || strcmp(buffno_2, "tab") == 0) {
				char buffno_3[16];
				getstr(buffno_3);
				tabstop = atoi(buffno_3);
				refresh();
			} else {
				StatusMsg("Invalid value entered.");
			}
		} else {
			StatusMsg("Invalid option entered.");
		}
	}

	refresh();

	return 1;
}

// To enter command mode, change global variable state and
// call StatusBar("0"), then getstr() to a buff, or wgetnstr()
// because buffer overflows are not that cool to be honest.
void StatusBar(const char *mode) {
	// Status bar
	echo();
	mvhline(LINES - 2, 0, ACS_HLINE, COLS);

	// Mode
	for (int i = 0; i < COLS-1; ++i) {
		mvdelch(LINES-1, (COLS/2)-i);
	}
	
	if (strcmp(mode, "0") == 0) {
		move(LINES-1, 0);
		printw(":");
	} else {
		move(LINES-1, 0);
		printw("== %s ==", mode);
	}
}

void Lines() {
	mvprintw(0, 0, "~");
	for (int i = 1; i < LINES-2; ++i) {
		move(i, 1);
		if (number == true) {
			printw("%d", i);
		} else if (number == false) {
			printw("~");
		}
	}
}

void PrintStatBarInfo(int line, int col, const char *file) {
	// Filename (if there is one)
	if (strlen(file) >= 3) {
		mvprintw(LINES-2, 0, "%s", file);
	}
	
	// LINES:COLS size
	mvprintw(LINES-2, (COLS/5), "%d:%d", line, col-3);
}

void StatusMsg(const char *msg) {
    move(LINES-2, ((COLS/5)+(COLS/2))-(strlen(msg)/3));
    //clrtoeol();
    printw("%s", msg);
}

