#define _CRT_SECURE_NO_WARNINGS /* for vs */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <locale.h>
#include "yixin.h"

#define MAX_SIZE 22
#define CAUTION_NUM 5 // 0..CAUTION_NUM
#define MIN_SPLIT_DEPTH 5
#define MAX_SPLIT_DEPTH 20
#define MAX_TOOLBAR_ITEM 64
#define MAX_TOOLBAR_COMMAND_LEN 2048
#define MAX_HOTKEY_ITEM 64
#define MAX_HOTKEY_COMMAND_LEN 2048

int yixin_strnicmp(const char *string1, const char *string2, size_t count)
{
#ifdef _WIN32
	return _strnicmp(string1, string2, count);
#else
	return strncasecmp(string1, string2, count);
#endif
}

int respawn = 0;
int showdatabase = 1;
int usedatabase = 1;
int databasereadonly = 0;
int boardsizeh = 15, boardsizew = 15;
int rboardsizeh = 15, rboardsizew = 15;
int inforule = 0;
int specialrule = 0;
int infopondering = 0;
int infovcthread = 0;
int64_t timeoutturn = 10000;
int64_t timeoutmatch = 2000000;
int64_t maxnode = 1000000000;
int maxdepth = 100;
int maxthreadnum = 1; // 1..maxthreadnum
int maxhashsizemb = 65536;
int increment = 0;
int computerside = 0; /* 0 none 1 black 2 while 3 black&white */
int cautionfactor = 1;
int threadnum = 1;
int hashsizemb = 256;
int nbestsym = 0;
int board[MAX_SIZE][MAX_SIZE];
int boardnumber[MAX_SIZE][MAX_SIZE];
int movepath[MAX_SIZE * MAX_SIZE];
int forbid[MAX_SIZE][MAX_SIZE];
int boardblock[MAX_SIZE][MAX_SIZE];
int boardbestX = -1, boardbestY = -1;
int boardlose[MAX_SIZE][MAX_SIZE];
int boardpos[MAX_SIZE][MAX_SIZE];
int boardtag[MAX_SIZE][MAX_SIZE];
char boardtext[MAX_SIZE][MAX_SIZE][8];
int blockautoreset = 0;
int blockpathautoreset = 0;
int hashautoclear = 0;
int piecenum = 0;
char isthinking = 0, isgameover = 0, isneedrestart = 0, isneedomit = 0;
char isctrlpressed = 0;
char drawpieceonly = 0;
char bestline[MAX_SIZE * MAX_SIZE * 5 + 1] = "";
int bestval;
int move5N;
int levelchoice = 0;
int commandmodel = 0;
int shownumber = 1;
int showlog = 1;
int showanalysis = 1;
int showclock = 0;
int showforbidden = 1;
int showboardtext = 1;
int showtoolbarboth = 1;
int showwarning = 1;
int showdbdelconfirm = 1;
int checktimeout = 1;
int toolbarpos = 0;
int language = 0; /* 0: English 1: Other languages */
int rlanguage = 0;
char **clanguage = NULL;				   /* Custom language */
int movx[8] = {0, 0, 1, -1, 1, 1, -1, -1}; /* note that the order is related to winning checking function(s)*/
int movy[8] = {1, -1, 0, 0, 1, -1, 1, -1};
/* engine */
GIOChannel *iochannelin, *iochannelout, *iochannelerr;
/* windowmain */
GtkWidget *windowmain;
GtkWidget *tableboard;
GtkWidget *imageboard[MAX_SIZE][MAX_SIZE];
GtkWidget *labelboard[2][MAX_SIZE];
GtkWidget *vboxwindowmain;
GdkPixbuf *pixbufboard[9][14];
GdkPixbuf *pixbufboardnumber[9][14][MAX_SIZE * MAX_SIZE + 1][2];
GdkPixbuf *pixbufboardchar[9][14][128 + 100 + 100 + 100][2];

int imgtypeboard[MAX_SIZE][MAX_SIZE];
char piecepicname[80] = "piece.bmp";
char fontname[4][120];
const char *font_familyboardtext;
int font_sizeboardtext;
/* log */
GtkWidget *textlog, *textpos, *textdbcomment;
GtkTextBuffer *buffertextlog, *buffertextcommand, *buffertextdbcomment, *buffertextpos;
GtkWidget *scrolledtextlog, *scrolledtextcommand, *scrolledtextdbcomment, *scrolledtextpos;
GtkWidget *toolbar, *posbar;

double hdpiscale = 1.0;

int toolbarnum = 6;

GtkWidget *savingdialog, *loadingdialog;
GtkWidget *windowclock;
GtkWidget *clocklabel[4];
GtkWidget *playerlabel[2];
int64_t timercomputerturn;
int64_t timercomputermatch;
int64_t timerhumanturn;
int64_t timerhumanmatch;
int timercomputerincrement;
int timerhumanincrement;
int timerstart;
int timerstatus = 0;
int timeoutflag = 0;

int recorddebuglog = 0;
FILE *debuglog;

int toolbarlng[MAX_TOOLBAR_ITEM] =
	{
		48,
		46,
		47,
		49,
		45,
		44};

char *toolbaricon[MAX_TOOLBAR_ITEM] =
	{
		"go-first",
		"go-previous",
		"go-next",
		"go-last",
		"media-playback-stop",
		"media-playback-start"};

char toolbarcommand[MAX_TOOLBAR_ITEM][MAX_TOOLBAR_COMMAND_LEN] =
	{
		"undo all\n",
		"undo one\n",
		"redo one\n",
		"redo all\n",
		"thinking stop\n",
		"thinking start\n"};

int hotkeynum = 6;

int hotkeykey[MAX_HOTKEY_ITEM] =
	{
		13,
		14,
		15,
		16,
		53,
		1};

char hotkeycommand[MAX_HOTKEY_ITEM][MAX_HOTKEY_COMMAND_LEN] =
	{
		"undo all\n",
		"redo all\n",
		"undo one\n",
		"redo one\n",
		"thinking stop\n",
		"thinking toggle\n"};

char *hotkeynamelist[] = {
	"",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Ctrl + Up", "Ctrl + Down", "Ctrl + Left", "Ctrl + Right",
	"Ctrl + 1", "Ctrl + 2", "Ctrl + 3", "Ctrl + 4", "Ctrl + 5", "Ctrl + 6", "Ctrl + 7", "Ctrl + 8", "Ctrl + 9", "Ctrl + 0",
	"Ctrl + A", "Ctrl + B", "Ctrl + C", "Ctrl + D", "Ctrl + E", "Ctrl + F", "Ctrl + G", "Ctrl + H", "Ctrl + I", "Ctrl + J",
	"Ctrl + K", "Ctrl + L", "Ctrl + M", "Ctrl + N", "Ctrl + O", "Ctrl + P", "Ctrl + Q", "Ctrl + R", "Ctrl + S", "Ctrl + T",
	"Ctrl + U", "Ctrl + V", "Ctrl + W", "Ctrl + X", "Ctrl + Y", "Ctrl + Z",
	"Escape", NULL};

int hotkeykeylist[][2] =
	{
		{0, 0},			 // 0
		{0, GDK_KEY_F1}, // 1
		{0, GDK_KEY_F1}, // 2
		{0, GDK_KEY_F1}, // 3
		{0, GDK_KEY_F1}, // 4
		{0, GDK_KEY_F1}, // 5
		{0, GDK_KEY_F1}, // 6
		{0, GDK_KEY_F1}, // 7
		{0, GDK_KEY_F1}, // 8
		{0, GDK_KEY_F1}, // 9
		{0, GDK_KEY_F1}, // 10
		{0, GDK_KEY_F1}, // 11
		{0, GDK_KEY_F1}, // 12

		{GDK_CONTROL_MASK, GDK_KEY_Up},	   // 13
		{GDK_CONTROL_MASK, GDK_KEY_Down},  // 14
		{GDK_CONTROL_MASK, GDK_KEY_Left},  // 15
		{GDK_CONTROL_MASK, GDK_KEY_Right}, // 16

		{GDK_CONTROL_MASK, GDK_KEY_1}, // 17
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 18
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 19
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 20
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 21
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 22
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 23
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 24
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 25
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 26

		{GDK_CONTROL_MASK, GDK_KEY_1}, // 27
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 28
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 29
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 30
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 31
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 32
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 33
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 34
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 35
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 36
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 37
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 38
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 39
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 40
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 41
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 42
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 43
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 44
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 45
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 46
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 47
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 48
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 49
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 50
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 51
		{GDK_CONTROL_MASK, GDK_KEY_1}, // 52

		{0, GDK_KEY_Escape} // 53
};

char *_T(char *s)
{
	if (!s)
		return NULL;

	char *result = g_locale_to_utf8(s, -1, 0, 0, 0);
	return result ? result : "??";
}

char *__invT(char *s)
{
	if (!s)
		return NULL;

	char *result = g_locale_from_utf8(s, -1, 0, 0, 0);
	return result ? result : "??";
}

static char *pending_text_buffer = NULL;
static int pending_size_buffer = 0;
static int pending_capacity_buffer = 0;
static guint batch_timeout_id_global = 0;
static guint scroll_timeout_id_global = 0;

static gboolean delayed_scroll_handler(gpointer data)
{
	GtkTextIter end;
	GtkTextMark *endmark;

	if (buffertextlog == NULL)
	{
		scroll_timeout_id_global = 0;
		return FALSE;
	}

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffertextlog), &end);
	gtk_text_iter_set_line_offset(&end, 0);
	endmark = gtk_text_buffer_get_mark(GTK_TEXT_BUFFER(buffertextlog), "scroll");
	if (endmark)
	{
		gtk_text_buffer_move_mark(GTK_TEXT_BUFFER(buffertextlog), endmark, &end);
		gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textlog), endmark);
	}
	scroll_timeout_id_global = 0;
	return FALSE;
}

static gboolean flush_pending_text_handler(gpointer data)
{
	if (pending_text_buffer && pending_size_buffer > 0)
	{
		GtkTextIter start, end;
		int len;

		if (buffertextlog == NULL)
		{
			pending_size_buffer = 0;
			batch_timeout_id_global = 0;
			return FALSE;
		}

		g_signal_handlers_block_by_func(buffertextlog, NULL, NULL);

		len = gtk_text_buffer_get_line_count(GTK_TEXT_BUFFER(buffertextlog));
		if (len > 800)
		{
			gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(buffertextlog), &start, 0);
			gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(buffertextlog), &end, len - 800);
			gtk_text_buffer_delete(GTK_TEXT_BUFFER(buffertextlog), &start, &end);
		}

		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextlog), &start, &end);
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextlog), &end, pending_text_buffer, pending_size_buffer);

		g_signal_handlers_unblock_by_func(buffertextlog, NULL, NULL);

		pending_size_buffer = 0;

		if (scroll_timeout_id_global == 0)
		{
			scroll_timeout_id_global = g_timeout_add(50, delayed_scroll_handler, NULL);
		}
	}
	batch_timeout_id_global = 0;
	return FALSE;
}

void print_log(char *text)
{
	GtkTextIter start, end;
	GtkTextMark *endmark;
	static int init = 0;
	static int flag = 0, fspace = 0;
	int len;
	int i;

	if (buffertextlog == NULL)
	{
		printf("%s", text);
		return;
	}

	if (commandmodel == 0)
	{
		if (strncmp(text, "OK", 2) == 0)
			text += 2;
		if (strncmp(text, "MESSAGE", 7) == 0)
			text += 7 + 1;
		if (strncmp(text, "DETAIL", 6) == 0)
			text += 6 + 1;
		if (strncmp(text, "DEBUG", 5) == 0)
			text += 5 + 1;
		if (strncmp(text, "ERROR", 5) == 0)
			text += 5;
		if (strncmp(text, "UNKNOWN", 7) == 0)
			text += 7;
		// if(strncmp(text, "FORBID", 6) == 0) text += 6;
		if (strncmp(text, "YIXIN", 5) == 0 || strncmp(text, "DEEP YIXIN", 10) == 0)
		{
			if (flag == 0)
				flag = 1;
			else
				return;
		}

		for (i = 0; text[i]; i++)
		{
			if (!isspace(text[i]))
				break;
		}
		if (text[i] == 0)
		{
			fspace++;
		}
		else
		{
			fspace = 0;
		}
		if (fspace >= 2)
			return;
	}

	if (init == 0)
	{
		GtkTextIter end;
		gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffertextlog), &end);
		gtk_text_iter_set_line_offset(&end, 0);
		init = 1;
		gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(buffertextlog), "scroll", &end, TRUE);
	}

	int text_len = strlen(text);
	if (pending_size_buffer + text_len + 1 > pending_capacity_buffer)
	{
		pending_capacity_buffer = (pending_size_buffer + text_len + 1) * 2;
		pending_text_buffer = realloc(pending_text_buffer, pending_capacity_buffer);
	}

	if (pending_text_buffer)
	{
		memcpy(pending_text_buffer + pending_size_buffer, text, text_len);
		pending_size_buffer += text_len;
		pending_text_buffer[pending_size_buffer] = '\0';
	}

	if (batch_timeout_id_global == 0)
	{
		batch_timeout_id_global = g_timeout_add(10, flush_pending_text_handler, NULL);
	}
}

void cleanup_log_pending_text()
{
	static int cleanup_called = 0;
	if (!cleanup_called)
	{
		cleanup_called = 1;
	}
}

int printf_log(char *fmt, ...)
{
	int cnt;
	char buffer[8192];
	char text[8192], *p = text;
	int i;
	va_list va;
	va_start(va, fmt);
	cnt = vsprintf(buffer, fmt, va);

#define REPLACE_STRING(str, len, index)                        \
	if (strncmp(buffer + i, str, len) == 0 &&                  \
		(buffer[i + len] == '\0' || isspace(buffer[i + len]))) \
	{                                                          \
		strcpy(p, clanguage[index]);                           \
		p += strlen(clanguage[index]);                         \
		i += len;                                              \
		continue;                                              \
	}

	if (language)
	{
		for (i = 0; buffer[i];)
		{
			REPLACE_STRING("BESTLINE", 8, 0)
			REPLACE_STRING("EVALUATION", 10, 1)
			REPLACE_STRING("SPEED", 5, 2)
			REPLACE_STRING("TIME", 4, 3)
			REPLACE_STRING("DEPTH", 5, 4)
			REPLACE_STRING("BLOCK", 5, 5)
			REPLACE_STRING("EVAL", 4, 7)
			REPLACE_STRING("VAL", 3, 7)
			REPLACE_STRING("NODE", 4, 6)
			REPLACE_STRING("NODES", 5, 6)
			REPLACE_STRING("MS", 2, 8)
			REPLACE_STRING("RULE", 4, 9)

			*p = buffer[i];
			p++;
			i++;
		}
		*p = '\0';
	}

#undef REPLACE_STRING
	else
	{
		strcpy(text, buffer);
	}

	print_log(_T(text));
	va_end(va);
	return cnt;
}

void print_command(char *text)
{
	GtkTextIter start, end;

	if (buffertextcommand == NULL)
	{
		printf("%s", text);
		return;
	}

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextcommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextcommand), &end, text, strlen(text));
}

int printf_command(char *fmt, ...)
{
	int cnt;
	char buffer[1024];
	va_list va;
	va_start(va, fmt);
	cnt = vsprintf(buffer, fmt, va);
	print_command(_T(buffer));
	va_end(va);
	return cnt;
}

void show_welcome()
{
	printf_log("Yixin Board " VERSION "\n");
	if (language)
	{
		printf_command(clanguage[10]);
	}
	else
	{
		printf_command("To get help, type help and press Enter here");
	}
}

void show_thanklist()
{
	printf_log(language == 0 ? "Thanks to all who helped development of Yixin:" : clanguage[11]);
	printf_log("\n");
	printf_log("  彼方\n");
	printf_log("  XR\n");
	printf_log("  舒自均\n");
	printf_log("  Tianyi Hao\n");
	printf_log("  Hao Wu\n");
	printf_log("  雨中飞燕\n");
	printf_log("  Tuyen Do\n");
	printf_log("  肥国乃乃\n");
	printf_log("  Saturn|Titan\n");
	printf_log("  元\n");
	printf_log("  Alexander Bogatirev\n");
	printf_log("  Epifanov Dmitry\n");
	printf_log("  TZ\n");
	printf_log("  濤声依旧\n");
	printf_log("  嘿嘿\n");
	printf_log("  张锡森\n");
	printf_log("  ax_pokl\n");
	printf_log("  Ola Strom");
	printf_log("\n");
}

GdkPixbuf *draw_overlay_scaled(GdkPixbuf *pb, int w, int h, gchar *text, const char *color, const char *weight, float scale)
{
	cairo_surface_t *surface;
	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *font_desc;
	gchar *markup;
	GdkPixbuf *ret;
	gchar format[100];
	PangoRectangle ink_rect, logical_rect;

	// Create Cairo surface and context
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cr = cairo_create(surface);

	// Draw the base pixbuf onto the surface
	gdk_cairo_set_source_pixbuf(cr, pb, 0, 0);
	cairo_paint(cr);

	// Create Pango layout
	layout = pango_cairo_create_layout(cr);

	// Set up font and markup
	sprintf(format, "<span foreground='%s' weight='%s' size='%d' font_family='%s'>%%s</span>",
			color, weight, (int)(font_sizeboardtext * 1000 * scale), font_familyboardtext);
	gchar *escaped_text = g_markup_escape_text(text, -1);
	markup = g_strdup_printf(format, escaped_text);
	pango_layout_set_markup(layout, markup, -1);
	g_free(escaped_text);
	g_free(markup);

	// Get text extents and center the text
	pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);
	cairo_move_to(cr, w / 2 - logical_rect.width / 2, h / 2 - logical_rect.height / 2);

	// Draw the text
	pango_cairo_show_layout(cr, layout);

	// Convert surface back to pixbuf
	ret = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);

	// Cleanup
	g_object_unref(layout);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	return ret;
}

GdkPixbuf *draw_overlay(GdkPixbuf *pb, int w, int h, gchar *text, const char *color)
{
	return draw_overlay_scaled(pb, w, h, text, color, "bold", 1.0f);
}

void send_command(const char *command)
{
	gsize size;
	g_io_channel_write_chars(iochannelin, command, -1, &size, NULL);
	g_io_channel_flush(iochannelin, NULL);

	if (debuglog != NULL)
	{
		fprintf(debuglog, "SEND_COMMAND [%s,%s,%s,%s]: %s\n",
				gtk_label_get_text(clocklabel[0]),
				gtk_label_get_text(clocklabel[1]),
				gtk_label_get_text(clocklabel[2]),
				gtk_label_get_text(clocklabel[3]),
				command);
		fflush(debuglog);
	}
}

int swap2done = 0;

int boardtagclear = 0;
void clear_board_tag()
{
	if (boardtagclear == 0)
	{
		boardtagclear = 1;
		memset(boardtag, 0, sizeof(boardtag));
		memset(boardtext, 0, sizeof(boardtext));
	}
}

typedef struct
{
	double r; // a fraction between 0 and 1
	double g; // a fraction between 0 and 1
	double b; // a fraction between 0 and 1
} rgb;

typedef struct
{
	double h; // angle in degrees
	double s; // a fraction between 0 and 1
	double v; // a fraction between 0 and 1
} hsv;

rgb hsv2rgb(hsv in)
{
	double hh, p, q, t, ff;
	long i;
	rgb out;

	if (in.s <= 0.0)
	{ // < is bogus, just shuts up warnings
		out.r = in.v;
		out.g = in.v;
		out.b = in.v;
		return out;
	}
	hh = in.h;
	if (hh >= 360.0)
		hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = in.v * (1.0 - in.s);
	q = in.v * (1.0 - (in.s * ff));
	t = in.v * (1.0 - (in.s * (1.0 - ff)));

	switch (i)
	{
	case 0:
		out.r = in.v;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = in.v;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = in.v;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = in.v;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = in.v;
		break;
	case 5:
	default:
		out.r = in.v;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

int losssaturation = 0;
int winsaturation = 0;
int minsaturation = 0;
int maxsaturation = 0;
int colorvalue = 100;
const char *winrate2colorstr(int winrate)
{
	static char color_buf[10] = "#";
	hsv hsv_color = {
		(100 - winrate) * (180 / 100.0),
		(winrate <= 0 ? losssaturation : winrate >= 100 ? winsaturation
														: minsaturation + (maxsaturation - minsaturation) * (winrate / 100.0)) *
			0.01,
		colorvalue * 0.01};
	rgb rgb_color = hsv2rgb(hsv_color);
	sprintf(color_buf + 1, "%02X", (int)(rgb_color.r * 255));
	sprintf(color_buf + 3, "%02X", (int)(rgb_color.g * 255));
	sprintf(color_buf + 5, "%02X", (int)(rgb_color.b * 255));
	return color_buf;
}

int refreshboardflag = 0;  // it is set to 1 when making the 5th move under soosorv rule, otherwise, 0
int refreshboardflag2 = 0; // it is set to 1 when refreshboardflag has been set to 1
void refresh_board_area(int x0, int y0, int x1, int y1)
{
	for (int i = y0; i < y1; i++)
	{
		for (int j = x0; j < x1; j++)
		{
			if (board[i][j] == 0)
			{
				if (drawpieceonly)
					continue;

				int f = 0;
				if (inforule == 2 && (computerside & 1) == 0 && piecenum % 2 == 0 && forbid[i][j] && isgameover == 0 && isthinking == 0 && showforbidden)
					f = 2;
				if (f == 0)
				{
					if (boardblock[i][j])
						f = 10;
					else if (showanalysis)
					{
						if (boardlose[i][j])
							f = 7;
						else if (i == boardbestY && j == boardbestX)
							f = 8;
						else if (boardpos[i][j] == 1)
							f = 9;
						else if (boardpos[i][j] == 2)
							f = 11;
					}
					if (f == 0 && usedatabase && showdatabase)
					{
						if (boardtag[i][j])
							f = 12 + piecenum % 2;
					}
				}
				if (f <= 11 || boardtag[i][j] <= 0 && (!showboardtext || boardtext[i][j][0] == 0))
				{
					if (imgtypeboard[i][j] <= 8)
						gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[imgtypeboard[i][j]][max(0, f)]);
					else
						gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[0][max(1, f)]);
				}
				else
				{
					int x, y;
					GdkPixbuf *p = NULL;
					char n[10];

					if (imgtypeboard[i][j] <= 8)
					{
						y = imgtypeboard[i][j];
						x = 0;
					}
					else
					{
						y = 0;
						x = 1;
					}

					int W = gdk_pixbuf_get_width(pixbufboard[y][x]);
					int H = gdk_pixbuf_get_height(pixbufboard[y][x]);
					const char *color = piecenum % 2 ? "#FFFFFF" : "#000000";
					const char *weight = "normal";

					if (showboardtext && boardtext[i][j][0])
					{
						int len = strlen(boardtext[i][j]);
						char *text = _T(boardtext[i][j]);
						float scale = 1.0f - 0.1f * max(len - 4, 0);
						p = draw_overlay_scaled(pixbufboard[y][x], W, H, text, color, "bold", scale);
						gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), p);
						g_object_unref(G_OBJECT(p));
						g_free(text);
						continue;
					}

#define UPPERCASE_TAG(tag) ((tag) == 'w' ? 'W' : ((tag) == 'l' ? 'L' : ((tag) == 'd' ? 'D' : (tag))))
					int tag = boardtag[i][j];
					if (tag < 128)
					{
						tag = UPPERCASE_TAG(tag);
						if (pixbufboardchar[y][x][tag][piecenum % 2] == NULL)
						{
							if (tag == 'W')
								color = winrate2colorstr(100), weight = "bold";
							else if (tag == 'L')
								color = winrate2colorstr(0);

							sprintf(n, "%c", tag);
							pixbufboardchar[y][x][tag][piecenum % 2] = draw_overlay_scaled(pixbufboard[y][x], W, H, n, color, weight, 1.0f);
						}
						gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboardchar[y][x][tag][piecenum % 2]);
					}
					else
					{
						int first = 0; // first character of the tag
						float scale;
						if (tag >> 16 == 0)
							sprintf(n, "%c%c", first = UPPERCASE_TAG(tag / 256), tag % 256), scale = 0.97f;
						else if (tag >> 24 == 0)
							sprintf(n, "%c%c%c", first = UPPERCASE_TAG(tag / 65536), tag / 256 % 256, tag % 256), scale = 0.95f;
						else
							sprintf(n, "%c%c%c%c", first = UPPERCASE_TAG(tag / 16777216), tag / 65536 % 256, tag / 256 % 256, tag % 256), scale = 0.9f;

						if (first == 'W' || first == 'L')
						{
							if (first == 'W')
								color = winrate2colorstr(100), weight = "bold", scale *= 0.98f;
							else
								color = winrate2colorstr(0), scale *= 0.95f;

							int step = n[1] == '*' && n[2] == 0 ? 0 : atoi(n + 1);
							if (step == 0)
								n[1] = '*', n[2] = 0;
							if (step >= 0 && step < 100)
							{
								// use cached Win/Lose pixbuf
								int idx = step + 100 * (first == 'L') + 128;
								if (pixbufboardchar[y][x][idx][piecenum % 2] == NULL)
								{
									pixbufboardchar[y][x][idx][piecenum % 2] = draw_overlay_scaled(pixbufboard[y][x], W, H, n, color, weight, scale);
								}
								gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboardchar[y][x][idx][piecenum % 2]);
								continue;
							}
						}
						else if (tag % 256 == '%')
						{
							weight = "bold", scale *= 0.98f;
							int winrate = atoi(n);
							if (winrate >= 0 && winrate < 100)
							{
								// use cached Winrate pixbuf
								int idx = winrate + 100 * 2 + 128;
								if (pixbufboardchar[y][x][idx][piecenum % 2] == NULL)
								{
									color = winrate2colorstr(max(winrate, 1)); // use winrate=1 at least
									pixbufboardchar[y][x][idx][piecenum % 2] = draw_overlay_scaled(pixbufboard[y][x], W, H, n, color, weight, scale);
								}
								gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboardchar[y][x][idx][piecenum % 2]);
								continue;
							}
						}

						p = draw_overlay_scaled(pixbufboard[y][x], W, H, n, color, weight, scale);
						gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), p);
						g_object_unref(G_OBJECT(p));
					}
				}
			}
			else
			{
				int f = 0, _f = 0;
				int x, y;
				int bn = boardnumber[i][j];
				int bz = 3 + board[i][j] - 1;
				if (movepath[piecenum - 1] / boardsizew == i && movepath[piecenum - 1] % boardsizew == j)
					f = 2;
				if (refreshboardflag == 1)
				{
					if (specialrule == 3)
					{
						int k;
						for (k = 4; k < piecenum - 1; k++)
						{
							if (movepath[k] / boardsizew == i && movepath[k] % boardsizew == j)
							{
								_f = 2;
								bz -= k % 2;
								bn = 5;
								break;
							}
						}
						if (f)
						{
							bz -= (piecenum - 1) % 2;
							bn = 5;
						}
					}
				}

				if (shownumber)
				{
					y = imgtypeboard[i][j] % 9;
					x = bz;
					int W = gdk_pixbuf_get_width(pixbufboard[y][x]);
					int H = gdk_pixbuf_get_height(pixbufboard[y][x]);
					if (pixbufboardnumber[y][x][bn][(f || _f) ? 1 : 0] == NULL)
					{
						char n[10];
						sprintf(n, "%d", bn);
						if (f || _f)
						{
							pixbufboardnumber[y][x][bn][1] = draw_overlay(pixbufboard[y][x], W, H, n, "#FF0000");
						}
						else
						{
							if (boardnumber[i][j] % 2 == 1)
								pixbufboardnumber[y][x][bn][0] = draw_overlay(pixbufboard[y][x], W, H, n, "#FFFFFF");
							else
								pixbufboardnumber[y][x][bn][0] = draw_overlay(pixbufboard[y][x], W, H, n, "#000000");
						}
					}
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboardnumber[y][x][bn][(f || _f) ? 1 : 0]);
				}
				else
				{
					y = imgtypeboard[i][j] % 9;
					x = f + _f + bz;
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[y][x]);
				}
			}
		}
	}
}
void refresh_board()
{
	refresh_board_area(0, 0, boardsizew, boardsizeh);
}

void update_textpos()
{
	char posstring[1024];
	int offset, accumulatedOffset = 0;
	sprintf(posstring, "");
	for (int i = 0; i < piecenum; i++)
	{
		sprintf(posstring + accumulatedOffset, "%c%d%n", movepath[i] % boardsizew + 'a',
				boardsizeh - 1 - movepath[i] / boardsizew + 1, &offset);
		accumulatedOffset += offset;
	}
	gtk_text_buffer_set_text(buffertextpos, posstring, -1);
}

int is_legal_move(int y, int x)
{
	return y >= 0 && x >= 0 && y < boardsizeh && x < boardsizew && board[y][x] == 0;
}
void make_move(int y, int x)
{
	int i, j, k;

	board[y][x] = piecenum % 2 + 1;
	boardnumber[y][x] = piecenum + 1;
	if (movepath[piecenum] != y * boardsizew + x)
	{
		movepath[piecenum] = y * boardsizew + x;
		for (i = piecenum + 1; i < MAX_SIZE * MAX_SIZE; i++)
			movepath[i] = -1;
	}

	piecenum++;
	if (piecenum == boardsizeh * boardsizew)
		isgameover = 1;

	memset(bestline, 0, sizeof(bestline));

	boardbestX = boardbestY = -1;
	memset(boardlose, 0, sizeof(boardlose));
	memset(boardpos, 0, sizeof(boardpos));

	drawpieceonly = usedatabase;
	refresh_board();
	drawpieceonly = 0;

	update_textpos();

	for (i = 0; i < 8; i += 2)
	{
		int ny, nx;
		k = 1;
		ny = y;
		nx = x;
		for (j = 1; j < 6; j++)
		{
			ny += movy[i];
			nx += movx[i];
			if (nx < 0 || ny < 0 || nx >= boardsizew || ny >= boardsizeh)
				break;
			if (board[ny][nx] != board[y][x])
				break;
			k++;
		}
		ny = y;
		nx = x;
		for (j = 1; j < 6; j++)
		{
			ny -= movy[i];
			nx -= movx[i];
			if (nx < 0 || ny < 0 || nx >= boardsizew || ny >= boardsizeh)
				break;
			if (board[ny][nx] != board[y][x])
				break;
			k++;
		}
		if (k == 5 || (k > 5 && inforule != 1))
		{
			isgameover = 1;
			break;
		}
	}
}
void show_database()
{
	int i;
	char command[80];
	if (usedatabase)
	{
		sprintf(command, "yxquerydatabaseallt\n"); // query all & text
		send_command(command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(command);
		}
		sprintf(command, "done\n");
		send_command(command);
	}
}
void show_forbid()
{
	int i;
	char command[80];
	if ((computerside & 1) || (piecenum % 2))
	{
		memset(forbid, 0, sizeof(forbid));
		return;
	}
	sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
	send_command(command);
	sprintf(command, "yxboard\n");
	send_command(command);
	for (i = 0; i < piecenum; i++)
	{
		sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
				movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
		send_command(command);
	}
	sprintf(command, "done\n");
	send_command(command);
	sprintf(command, "yxshowforbid\n");
	send_command(command);
}

void show_dialog_undo_warning_query(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, language == 0 ? "The operation will stop the calculation. Do you want to continue?" : _T(clanguage[12]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case GTK_RESPONSE_YES:
		change_piece(window, (gpointer)1);
		break;
	case GTK_RESPONSE_NO:
		break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_swap_query2(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	char msg[80];
	char command[80];
	int i;

	sprintf(msg, "%s", language == 0 ? "Choose one option" : _T(clanguage[107]));
	dialog = gtk_dialog_new_with_buttons(msg, GTK_WINDOW(windowmain), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										 language == 0 ? "Stay with white" : _T(clanguage[108]), 1,
										 language == 0 ? "Swap" : _T(clanguage[109]), 2,
										 language == 0 ? "Add 2 more pieces" : _T(clanguage[110]), 3,
										 NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case 1:
		swap2done = 1;
		break;
	case 2:
		// swap
		timerhumanincrement += increment;
		if (computerside == 2)
		{
			change_side_menu(1, NULL);
			change_side_menu(-2, NULL);
		}
		else
		{
			change_side_menu(-1, NULL);
			change_side_menu(2, NULL);
		}
		if (language)
			printf_log(clanguage[14]);
		else
			printf_log("Swap");
		printf_log("\n");

		isthinking = 1;
		clock_timer_change_status(1);
		isneedrestart = 0;
		sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
		send_command(command);
		if (hashautoclear)
			send_command("yxhashclear\n");
		sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(command);
		sprintf(command, "board\n");
		send_command(command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(command);
		}
		sprintf(command, "done\n");
		send_command(command);

		swap2done = 1;
		break;
	case 3:
		// should do nothing
		break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_swap_query(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	char msg[80];
	if (specialrule == 3 && piecenum == 4)
		sprintf(msg, "%s (N=%d)", language == 0 ? "Swap?" : _T(clanguage[13]), move5N);
	else
		sprintf(msg, "%s", language == 0 ? "Swap?" : _T(clanguage[13]));
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, msg);
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case GTK_RESPONSE_YES:
		timerhumanincrement += increment;
		if (specialrule == 4)
		{
			int i;
			char command[80];
			isthinking = 1;
			clock_timer_change_status(1);
			isneedrestart = 0;
			sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
			send_command(command);
			if (hashautoclear)
				send_command("yxhashclear\n");
			sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
			send_command(command);
			sprintf(command, "board\n");
			send_command(command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
				send_command(command);
			}
			sprintf(command, "done\n");
			send_command(command);
		}
		else if (specialrule == 3)
		{
			isneedrestart = 1;
			if (computerside == 2)
			{
				change_side_menu(1, NULL);
				change_side_menu(-2, NULL);
			}
			else
			{
				change_side_menu(-1, NULL);
				change_side_menu(2, NULL);
			}
			if (piecenum == 3)
			{
				int i;
				char command[80];
				send_command("yxsoosorvstep3\n");
				for (i = 0; i < piecenum; i++)
				{
					sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
							movepath[i] % boardsizew);
					send_command(command);
				}
				send_command("done\n");
			}
			else //==4
			{
				int i;
				char command[80];
				sprintf(command, "yxsoosorvstep5 %d\n", move5N);
				send_command(command);
				for (i = 0; i < piecenum; i++)
				{
					sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
							movepath[i] % boardsizew);
					send_command(command);
				}
				send_command("done\n");
			}
		}
		else if (specialrule == 2)
		{
			isneedrestart = 1;
			make_move(4, 5);
			if (computerside == 2)
			{
				change_side_menu(1, NULL);
				change_side_menu(-2, NULL);
			}
			else
			{
				change_side_menu(-1, NULL);
				change_side_menu(2, NULL);
			}
		}
		if (language)
			printf_log(clanguage[14]);
		else
			printf_log("Swap");
		printf_log("\n");
		break;
	case GTK_RESPONSE_NO:
		if (specialrule == 4)
		{
			if (computerside == 2)
			{
				change_side_menu(1, NULL);
				change_side_menu(-2, NULL);
			}
			else
			{
				change_side_menu(-1, NULL);
				change_side_menu(2, NULL);
			}
		}
		printf_log("\n");
		break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_swap_info(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language == 0 ? "Swap" : _T(clanguage[15]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	isneedrestart = 1;
	if (computerside == 2)
	{
		change_side_menu(1, NULL);
		change_side_menu(-2, NULL);
	}
	else
	{
		change_side_menu(-1, NULL);
		change_side_menu(2, NULL);
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_timeout(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language == 0 ? "Time out!" : _T(clanguage[105]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void show_dialog_forbidden_info(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language == 0 ? "Forbidden Move" : _T(clanguage[98]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void show_dialog_illegal_opening(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language == 0 ? "Illegal Opening" : _T(clanguage[16]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	isneedrestart = 1;
	gtk_widget_destroy(dialog);
}

void show_dialog_boardtext(GtkWidget *window, int x, int y)
{
	if (databasereadonly)
		return;

	gchar text[80], command[100];
	const gchar *ptext;
	GtkWidget *dialog;
	GtkWidget *table;
	GtkWidget *label[1];
	GtkWidget *entry[1];
	gint result;

	dialog = gtk_dialog_new_with_buttons("Board Text", GTK_WINDOW(windowmain), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "Delete", 2, "OK", 1, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	table = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(table), 0);
	gtk_grid_set_column_spacing(GTK_GRID(table), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 3);

	label[0] = gtk_label_new(language == 0 ? "Input board text:" : _T(clanguage[114]));
	entry[0] = gtk_entry_new();
	gtk_entry_set_max_length(entry[0], 6);
	char *t_utf8 = _T(boardtext[y][x]);
	sprintf(text, "%s", t_utf8);
	gtk_entry_set_text(GTK_ENTRY(entry[0]), text);
	g_free(t_utf8);

	gtk_grid_attach(GTK_GRID(table), label[0], 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), entry[0], 1, 0, 1, 1);

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == 1 || result == 2)
	{
		ptext = gtk_entry_get_text(GTK_ENTRY(entry[0]));
		if (ptext[0] == '\0' || result == 2)
		{
			sprintf(command, "yxeditlabeldatabase %d,%d \n", y, x); // delete for empty text
		}
		else
		{
			sscanf(ptext, "%s", text);
			char *t = __invT(text);
			sprintf(command, "yxeditlabeldatabase %d,%d %s\n", y, x, t);
			g_free(t);
		}

		send_command(command);
		for (int i = 0; i < piecenum; i++)
		{
			sprintf(command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
			send_command(command);
		}
		send_command("done\n");
	}
	gtk_widget_destroy(dialog);
}

gboolean show_dbdelall_query()
{
	if (showdbdelconfirm == 0)
		return TRUE;

	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(windowmain),
									GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
									GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
									language == 0 ? "Are your sure to delete all branches?" : _T(clanguage[116]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Confirm deleting all branches");
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return result == GTK_RESPONSE_YES;
}

gboolean on_button_press_windowmain(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge)
{
	int x, y;
	int size;
	char command[80];
	if (!isthinking && event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == 1 || event->button == 2)
		{
			size = gdk_pixbuf_get_width(pixbufboard[0][0]);

			x = (int)(event->x);
			y = (int)(event->y);
			if (x < 0 || y < 0)
			{
				x = y = -1;
			}
			else
			{
				x = (int)(x / size);
				y = (int)(y / size);
			}
			if (x >= 0 && x < boardsizew && y >= 0 && y < boardsizeh && !isgameover)
			{
				if (usedatabase && (isctrlpressed || event->button == 2) && board[y][x] == 0 && (piecenum % 2 == 1 || !forbid[y][x]))
				{
					show_dialog_boardtext(widget, x, y);
					show_database();
					isctrlpressed = 0;
				}
				else if (specialrule == 4 && (piecenum < 3 || (piecenum < 5 && !swap2done)))
				{
					if (board[y][x] == 0)
					{
						if (piecenum < 2 && !(computerside & 1))
						{
							make_move(y, x);
						}
						else if (piecenum == 2 && !(computerside & 1))
						{
							int i;
							make_move(y, x);
							timerhumanincrement += increment;

							isthinking = 1;
							clock_timer_change_status(1);
							isneedrestart = 0;
							sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
							send_command(command);
							if (hashautoclear)
								send_command("yxhashclear\n");
							sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
							send_command(command);
							sprintf(command, "yxboard\n");
							send_command(command);
							for (i = 0; i < piecenum; i++)
							{
								sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
										movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
								send_command(command);
							}
							sprintf(command, "done\n");
							send_command(command);
							send_command("yxswap2step2\n");
						}
						else if (piecenum == 3 && !(computerside & 2) && !swap2done)
						{
							make_move(y, x);
						}
						else if (piecenum == 4 && !(computerside & 2) && !swap2done)
						{
							int i;
							make_move(y, x);
							timerhumanincrement += increment;

							isthinking = 1;
							clock_timer_change_status(1);
							isneedrestart = 0;
							sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
							send_command(command);
							if (hashautoclear)
								send_command("yxhashclear\n");
							sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
							send_command(command);
							sprintf(command, "yxboard\n");
							send_command(command);
							for (i = 0; i < piecenum; i++)
							{
								sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
										movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
								send_command(command);
							}
							sprintf(command, "done\n");
							send_command(command);
							send_command("yxswap2step3\n");
						}
					}
				}
				else if (!refreshboardflag && ((specialrule != 1 && specialrule != 3) || piecenum >= 3 || piecenum == 0) && (((computerside & 1) && piecenum % 2 == 0) || ((computerside & 2) && piecenum % 2 == 1)))
				{
					int i;
					// the first move of `swap after 1st move' rule
					if (specialrule == 2 && piecenum == 0 && computerside != 3)
					{
						isneedrestart = 1;
						make_move(2, 3);
						show_dialog_swap_query(widget);
					}
					else
					{
						isthinking = 1;
						clock_timer_change_status(1);
						isneedrestart = 0;
						sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
						send_command(command);
						if (hashautoclear)
							send_command("yxhashclear\n");
						sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
						send_command(command);
						sprintf(command, "board\n");
						send_command(command);
						for (i = 0; i < piecenum; i++)
						{
							sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
									movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
							send_command(command);
						}
						sprintf(command, "done\n");
						send_command(command);
					}
					show_database();
				}
				else
				{
					if (specialrule == 3 && refreshboardflag == 1 && computerside != 2)
					{
						if (boardnumber[y][x] >= 4)
						{
							isneedrestart = 1;
							refreshboardflag = 0;
							while (piecenum >= 5)
								change_piece(NULL, (gpointer)1);
							make_move(y, x);
						}
					}
					else if (board[y][x] == 0 && piecenum % 2 == 0 && forbid[y][x] && !refreshboardflag)
					{
						show_dialog_forbidden_info(widget);
					}
					else if (board[y][x] == 0 && (piecenum % 2 == 1 || !forbid[y][x] || refreshboardflag))
					{
						int i;
						int flag = 0;

						make_move(y, x);
						if (specialrule == 2 && piecenum == 1 && computerside != 0)
						{
							int _x, _y;
							_x = min(x, boardsizew - 1 - x);
							_y = min(y, boardsizeh - 1 - y);
							// the condition for swapping under `swap after 1st move' rule
							if ((((_x == 2 && _y == 3) || (_x == 3 && _y == 2)) && rand() % 2 == 1) || (_x > 1 && _y > 1 && _x + _y > 5))
							{
								show_dialog_swap_info(widget);
							}
						}
						if (specialrule == 3 && piecenum == 3 && /*computerside != 0 &&*/ computerside != 1)
						{
							// check whether the current opening is one of the 26 standard openings
							if (movepath[0] % boardsizew == boardsizew / 2 && movepath[0] / boardsizew == boardsizeh / 2 &&
								movepath[1] % boardsizew <= boardsizew / 2 + 1 && movepath[1] % boardsizew >= boardsizew / 2 - 1 &&
								movepath[1] / boardsizew <= boardsizeh / 2 + 1 && movepath[1] / boardsizew >= boardsizeh / 2 - 1 &&
								movepath[2] % boardsizew <= boardsizew / 2 + 2 && movepath[2] % boardsizew >= boardsizew / 2 - 2 &&
								movepath[2] / boardsizew <= boardsizeh / 2 + 2 && movepath[2] / boardsizew >= boardsizeh / 2 - 2)
							{
								// do nothing
							}
							else
							{
								show_dialog_illegal_opening(widget);
								new_game(NULL, NULL);
								flag = 1;
							}
						}
						if (specialrule == 3 && piecenum == 3 && computerside != 0 && computerside != 1)
						{
							timerhumanincrement += increment;
							sprintf(command, "yxsoosorvstep2\n");
							send_command(command);
							for (i = 0; i < piecenum; i++)
							{
								sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
										movepath[i] % boardsizew);
								send_command(command);
							}
							send_command("done\n");
							flag = 1;
						}
						if (specialrule == 3 && piecenum == 4 && computerside != 0 && computerside != 2)
						{
							show_dialog_move5N(widget, NULL);
							timerhumanincrement += increment;
							sprintf(command, "yxsoosorvstep4 %d\n", move5N);
							send_command(command);
							for (i = 0; i < piecenum; i++)
							{
								sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
										movepath[i] % boardsizew);
								send_command(command);
							}
							send_command("done\n");
							flag = 1;
						}
						if (specialrule == 3 && piecenum >= 5 && piecenum <= 5 + move5N - 1 && computerside != 0 && computerside != 1)
						{
							if (!refreshboardflag2)
							{
								refreshboardflag = 1;
							}
							if (refreshboardflag == 1)
							{
								flag = 1;
							}
						}
						if (specialrule == 3 && piecenum == 5 + move5N - 1 && computerside != 0 && computerside != 1 && refreshboardflag2 == 0)
						{
							refreshboardflag = 0;
							refreshboardflag2 = 1;
							isneedrestart = 1;
							timerhumanincrement += increment;
							sprintf(command, "yxsoosorvstep6\n");
							send_command(command);
							for (i = 0; i < piecenum; i++)
							{
								sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
										movepath[i] % boardsizew);
								send_command(command);
							}
							send_command("done\n");
							flag = 1;
						}

						show_forbid();
						if (!isgameover && !flag && ((specialrule != 1 && specialrule != 3) || piecenum >= 3) && (((computerside & 1) && piecenum % 2 == 0) || ((computerside & 2) && piecenum % 2 == 1)))
						{
							timerhumanincrement += increment;
							if (isneedrestart)
							{
								isthinking = 1;
								clock_timer_change_status(1);
								isneedrestart = 0;
								sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
								send_command(command);
								if (hashautoclear)
									send_command("yxhashclear\n");
								sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
								send_command(command);
								sprintf(command, "board\n");
								send_command(command);
								for (i = 0; i < piecenum; i++)
								{
									sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
											movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
									send_command(command);
								}
								sprintf(command, "done\n");
								send_command(command);
							}
							else
							{
								sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
								send_command(command);
								if (hashautoclear)
									send_command("yxhashclear\n");
								sprintf(command, "turn %d,%d\n", y, x);
								send_command(command);
								isthinking = 1;
								clock_timer_change_status(1);
							}
						}
					}
					show_database();
				}
				return TRUE;
			}
		}
	}
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		if (showwarning && isthinking)
			show_dialog_undo_warning_query(widget);
		else
			change_piece(widget, (gpointer)1);
		return TRUE;
	}
	return FALSE; /* why FALSE? */
}

int is_integer(const char *str)
{
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return 0;
		str++;
	}
	return 1;
}

void set_level(int x)
{
	gchar command[80];
	levelchoice = x;
	if (levelchoice == 1)
	{
		sprintf(command, "INFO timeout_turn %lld\n", timeoutturn);
		send_command(command);
		sprintf(command, "INFO timeout_match %lld\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
		send_command(command);
		sprintf(command, "INFO max_node %lld\n", maxnode); // now it should not be -1
		send_command(command);
		sprintf(command, "INFO max_depth %d\n", maxdepth);
		send_command(command);
		sprintf(command, "INFO time_increment %d\n", increment);
		send_command(command);
	}
	else
	{
		switch (levelchoice)
		{
		case 0:
			sprintf(command, "INFO max_node %d\n", -1);
			send_command(command);
			break;
		case 2:
			sprintf(command, "INFO max_node %lld\n", 100 * 1000); // 100k
			send_command(command);
			break;
		case 3:
			sprintf(command, "INFO max_node %lld\n", 500 * 1000); // 500k
			send_command(command);
			break;
		case 4:
			sprintf(command, "INFO max_node %lld\n", 1 * 1000000); // 1M
			send_command(command);
			break;
		case 5:
			sprintf(command, "INFO max_node %lld\n", 5 * 1000000); // 5M
			send_command(command);
			break;
		case 6:
			sprintf(command, "INFO max_node %lld\n", 10 * 1000000); // 10M
			send_command(command);
			break;
		case 7:
			sprintf(command, "INFO max_node %lld\n", 20 * 1000000); // 20M
			send_command(command);
			break;
		case 8:
			sprintf(command, "INFO max_node %lld\n", 50 * 1000000); // 50M
			send_command(command);
			break;
		case 9:
			sprintf(command, "INFO max_node %lld\n", 100 * 1000000); // 100M
			send_command(command);
			break;
		case 10:
			sprintf(command, "INFO max_node %lld\n", 200 * 1000000); // 200M
			send_command(command);
			break;
		case 11:
			sprintf(command, "INFO max_node %lld\n", 500 * 1000000); // 500M
			send_command(command);
			break;
		case 12:
			sprintf(command, "INFO max_node %lld\n", 1000 * 1000000); // 1000M
			send_command(command);
			break;
		}
		timeoutmatch = 100000000;
		timeoutturn = 2000000;
		increment = 0;
		sprintf(command, "INFO timeout_match %lld\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO time_left %lld\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO timeout_turn %lld\n", timeoutturn);
		send_command(command);
		sprintf(command, "INFO max_depth %d\n", boardsizeh * boardsizew);
		send_command(command);
		sprintf(command, "INFO time_increment %d\n", increment);
		send_command(command);
	}
}

void set_cautionfactor(int x)
{
	gchar command[80];
	if (x < 0)
		x = 0;
	if (x > CAUTION_NUM)
		x = CAUTION_NUM;
	cautionfactor = x;
	sprintf(command, "INFO caution_factor %d\n", cautionfactor);
	send_command(command);
}

void set_threadnum(int x)
{
	gchar command[80];
	if (x < 1)
		x = 1;
	// if(x > maxthreadnum) x = maxthreadnum;
	threadnum = x;
	sprintf(command, "INFO thread_num %d\n", threadnum);
	send_command(command);
}

void set_hashsize(int x)
{
	gchar command[80];
	if (x <= 0)
		x = 1;
	if (x > maxhashsizemb)
		x = maxhashsizemb;
	hashsizemb = x;
	sprintf(command, "INFO hash_size %lld\n", (long long)hashsizemb << 10);
	send_command(command);
}

void set_pondering(int x)
{
	gchar command[80];
	if (x < 0)
		x = 0;
	if (x > 1)
		x = 1;
	infopondering = x;
	sprintf(command, "INFO pondering %d\n", infopondering);
	send_command(command);
}

void setvcthread(int x)
{
	gchar command[80];
	if (x < 0)
		x = 0;
	if (x > 2)
		x = 2;
	infovcthread = x;
	sprintf(command, "INFO vcthread %d\n", infovcthread);
	send_command(command);
}

void show_dialog_settings_custom_entry(GtkWidget *widget, gpointer data)
{
	static GtkWidget *editable[2];
	static int flag = 0;
	if (widget == NULL)
	{
		if (data == 0)
		{
			flag = 0;
		}
		else
		{
			editable[flag] = (GtkWidget *)data;
			flag++;
		}
		return;
	}

	if (GPOINTER_TO_INT(data) == 0) // unlimited time
	{
		gtk_widget_set_visible(editable[0], FALSE);
		gtk_widget_set_visible(editable[1], FALSE);
	}
	else if (GPOINTER_TO_INT(data) == 1) // custom level
	{
		gtk_widget_set_visible(editable[0], TRUE);
		gtk_widget_set_visible(editable[1], FALSE);
	}
	else // if (data >= 2) //predefined level
	{
		gtk_widget_set_visible(editable[0], FALSE);
		gtk_widget_set_visible(editable[1], TRUE);
	}
}

void show_dialog_settings(GtkWidget *widget, gpointer data)
{
	int i;
	// gchar command[80];
	gchar text[80];
	const gchar *ptext;
	GtkWidget *dialog;
	GtkWidget *notebook;
	GtkWidget *notebookvbox[3];
	GtkWidget *hbox[12];
	GtkWidget *radiolevel[3];
	GtkWidget *radiovcthread[3];
	GtkWidget *labeltimeturn[2], *labeltimematch[2], *labelmaxdepth[2], *labelmaxnode[2], *labelincrement[2], *labelblank[9];
	GtkWidget *entrytimeturn, *entrytimematch, *entrymaxdepth, *entrymaxnode, *entryincrement;
	GtkWidget *scalelevel, *scalecaution, *scalethreads, *scalesplitdepth, *scalehash;
	GtkWidget *tablesetting;
	gint result;

	show_dialog_settings_custom_entry(NULL, 0);

	dialog = gtk_dialog_new_with_buttons("Settings", data, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), notebook, FALSE, FALSE, 3);
	notebookvbox[0] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[0], gtk_label_new(language == 0 ? "Level" : _T(clanguage[18])));
	notebookvbox[1] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[1], gtk_label_new(language == 0 ? "Style" : _T(clanguage[19])));
	notebookvbox[2] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[2], gtk_label_new(language == 0 ? "Resource" : _T(clanguage[20])));

	for (i = 0; i < 9; i++)
	{
		labelblank[i] = gtk_label_new(" ");
		gtk_label_set_width_chars(GTK_LABEL(labelblank[i]), 6);
	}

	labeltimeturn[0] = gtk_label_new(language == 0 ? "Turn time:" : _T(clanguage[21]));
	entrytimeturn = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrytimeturn), 9);
	sprintf(text, "%lld", timeoutturn / 1000);
	gtk_entry_set_text(GTK_ENTRY(entrytimeturn), text);
	labeltimeturn[1] = gtk_label_new(language == 0 ? "s" : _T(clanguage[22]));
	gtk_widget_set_halign(labeltimeturn[0], GTK_ALIGN_END);
	gtk_widget_set_valign(labeltimeturn[0], GTK_ALIGN_CENTER);
	gtk_widget_set_halign(labeltimeturn[1], GTK_ALIGN_START);
	gtk_widget_set_valign(labeltimeturn[1], GTK_ALIGN_CENTER);

	labeltimematch[0] = gtk_label_new(language == 0 ? "Match time:" : _T(clanguage[23]));
	entrytimematch = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrytimematch), 9);
	sprintf(text, "%lld", timeoutmatch / 1000);
	gtk_entry_set_text(GTK_ENTRY(entrytimematch), text);
	labeltimematch[1] = gtk_label_new(language == 0 ? "s" : _T(clanguage[22]));
	gtk_widget_set_halign(labeltimematch[0], GTK_ALIGN_END);
	gtk_widget_set_valign(labeltimematch[0], GTK_ALIGN_CENTER);
	gtk_widget_set_halign(labeltimematch[1], GTK_ALIGN_START);
	gtk_widget_set_valign(labeltimematch[1], GTK_ALIGN_CENTER);

	labelmaxdepth[0] = gtk_label_new(language == 0 ? "Max depth:" : _T(clanguage[24]));
	entrymaxdepth = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrymaxdepth), 3);
	sprintf(text, "%d", maxdepth);
	gtk_entry_set_text(GTK_ENTRY(entrymaxdepth), text);
	labelmaxdepth[1] = gtk_label_new(language == 0 ? "ply" : _T(clanguage[25]));
	gtk_widget_set_halign(labelmaxdepth[0], GTK_ALIGN_END);
	gtk_widget_set_valign(labelmaxdepth[0], GTK_ALIGN_CENTER);
	gtk_widget_set_halign(labelmaxdepth[1], GTK_ALIGN_START);
	gtk_widget_set_valign(labelmaxdepth[1], GTK_ALIGN_CENTER);

	labelmaxnode[0] = gtk_label_new(language == 0 ? "Max node number:" : _T(clanguage[26]));
	entrymaxnode = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrymaxnode), 9);
	sprintf(text, "%lld", maxnode / 1000);
	gtk_entry_set_text(GTK_ENTRY(entrymaxnode), text);
	labelmaxnode[1] = gtk_label_new(language == 0 ? "K" : _T(clanguage[27]));
	gtk_widget_set_halign(labelmaxnode[0], GTK_ALIGN_END);
	gtk_widget_set_valign(labelmaxnode[0], GTK_ALIGN_CENTER);
	gtk_widget_set_halign(labelmaxnode[1], GTK_ALIGN_START);
	gtk_widget_set_valign(labelmaxnode[1], GTK_ALIGN_CENTER);

	labelincrement[0] = gtk_label_new(language == 0 ? "Increment:" : _T(clanguage[95]));
	entryincrement = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entryincrement), 6);
	sprintf(text, "%d", increment / 1000);
	gtk_entry_set_text(GTK_ENTRY(entryincrement), text);
	labelincrement[1] = gtk_label_new(language == 0 ? "s / move" : _T(clanguage[96]));
	gtk_widget_set_halign(labelincrement[0], GTK_ALIGN_END);
	gtk_widget_set_valign(labelincrement[0], GTK_ALIGN_CENTER);
	gtk_widget_set_halign(labelincrement[1], GTK_ALIGN_START);
	gtk_widget_set_valign(labelincrement[1], GTK_ALIGN_CENTER);

	scalelevel = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1, 12, 1);
	if (levelchoice < 2)
		gtk_range_set_value(GTK_RANGE(scalelevel), 1);
	else
		gtk_range_set_value(GTK_RANGE(scalelevel), levelchoice - 1);
	gtk_widget_set_size_request(scalelevel, 100, -1);

	radiolevel[0] = gtk_radio_button_new_with_label(NULL, language == 0 ? "Unlimited Time" : _T(clanguage[36]));
	radiolevel[1] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[0])), language == 0 ? "Custom Level" : _T(clanguage[32]));
	radiolevel[2] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[1])), language == 0 ? "Predefined Level" : _T(clanguage[28]));

	g_signal_connect(G_OBJECT(radiolevel[0]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[1]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)1);
	g_signal_connect(G_OBJECT(radiolevel[2]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)2);

	tablesetting = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(tablesetting), 0);
	gtk_grid_set_column_spacing(GTK_GRID(tablesetting), 0);

	// 第0行
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[0], 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labeltimeturn[0], 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), entrytimeturn, 2, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labeltimeturn[1], 3, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[1], 4, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labeltimematch[0], 5, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), entrytimematch, 6, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labeltimematch[1], 7, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[2], 8, 0, 1, 1);

	// 第1行
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[3], 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelincrement[0], 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), entryincrement, 2, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelincrement[1], 3, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[4], 4, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelmaxnode[0], 5, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), entrymaxnode, 6, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelmaxnode[1], 7, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[5], 8, 1, 1, 1);

	// 第2行
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[6], 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelmaxdepth[0], 1, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), entrymaxdepth, 2, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelmaxdepth[1], 3, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[7], 4, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(tablesetting), labelblank[8], 8, 2, 1, 1);

	hbox[11] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox[11]), gtk_label_new("     "), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[11]), gtk_label_new(language == 0 ? "Fast" : _T(clanguage[29])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[11]), scalelevel, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[11]), gtk_label_new(language == 0 ? "Slow" : _T(clanguage[30])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[11]), gtk_label_new("     "), FALSE, FALSE, 3);

	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[0], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[2], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), hbox[11], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[1], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), tablesetting, FALSE, FALSE, 3);

	show_dialog_settings_custom_entry(NULL, (gpointer)tablesetting);
	show_dialog_settings_custom_entry(NULL, (gpointer)hbox[11]);

	scalecaution = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, CAUTION_NUM, 1);
	// gtk_scale_set_value_pos(GTK_SCALE(scalecaution), GTK_POS_LEFT);
	gtk_range_set_value(GTK_RANGE(scalecaution), cautionfactor);
	gtk_widget_set_size_request(scalecaution, 100, -1);

	GtkAdjustment *adjustment_threads = gtk_adjustment_new(threadnum, 1, maxthreadnum, 1, 1, 1);
	scalethreads = gtk_spin_button_new(adjustment_threads, 1, 0);

	GtkAdjustment *adjustment_hashsize = gtk_adjustment_new(hashsizemb, 0, maxhashsizemb, 256, 256, 256);
	scalehash = gtk_spin_button_new(adjustment_hashsize, 1, 0);

	radiovcthread[0] = gtk_radio_button_new_with_label(NULL, language == 0 ? "None" : _T(clanguage[87]));
	radiovcthread[1] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiovcthread[0])), language == 0 ? "Check VCT" : _T(clanguage[86]));
	radiovcthread[2] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiovcthread[1])), language == 0 ? "Check VC2" : _T(clanguage[88]));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiovcthread[infovcthread]), TRUE);

	hbox[8] = radiovcthread[0];
	hbox[9] = radiovcthread[1];
	hbox[10] = radiovcthread[2];

	hbox[7] = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

	hbox[1] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox[1]), gtk_label_new(language == 0 ? "Rash" : _T(clanguage[37])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[1]), scalecaution, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[1]), gtk_label_new(language == 0 ? "Cautious" : _T(clanguage[38])), FALSE, FALSE, 3);

	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), gtk_label_new(language == 0 ? "Additional Threat Check in Global Search:" : _T(clanguage[31])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[8], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[9], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[10], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[7], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[1], FALSE, FALSE, 3);

	if (maxthreadnum > 1)
	{
		hbox[2] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_pack_start(GTK_BOX(hbox[2]), gtk_label_new(language == 0 ? "Number of Threads" : _T(clanguage[39])), FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(hbox[2]), scalethreads, FALSE, FALSE, 3);
		hbox[4] = NULL;
		// hbox[4] = gtk_hbox_new(FALSE, 0);
		// gtk_box_pack_start(GTK_BOX(hbox[4]), gtk_label_new(language == 0 ? "Split Depth" : _T(clanguage[40])), FALSE, FALSE, 3);
		// gtk_box_pack_start(GTK_BOX(hbox[4]), scalesplitdepth, FALSE, FALSE, 3);
	}

	hbox[3] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox[3]), gtk_label_new(language == 0 ? "Hash Size" : _T(clanguage[41])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[3]), scalehash, FALSE, FALSE, 3);

	hbox[5] = gtk_check_button_new_with_label(language == 0 ? "Pondering" : _T(clanguage[85]));
	gtk_toggle_button_set_active(hbox[5], infopondering ? TRUE : FALSE);

	hbox[6] = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[5], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[6], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[2], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[4], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[3], FALSE, FALSE, 3);

	gtk_widget_show_all(dialog);

	if (levelchoice == 0)
	{
		show_dialog_settings_custom_entry(widget, (gpointer)0);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiolevel[0]), TRUE);
	}
	else if (levelchoice == 1)
	{
		show_dialog_settings_custom_entry(widget, (gpointer)1);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiolevel[1]), TRUE);
	}
	else
	{
		show_dialog_settings_custom_entry(widget, (gpointer)2);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiolevel[2]), TRUE);
	}

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case 1:
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[1])))
		{
			ptext = gtk_entry_get_text(GTK_ENTRY(entrytimeturn));
			if (is_integer(ptext))
			{
				sscanf(ptext, "%lld", &timeoutturn);
				timeoutturn = 1000 * max(timeoutturn, 0);
				if (timeoutturn == 0)
					timeoutturn = 100;
			}
			ptext = gtk_entry_get_text(GTK_ENTRY(entrytimematch));
			if (is_integer(ptext))
			{
				sscanf(ptext, "%lld", &timeoutmatch);
				timeoutmatch = 1000 * max(timeoutmatch, 1);
				if (timeoutmatch < timeoutturn)
					timeoutmatch = timeoutturn;
			}
			ptext = gtk_entry_get_text(GTK_ENTRY(entrymaxdepth));
			if (is_integer(ptext))
			{
				sscanf(ptext, "%d", &maxdepth);
				if (maxdepth > boardsizeh * boardsizew)
					maxdepth = boardsizeh * boardsizew;
				if (maxdepth < 2)
					maxdepth = 2;
			}
			ptext = gtk_entry_get_text(GTK_ENTRY(entrymaxnode));
			if (is_integer(ptext))
			{
				sscanf(ptext, "%lld", &maxnode);
				maxnode = 1000 * max(maxnode, 1);
			}
			ptext = gtk_entry_get_text(GTK_ENTRY(entryincrement));
			if (is_integer(ptext))
			{
				sscanf(ptext, "%d", &increment);
				if (increment > 10000)
					increment = 10000;
				if (increment < 0)
					increment = 0;
				increment *= 1000;
			}
			set_level(1);
		}
		else
		{
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[0])))
			{
				set_level(0);
			}
			else
			{
				set_level((int)(gtk_range_get_value(GTK_RANGE(scalelevel)) + 1 + 1e-8));
			}
		}

		if (gtk_range_get_value(GTK_RANGE(scalecaution)) < 0)
			set_cautionfactor((int)(gtk_range_get_value(GTK_RANGE(scalecaution)) - 1e-8));
		else
			set_cautionfactor((int)(gtk_range_get_value(GTK_RANGE(scalecaution)) + 1e-8));

		set_threadnum((int)(gtk_spin_button_get_value(GTK_SPIN_BUTTON(scalethreads)) + 1e-8));

		set_hashsize((int)(gtk_spin_button_get_value(GTK_SPIN_BUTTON(scalehash)) + 1e-8));

		set_pondering(gtk_toggle_button_get_active(hbox[5]) == TRUE ? 1 : 0);

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiovcthread[0])))
			setvcthread(0);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiovcthread[1])))
			setvcthread(1);
		else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiovcthread[2])))
			setvcthread(2);
		break;
	case 2:
		break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_load(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	FILE *in;
	dialog = gtk_file_chooser_dialog_new("Load", data, GTK_FILE_CHOOSER_ACTION_OPEN,
										 "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Yixin saved positions");
	gtk_file_filter_add_pattern(filter, "*.[Ss][Aa][Vv]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "POS");
	gtk_file_filter_add_pattern(filter, "*.[Pp][Oo][Ss]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Piskvork saved positions");
	gtk_file_filter_add_pattern(filter, "*.[Pp][Ss][Qq]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		int nl;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		printf_log("%s\n", filename);
		nl = strlen(filename);
		if ((filename[nl - 3] == 'P' || filename[nl - 3] == 'p') &&
			(filename[nl - 2] == 'O' || filename[nl - 2] == 'o') &&
			(filename[nl - 1] == 'S' || filename[nl - 1] == 's'))
		{
			if ((in = fopen(filename, "rb")) != NULL)
			{
				int i;
				unsigned char num;
				new_game(NULL, NULL);
				fread(&num, 1, 1, in);
				for (i = 0; i < num; i++)
				{
					unsigned char xy;
					int x, y;
					fread(&xy, 1, 1, in);
					x = xy % 15;
					y = xy / 15;
					make_move(x, y);
					// printf_log("[%d/%d] %d %d (%d)\n", i, (int)num, x, y, (int)xy);
				}
				fclose(in);
				show_forbid();
				show_database();
			}
		}
		else if ((filename[nl - 3] == 'S' || filename[nl - 3] == 's') &&
				 (filename[nl - 2] == 'A' || filename[nl - 2] == 'a') &&
				 (filename[nl - 1] == 'V' || filename[nl - 1] == 'v'))
		{
			if ((in = fopen(filename, "r")) != NULL)
			{
				int i, num;
				new_game(NULL, NULL);
				fscanf(in, "%*d"); // TODO: use boardsizeh?
				fscanf(in, "%*d"); // TODO: use boardsizew or inforule?
				/*
				fscanf(in, "%d", &inforule);
				switch(inforule)
				{
					case 0: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule1), TRUE); break;
					case 1: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule2), TRUE); break;
					case 2: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule3), TRUE); break;
					case 3: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule4), TRUE); break;
				}
				*/
				change_rule(NULL, GINT_TO_POINTER(inforule)); // fixed pointer conversion

				fscanf(in, "%d", &num);
				for (i = 0; i < num; i++)
				{
					int x, y;
					fscanf(in, "%d %d", &x, &y);
					make_move(x, y);
				}
				fclose(in);
				show_forbid();
				show_database();
			}
		}
		else if ((filename[nl - 3] == 'P' || filename[nl - 3] == 'p') &&
				 (filename[nl - 2] == 'S' || filename[nl - 2] == 's') &&
				 (filename[nl - 1] == 'Q' || filename[nl - 1] == 'q'))
		{
			if ((in = fopen(filename, "r")) != NULL)
			{
				int x, y;
				char line[80];
				new_game(NULL, NULL);
				fscanf(in, "%*[^\n]%*c"); // TODO: use boardsizeh, boardsizew, etc.?

				fscanf(in, "%[^\n]%*c", line);
				if (line[0] >= '0' && line[0] <= '9')
				{
					sscanf(line, "%d", &x);
					while (x != -1)
					{
						if (sscanf(line, "%*d,%d,%*d", &y) == 0)
							break;
						make_move(y - 1, x - 1);
						if (fscanf(in, "%[^\n]%*c", line) == EOF)
							break;
						if (line[0] >= '0' && line[0] <= '9')
						{
							if (sscanf(line, "%d", &x) == 0)
								break;
						}
						else
							break;
					}
				}
				fclose(in);
				show_forbid();
				show_database();
			}
		}
		g_free(filenameutf);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}
void show_dialog_save(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	FILE *out;
	dialog = gtk_file_chooser_dialog_new("Save", data, GTK_FILE_CHOOSER_ACTION_SAVE,
										 "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Yixin saved positions");
	gtk_file_filter_add_pattern(filter, "*.[Ss][Aa][Vv]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		char _filename[256];
		int len;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		len = strlen(filename);
		if (len >= 4 && (filename[len - 1] == 'V' || filename[len - 1] == 'v') &&
			(filename[len - 2] == 'A' || filename[len - 2] == 'a') &&
			(filename[len - 3] == 'S' || filename[len - 3] == 's') &&
			(filename[len - 4] == '.'))
		{
			sprintf(_filename, "%s", filename);
		}
		else
		{
			sprintf(_filename, "%s.sav", filename);
		}
		printf_log("%s\n", _filename);
		if ((out = fopen(_filename, "w")) != NULL)
		{
			int i;
			fprintf(out, "%d\n", boardsizeh);
			fprintf(out, "%d\n", boardsizew);
			// fprintf(out, "%d\n", inforule);
			fprintf(out, "%d\n", piecenum);
			for (i = 0; i < piecenum; i++)
			{
				fprintf(out, "%d %d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
			}
			fclose(out);
		}
		g_free(filenameutf);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

const char *show_dialog_dbselect(GtkWidget *widget, gpointer data, int isSave)
{
	static char _filename[256];

	GtkWidget *dialog;
	GtkFileFilter *filter;
	dialog = gtk_file_chooser_dialog_new("Select database file", data,
										 (isSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN),
										 "_Cancel", GTK_RESPONSE_CANCEL, (isSave ? "_Save" : "_Open"), GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Yixin database");
	gtk_file_filter_add_pattern(filter, "*.[Dd][Bb]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		int len;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		len = strlen(filename);
		if (len >= 3 && (filename[len - 1] == 'B' || filename[len - 1] == 'b') &&
			(filename[len - 2] == 'D' || filename[len - 2] == 'd') &&
			(filename[len - 3] == '.'))
		{
			sprintf(_filename, "%s", filename);
		}
		else
		{
			sprintf(_filename, "%s.db", filename);
		}
		printf_log("%s\n", _filename);

		len = strlen(_filename);
		_filename[len] = '\n';
		_filename[len + 1] = 0;

		g_free(filenameutf);
		g_free(filename);
		gtk_widget_destroy(dialog);
		return _filename;
	}
	else
	{
		gtk_widget_destroy(dialog);
		return NULL;
	}
}
const char *show_dialog_libselect(GtkWidget *widget, gpointer data, int isSave)
{
	static char _filename[256];

	GtkWidget *dialog;
	GtkFileFilter *filter;
	dialog = gtk_file_chooser_dialog_new("Select library file", data,
										 (isSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN),
										 "_Cancel", GTK_RESPONSE_CANCEL, (isSave ? "_Save" : "_Open"), GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Library file");
	gtk_file_filter_add_pattern(filter, "*.[Ll][Ii][Bb]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		int len;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		len = strlen(filename);
		if (len >= 3 &&
			(filename[len - 1] == 'B' || filename[len - 1] == 'b') &&
			(filename[len - 2] == 'I' || filename[len - 2] == 'i') &&
			(filename[len - 3] == 'L' || filename[len - 3] == 'l') &&
			(filename[len - 4] == '.'))
		{
			sprintf(_filename, "%s", filename);
		}
		else
		{
			sprintf(_filename, "%s.lib", filename);
		}
		printf_log("%s\n", _filename);

		len = strlen(_filename);
		_filename[len] = '\n';
		_filename[len + 1] = 0;

		g_free(filenameutf);
		g_free(filename);
		gtk_widget_destroy(dialog);
		return _filename;
	}
	else
	{
		gtk_widget_destroy(dialog);
		return NULL;
	}
}

void show_dialog_move5N(GtkWidget *widget, gpointer data)
{
	gchar text[80];
	const gchar *ptext[1];
	GtkWidget *dialog;
	GtkWidget *table;
	GtkWidget *label[1];
	GtkWidget *entry[1];
	gint result;
	int done = 0;

	dialog = gtk_dialog_new_with_buttons("Number of 5th Moves", GTK_WINDOW(windowmain), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	table = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(table), 0);
	gtk_grid_set_column_spacing(GTK_GRID(table), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 3);

	label[0] = gtk_label_new(language == 0 ? "Input N (1-8):" : _T(clanguage[104]));

	entry[0] = gtk_entry_new();
	sprintf(text, "");
	gtk_entry_set_text(GTK_ENTRY(entry[0]), text);

	gtk_grid_attach(GTK_GRID(table), label[0], 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), entry[0], 1, 0, 1, 1);

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case 1:
		ptext[0] = gtk_entry_get_text(GTK_ENTRY(entry[0]));
		if (is_integer(ptext[0]))
		{
			sscanf(ptext[0], "%d", &move5N);
			if (move5N >= 1 && move5N <= 8)
				done = 1;
		}
		break;
	}
	gtk_widget_destroy(dialog);
	if (!done)
	{
		show_dialog_move5N(widget, data);
	}
}

void show_dialog_size(GtkWidget *widget, gpointer data)
{
	gchar text[80];
	const gchar *ptext[2];
	GtkWidget *dialog;
	GtkWidget *table;
	GtkWidget *label[2];
	GtkWidget *entry[2];
	gint result;

	dialog = gtk_dialog_new_with_buttons("Settings", GTK_WINDOW(windowmain), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	table = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(table), 0);
	gtk_grid_set_column_spacing(GTK_GRID(table), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 3);

	// label[0] = gtk_label_new(language == 0 ? "Board Height (5 ~ 22):" : _T(clanguage[42]));
	label[1] = gtk_label_new(language == 0 ? "Board Width (5 ~ 22):" : _T(clanguage[43]));

	// entry[0] = gtk_entry_new();
	// sprintf(text, "%d", boardsizeh);
	// gtk_entry_set_text(GTK_ENTRY(entry[0]), text);

	entry[1] = gtk_entry_new();
	sprintf(text, "%d", boardsizew);
	gtk_entry_set_text(GTK_ENTRY(entry[1]), text);

	// gtk_grid_attach(GTK_GRID(table), label[0], 0, 0, 1, 1);
	// gtk_grid_attach(GTK_GRID(table), entry[0], 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label[1], 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), entry[1], 1, 1, 1, 1);

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case 1:
		// ptext[0] = gtk_entry_get_text(GTK_ENTRY(entry[0]));
		ptext[1] = gtk_entry_get_text(GTK_ENTRY(entry[1]));
		if (/*is_integer(ptext[0]) &&*/ is_integer(ptext[1]))
		{
			int /*s1,*/ s2;
			// sscanf(ptext[0], "%d", &s1);
			sscanf(ptext[1], "%d", &s2);
			if (/*s1 <= MAX_SIZE && s1 >= 5 &&*/ s2 <= MAX_SIZE && s2 >= 5)
			{
				rboardsizeh = s2; // s1;
				rboardsizew = s2;
				if (boardsizeh != /*s1*/ s2 || boardsizew != s2)
				{
					respawn = 1;
					yixin_quit();
				}
			}
		}
		break;
	case 2:
		break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_custom_toolbar(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *label[3];
	GtkWidget *table;
	GtkWidget *entry;
	GtkWidget *combo;
	GtkWidget *textcommand;
	GtkWidget *scrolledcommand;
	GtkCellRenderer *renderer;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTextBuffer *buffercommand;
	GtkTextIter start, end;
	int result;
	int i, icon_count, pi;
	const gchar *ptext;
	char text[40];

	dialog = gtk_dialog_new_with_buttons(_T(clanguage[119]), windowmain,
										 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
										 _T(clanguage[117]), GTK_RESPONSE_ACCEPT,
										 _T(clanguage[118]), GTK_RESPONSE_REJECT, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	icon_count = pi = 0;

	GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
	GList *icon_list = gtk_icon_theme_list_icons(icon_theme, NULL);

	for (GList *iter = icon_list; iter != NULL; iter = iter->next)
	{
		char *icon_name = (char *)iter->data;
		if (strstr(icon_name, "symbolic") != NULL)
			continue;
		if (gtk_icon_theme_has_icon(icon_theme, icon_name))
		{
			GdkPixbuf *pixbuf = gtk_icon_theme_load_icon(icon_theme, icon_name,
														 GTK_ICON_SIZE_BUTTON, 0, NULL);
			if (pixbuf)
			{
				GtkTreeIter iter;
				icon_count++;

				gtk_list_store_append(store, &iter);
				gtk_list_store_set(store, &iter, 0, pixbuf, 1, icon_name, -1);
				g_object_unref(pixbuf);

				if (strcmp(icon_name, toolbaricon[GPOINTER_TO_INT(data)]) == 0)
				{
					pi = icon_count - 1;
				}
			}
		}
		g_free(icon_name);
	}
	g_list_free(icon_list);

	model = GTK_TREE_MODEL(store);
	combo = gtk_combo_box_new_with_model(model);
	g_object_unref(model);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "pixbuf", 0, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", 1, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), pi);

	entry = gtk_entry_new();
	sprintf(text, "%d", toolbarlng[GPOINTER_TO_INT(data)]);
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	label[0] = gtk_label_new(language == 0 ? "Icon:" : _T(clanguage[89]));
	label[1] = gtk_label_new(language == 0 ? "Text:" : _T(clanguage[90]));
	label[2] = gtk_label_new(language == 0 ? "Command:" : _T(clanguage[91]));

	textcommand = gtk_text_view_new();
	buffercommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledcommand = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledcommand), textcommand);
	gtk_widget_set_size_request(scrolledcommand, 200, 200);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffercommand), &end, toolbarcommand[GPOINTER_TO_INT(data)], strlen(toolbarcommand[GPOINTER_TO_INT(data)]));

	table = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(table), 0);
	gtk_grid_set_column_spacing(GTK_GRID(table), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 3);

	gtk_grid_attach(GTK_GRID(table), label[0], 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), combo, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label[1], 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), entry, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label[2], 0, 2, 2, 1);
	gtk_grid_attach(GTK_GRID(table), scrolledcommand, 0, 3, 2, 1);
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case GTK_RESPONSE_ACCEPT:
	{
		gchar *command;
		ptext = gtk_entry_get_text(GTK_ENTRY(entry));
		if (is_integer(ptext))
			sscanf(ptext, "%d", &toolbarlng[GPOINTER_TO_INT(data)]);
		pi = gtk_combo_box_get_active(combo);

		if (pi >= 0 && pi < icon_count)
		{
			GtkTreeIter iter;
			gchar *text;
			if (gtk_combo_box_get_active_iter(combo, &iter))
			{
				gtk_tree_model_get(model, &iter, 1, &text, -1);
				toolbaricon[GPOINTER_TO_INT(data)] = strdup(text);
				g_free(text);
			}
		}

		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffercommand), &start, &end, FALSE);
		strcpy(toolbarcommand[GPOINTER_TO_INT(data)], command);

		respawn = 1;
		yixin_quit();
	}
	case GTK_RESPONSE_REJECT:
		break;
	}

	gtk_widget_destroy(dialog);
}

void show_dialog_custom_hotkey(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *label[2];
	GtkWidget *table;
	GtkWidget *combo;
	GtkTextBuffer *buffercommand;
	GtkWidget *scrolledcommand;
	GtkWidget *textcommand;
	gint result;
	GtkTextIter start, end;

	int i;

	dialog = gtk_dialog_new_with_buttons("Custom Hotkey", windowmain, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	combo = gtk_combo_box_text_new();
	for (i = 0; hotkeynamelist[i] != NULL; i++)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), hotkeynamelist[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), hotkeykey[GPOINTER_TO_INT(data)]);

	label[0] = gtk_label_new(language == 0 ? "Hotkey:" : _T(clanguage[93]));
	label[1] = gtk_label_new(language == 0 ? "Command:" : _T(clanguage[91]));

	textcommand = gtk_text_view_new();
	buffercommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledcommand = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledcommand), textcommand);
	gtk_widget_set_size_request(scrolledcommand, 200, 200);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffercommand), &end, hotkeycommand[GPOINTER_TO_INT(data)], strlen(hotkeycommand[GPOINTER_TO_INT(data)]));

	table = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(table), 0);
	gtk_grid_set_column_spacing(GTK_GRID(table), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), table, FALSE, FALSE, 3);

	gtk_grid_attach(GTK_GRID(table), label[0], 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), combo, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), label[1], 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), scrolledcommand, 0, 2, 2, 1);
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case 1:
	{
		gchar *command;

		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffercommand), &start, &end, FALSE);
		strcpy(hotkeycommand[GPOINTER_TO_INT(data)], command);
		hotkeykey[GPOINTER_TO_INT(data)] = gtk_combo_box_get_active(combo);
	}
	case 2:
		break;
	}

	gtk_widget_destroy(dialog);
}

void show_dialog_about(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;
	GtkWidget *name;
	GtkWidget *version;
	GtkWidget *author;
	GtkWidget *www;

	show_thanklist();

	dialog = gtk_dialog_new_with_buttons("About", data, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, NULL);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	pixbuf = gdk_pixbuf_new_from_file("icon.ico", NULL);

	icon = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	pixbuf = NULL;
	name = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(name), "<big><b>Yixin Board</b></big>");
	version = gtk_label_new("Version " VERSION);
	author = gtk_label_new("(C)2009-2017 Kai Sun");
	www = gtk_label_new("www.aiexp.info");
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), icon, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), name, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), version, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), author, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), www, FALSE, FALSE, 10);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void new_game(GtkWidget *widget, gpointer data)
{
	piecenum = 0;
	isgameover = 0;
	memset(board, 0, sizeof(board));
	memset(forbid, 0, sizeof(forbid));
	memset(bestline, 0, sizeof(bestline));
	boardbestX = boardbestY = -1;
	memset(boardlose, 0, sizeof(boardlose));
	memset(boardpos, 0, sizeof(boardpos));
	refresh_board();
	if (isthinking)
		isneedomit++;
	isthinking = 0;
	clock_timer_change_status(2);
	isneedrestart = 1;
	update_textpos();

	if (widget != NULL)
	{
		refreshboardflag = 0;
		show_database();
	}
}

void new_game_resetclock(GtkWidget *widget, gpointer data)
{
	new_game(widget, data);
	clock_timer_init();

	if (specialrule == 4)
	{
		swap2done = 0;
	}
	if (specialrule == 4 && computerside == 1)
	{
		send_command("yxswap2step1\n");
	}
	if (specialrule == 3)
	{
		refreshboardflag2 = 0;
	}
	if (specialrule == 3 && computerside == 1)
	{
		send_command("yxsoosorvstep1\n");
	}
}

void set_rule()
{
	char command[80];
	sprintf(command, "INFO rule %d\n", inforule);
	send_command(command);
	isneedrestart = 1;
	show_forbid();
	show_database();
}
void change_rule(GtkWidget *widget, gpointer data)
{
	inforule = GPOINTER_TO_INT(data);
	if (inforule == 3)
	{
		inforule = 2;
		specialrule = 1;
	}
	else if (inforule == 4)
	{
		inforule = 0;
		specialrule = 2;
	}
	else if (inforule == 5)
	{
		inforule = 2;
		specialrule = 3;
	}
	else if (inforule == 6)
	{
		inforule = 1;
		specialrule = 4;
	}
	else
	{
		specialrule = 0;
	}
	set_rule();
}
void change_timeoutcheck(GtkWidget *widget, gpointer data)
{
	checktimeout ^= 1;
}
void change_side(GtkWidget *widget, gpointer data)
{
	computerside ^= GPOINTER_TO_INT(data);
	isneedrestart = 1;
}
void change_side_menu(int flag, GtkWidget *w)
{
	static GtkWidget *rec[2];
	switch (flag)
	{
	case 3:
		rec[0] = w;
		break;
	case 4:
		rec[1] = w;
		break;
	case -1:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[0]), FALSE);
		break;
	case -2:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[1]), FALSE);
		break;
	case 1:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[0]), TRUE);
		break;
	case 2:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[1]), TRUE);
		break;
	}
}
void view_analysis(GtkWidget *widget, gpointer data)
{
	showanalysis ^= 1;
	refresh_board();
}
void view_forbidden(GtkWidget *widget, gpointer data)
{
	showforbidden ^= 1;
	refresh_board();
}
void view_boardtext(GtkWidget *widget, gpointer data)
{
	showboardtext ^= 1;
	refresh_board();
}
void use_database(GtkWidget *widget, gpointer data)
{
	char command[80];
	usedatabase ^= 1;
	sprintf(command, "info usedatabase %d\n", usedatabase);
	send_command(command);
	if (usedatabase)
		show_database();
	else
	{
		refresh_board();
		gtk_window_set_title(GTK_WINDOW(windowmain), "Yixin");
	}
	if (usedatabase && showlog)
		gtk_widget_show(scrolledtextdbcomment);
	else
		gtk_widget_hide(scrolledtextdbcomment);
}
void set_database_readonly(GtkWidget *widget, gpointer data)
{
	char command[80];
	databasereadonly ^= 1;
	sprintf(command, "info database_readonly %d\n", databasereadonly);
	send_command(command);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textdbcomment), !databasereadonly);
}
void view_numeration(GtkWidget *widget, gpointer data)
{
	shownumber ^= 1;
	refresh_board();
}
void view_log(GtkWidget *widget, gpointer data)
{
	showlog ^= 1;
	if (showlog)
	{
		gtk_widget_show(posbar);
		if (usedatabase)
			gtk_widget_show(scrolledtextdbcomment);
		gtk_widget_show(scrolledtextlog);
		gtk_widget_show(scrolledtextcommand);
		gtk_widget_show(toolbar);
	}
	else
	{
		gtk_widget_hide(posbar);
		gtk_widget_hide(scrolledtextdbcomment);
		gtk_widget_hide(scrolledtextlog);
		gtk_widget_hide(scrolledtextcommand);
		gtk_widget_hide(toolbar);
	}
}
void view_clock()
{
	showclock ^= 1;
	if (showclock)
	{
		gtk_widget_show_all(windowclock);
		gtk_window_move(windowclock, 0, 0);
	}
	else
	{
		gtk_widget_hide(windowclock);
	}
}
gint windowclock_delete()
{
	return TRUE;
}
void change_language(GtkWidget *widget, gpointer data)
{
	if (language == GPOINTER_TO_INT(data))
		return;
	rlanguage = GPOINTER_TO_INT(data);
	respawn = 1;
	yixin_quit();
}
void change_piece(GtkWidget *widget, gpointer data)
{
	int i;
	int p = 0;
	if (widget != NULL)
		refreshboardflag = 0;
	switch (GPOINTER_TO_INT(data))
	{
	case 0:
		p = 0;
		break;
	case 1:
		p = piecenum - 1;
		break;
	case 2:
		p = piecenum + 1;
		break;
	case 3:
		p = MAX_SIZE * MAX_SIZE;
		break;
	}
	while (p > 0 && movepath[p - 1] == -1)
		p--;

	clear_board_tag();

	new_game(NULL, NULL);
	for (i = 0; i < p; i++)
		make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
	show_forbid();
	show_database();

	stop_thinking(widget, data);
	if (p == 0)
		update_textpos();
}

void stop_thinking(GtkWidget *widget, gpointer data)
{
	char command[80];
	sprintf(command, "yxstop\n");
	send_command(command);
}

void start_thinking(GtkWidget *widget, gpointer data)
{
	GdkEventButton event;
	GdkWindowEdge edge = GDK_WINDOW_EDGE_NORTH_WEST;
	if (isthinking)
		return;
	if (piecenum % 2 == 1 && (computerside & 1))
		change_side_menu(-1, NULL);
	if (piecenum % 2 == 0 && (computerside & 2))
		change_side_menu(-2, NULL);
	if (piecenum % 2 == 0 && computerside == 0)
		change_side_menu(1, NULL);
	if (piecenum % 2 == 1 && computerside == 0)
		change_side_menu(2, NULL);
	event.type = GDK_BUTTON_PRESS;
	event.button = 1;
	GtkAllocation allocation;
	gtk_widget_get_allocation(imageboard[0][0], &allocation);
	event.x = allocation.x;
	event.y = allocation.y;
	on_button_press_windowmain(widget, &event, edge);
	if (computerside == 1)
		change_side_menu(-1, NULL);
	if (computerside == 2)
		change_side_menu(-2, NULL);
}

void custom_function(char *command)
{
	int l = 0, r;
	while (command[l])
	{
		char t;
		if (command[l] == '\n' || command[l] == '\r')
		{
			l++;
			continue;
		}
		r = l + 1;
		while (command[r] && command[r] != '\n')
			r++;

		t = command[r + 1];
		command[r + 1] = 0;
		if (yixin_strnicmp(command + l, "sleep", 5) == 0)
		{
			int milliseconds = 0;
			sscanf(command + l + 5 + 1, "%d", &milliseconds);

			command[r + 1] = t;
			if (milliseconds > 1)
			{
				gchar *commandqueue = g_strdup(command + r);
				g_timeout_add(milliseconds, sleep_timeout, commandqueue);
				break;
			}
		}
		else
		{
			execute_command(command + l);
			command[r + 1] = t;
		}
		l = r;
	}
}

gint sleep_timeout(gpointer data)
{
	custom_function((char *)data);
	g_free(data);
	return FALSE;
}

void hotkey_function(GtkWidget *widget, gpointer data)
{
	custom_function(hotkeycommand[GPOINTER_TO_INT(data)]);
}

void toolbar_function(GtkWidget *widget, gpointer data)
{
	custom_function(toolbarcommand[GPOINTER_TO_INT(data)]);
}

void execute_command(gchar *command)
{
	int i;
	if (debuglog != NULL)
	{
		fprintf(debuglog, "EXECUTE_COMMAND [%s,%s,%s,%s]: %s\n",
				gtk_label_get_text(clocklabel[0]),
				gtk_label_get_text(clocklabel[1]),
				gtk_label_get_text(clocklabel[2]),
				gtk_label_get_text(clocklabel[3]),
				command);
		fflush(debuglog);
	}
	if (yixin_strnicmp(command, "command on", 10) == 0)
	{
		commandmodel = 1;
	}
	else if (yixin_strnicmp(command, "command off", 11) == 0)
	{
		commandmodel = 0;
	}
	else if (commandmodel == 1)
	{
		printf_log(command);
		send_command(command);
	}
	else if (yixin_strnicmp(command, "echo", 4) == 0)
	{
		print_log(command + 5);
	}
	else if (yixin_strnicmp(command, "help", 4) == 0)
	{
		if (language)
		{
			printf_log(clanguage[50]);
		}
		else
		{
			printf_log("Command Lists:");
		}
		printf_log("\n");
		printf_log(" help\n");
		printf_log(" clear\n");
		printf_log(" rotate [90,180,270]\n");
		printf_log(" flip [/,\\,-,|]\n");
		printf_log(" move [^,v,<,>]\n");
		printf_log(" getpos\n");
		printf_log(" putpos\n");
		printf_log("   %s: putpos f11h7g10h6i10h5j11h8h9h4\n", language ? clanguage[51] : "Example");
		printf_log(" getposclipboard (getpos from clipboard text)\n");
		printf_log(" putposclipboard (putpos to clipboard)\n");
		printf_log(" block\n");
		printf_log("   %s: block h8\n", language ? clanguage[51] : "Example");
		printf_log(" block undo\n");
		printf_log("   %s: block undo h8\n", language ? clanguage[51] : "Example");
		printf_log(" block reset\n");
		printf_log(" block compare\n");
		printf_log("   %s: block compare h8i8j7\n", language ? clanguage[51] : "Example");
		printf_log(" block autoreset [on,off]\n");
		printf_log(" blockpath\n");
		printf_log("   %s: blockpath h8h7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath undo\n");
		printf_log("   %s: blockpath undo h8h7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath reset\n");
		printf_log(" blockpath except\n");
		printf_log("   %s: blockpath except h8i8j7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath autoreset [on,off]\n");
		printf_log(" hash clear\n");
		printf_log(" hash autoclear [on,off]\n");
		printf_log(" hash dump [filename]\n");
		printf_log(" hash load [filename]\n");
		printf_log(" bestline\n");
		printf_log(" balance<1,2>\n");
		printf_log("   %s: balance1\n", language ? clanguage[52] : "Example 1");
		printf_log("   %s: balance1 100\n", language ? clanguage[53] : "Example 2");
		printf_log("   %s: balance2\n", language ? clanguage[54] : "Example 3");
		printf_log("   %s: balance2 100\n", language ? clanguage[55] : "Example 4");
		printf_log(" nbest [2,3,...]\n");
		printf_log(" search from [depth]\n");
		printf_log(" toolbar edit [1,2,...]\n");
		printf_log(" toolbar add\n");
		printf_log(" toolbar remove\n");
		printf_log(" key edit [1,2,...]\n");
		printf_log(" key add\n");
		printf_log(" key remove\n");
		printf_log(" key list\n");
		printf_log(" thinking start\n");
		printf_log(" thinking stop\n");
		printf_log(" thinking toggle\n");
		printf_log(" draw\n");
		printf_log(" resign\n");
		printf_log(" undo one\n");
		printf_log(" undo all\n");
		printf_log(" redo one\n");
		printf_log(" redo all\n");
		printf_log(" searchdefend\n");
		printf_log(" dbval\n");
		printf_log(" dbdel one\n");
		printf_log(" dbdel all\n");
		printf_log(" dbdel all [w,l,wl,nonwl,wlnostep] (recursive)\n");
		printf_log(" dbdel all wlinstep [numberofstep] (recursive)\n");
		printf_log(" dbsave\n");
		printf_log(" dbset [filename]\n");
		printf_log(" dbmerge [filename]\n");
		printf_log(" dbsplit [filename]\n");
		printf_log(" hash usage\n");
		printf_log(" command [on,off]\n");
		printf_log(" dbeditcomment [comment...]\n");
		printf_log(" dbeditlabel [coord] [label]\n");
		printf_log(" dbeditlabelpoint [label]\n");
		printf_log(" dbedittag [tag]\n");
		printf_log(" dbeditval [value]\n");
		printf_log(" dbeditdep [depth]\n");
		printf_log(" dbsetbestmove\n");
		printf_log(" dbclearbestmove\n");
		printf_log(" dbtotxt all [filename]\n");
		printf_log(" dbtotxt [filename]\n");
		printf_log(" libtodb [filename]\n");
		printf_log(" forbid\n");
		printf_log(" forbid undo\n");
		printf_log(" echo [a line of text]\n");
		printf_log(" sleep [millisecond]\n");
		printf_log("   %s: sleep 5000\n", language ? clanguage[51] : "Example");
		printf_log("\n");
	}
	else if (yixin_strnicmp(command, "clear", 5) == 0)
	{
		GtkTextIter _start, _end;
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextlog), &_start, &_end);
		gtk_text_buffer_delete(buffertextlog, &_start, &_end);
	}
	else if (yixin_strnicmp(command, "rotate", 6) == 0)
	{
		int p = piecenum;
		int j, k = 1;
		if (boardsizew != boardsizeh)
		{
			printf_log(language == 0 ? "Sorry, board cannot be rotated when height<>width." : clanguage[56]);
			printf_log("\n");
		}
		else
		{
			if (strlen(command) >= 8)
			{
				if (command[7] == '9')
					k = 1;
				else if (command[7] == '1')
					k = 2;
				else if (command[7] == '2')
					k = 3;
			}
			refreshboardflag = 0;
			for (j = 0; j < k; j++)
			{
				for (i = 0; i < p; i++)
				{
					int _x, _y, x, y;
					_y = movepath[i] / boardsizew;
					_x = movepath[i] % boardsizew;
					y = _x;
					x = boardsizeh - 1 - _y;
					movepath[i] = y * boardsizew + x;
				}
			}
			new_game(NULL, NULL);
			for (i = 0; i < p; i++)
				make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
			show_forbid();
			show_database();
		}
	}
	else if (yixin_strnicmp(command, "flip", 4) == 0)
	{
		int p = piecenum;
		int k = 0;
		if (strlen(command) >= 6)
		{
			if (command[5] == '-')
				k = 0;
			else if (command[5] == '|')
				k = 1;
			else if (command[5] == '/')
				k = 2;
			else if (command[5] == '\\')
				k = 3;
		}
		if ((k == 2 || k == 3) && (boardsizew != boardsizeh))
		{
			printf_log(language == 0 ? "Sorry, board cannot be flipped with / or \\ when height<>width." : clanguage[57]);
			printf_log("\n");
		}
		else
		{
			refreshboardflag = 0;
			for (i = 0; i < p; i++)
			{
				int _x, _y, x, y;
				_y = movepath[i] / boardsizew;
				_x = movepath[i] % boardsizew;
				switch (k)
				{
				case 0:
					y = boardsizeh - 1 - _y;
					x = _x;
					break;
				case 1:
					y = _y;
					x = boardsizew - 1 - _x;
					break;
				case 2:
					y = boardsizeh - 1 - _y;
					x = boardsizew - 1 - _x;
					break;
				case 3:
					y = _x;
					x = _y;
					break;
				}
				movepath[i] = y * boardsizew + x;
			}
			new_game(NULL, NULL);
			for (i = 0; i < p; i++)
				make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
			show_forbid();
			show_database();
		}
	}
	else if (yixin_strnicmp(command, "move", 4) == 0)
	{
		int p = piecenum;
		int k = 0;
		int f = 1;
		if (strlen(command) >= 6)
		{
			if (command[5] == '^')
				k = 0;
			else if (command[5] == 'v')
				k = 1;
			else if (command[5] == '<')
				k = 2;
			else if (command[5] == '>')
				k = 3;
		}
		refreshboardflag = 0;
		for (i = 0; i < p; i++)
		{
			int _x, _y, x, y;
			_y = movepath[i] / boardsizew;
			_x = movepath[i] % boardsizew;
			switch (k)
			{
			case 0:
				y = _y - 1;
				x = _x;
				break;
			case 1:
				y = _y + 1;
				x = _x;
				break;
			case 2:
				y = _y;
				x = _x - 1;
				break;
			case 3:
				y = _y;
				x = _x + 1;
				break;
			}
			if (x < 0 || x > boardsizew - 1 || y < 0 || y > boardsizeh - 1)
				f = 0;
		}
		if (f)
		{
			for (i = 0; i < p; i++)
			{
				int _x, _y, x, y;
				_y = movepath[i] / boardsizew;
				_x = movepath[i] % boardsizew;
				switch (k)
				{
				case 0:
					y = _y - 1;
					x = _x;
					break;
				case 1:
					y = _y + 1;
					x = _x;
					break;
				case 2:
					y = _y;
					x = _x - 1;
					break;
				case 3:
					y = _y;
					x = _x + 1;
					break;
				}
				movepath[i] = y * boardsizew + x;
			}
		}
		new_game(NULL, NULL);
		for (i = 0; i < p; i++)
			make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
		show_forbid();
		show_database();
	}
	else if (yixin_strnicmp(command, "putposclipboard", 15) == 0)
	{
		GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
		gchar *posstring = gtk_clipboard_wait_for_text(clipboard);
		if (posstring != NULL)
		{
			gchar *_command = g_strdup_printf("putpos %s", posstring);
			execute_command(_command);
			g_free(_command);
			g_free(posstring);
		}
	}
	else if (yixin_strnicmp(command, "getposclipboard", 15) == 0)
	{
		char posstring[1024];
		int offset, accumulatedOffset = 0;
		for (i = 0; i < piecenum; i++)
		{
			sprintf(posstring + accumulatedOffset, "%c%d%n", movepath[i] % boardsizew + 'a',
					boardsizeh - 1 - movepath[i] / boardsizew + 1, &offset);
			accumulatedOffset += offset;
		}
		GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text(clipboard, posstring, -1);
	}
	else if (yixin_strnicmp(command, "putpos", 6) == 0)
	{
		new_game(NULL, NULL);
		i = 6;
		while (command[i] == '\t' || command[i] == ' ')
			i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z')
				command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z')
				break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			y = boardsizeh - 1 - y;
			if (board[y][x] != 0)
				break;
			make_move(y, x);
		}
		show_forbid();
		show_database();
	}
	else if (yixin_strnicmp(command, "getpos", 6) == 0)
	{
		for (i = 0; i < piecenum; i++)
		{
			printf_log("%c%d", movepath[i] % boardsizew + 'a', boardsizeh - 1 - movepath[i] / boardsizew + 1);
		}
		printf_log("\n");
	}
	else if (yixin_strnicmp(command, "blockpath reset", 15) == 0)
	{
		send_command("yxblockpathreset\n");
	}
	else if (yixin_strnicmp(command, "blockpath autoreset", 19) == 0)
	{
		if (strlen(command) >= 22)
		{
			if (command[21] == 'n' || command[21] == 'N')
			{
				blockpathautoreset = 1;
			}
			else
			{
				blockpathautoreset = 0;
			}
		}
	}
	else if (yixin_strnicmp(command, "blockpath undo", 14) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE * MAX_SIZE], yl[MAX_SIZE * MAX_SIZE];
		int len = 0;

		i = 14;
		while (command[i] == '\t' || command[i] == ' ')
			i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z')
				command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z')
				break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			send_command("yxblockpathundo\n");
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			for (i = 0; i < len; i++)
			{
				sprintf(_command, "%d,%d\n", xl[i], yl[i]);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (yixin_strnicmp(command, "blockpath except", 16) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE * MAX_SIZE], yl[MAX_SIZE * MAX_SIZE];
		int len = 0;

		i = 16;
		while (command[i] == '\t' || command[i] == ' ')
			i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z')
				command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z')
				break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			int j, k;
			for (j = 0; j < MAX_SIZE; j++)
			{
				for (k = 0; k < MAX_SIZE; k++)
				{
					if (xl[len - 1] == j && yl[len - 1] == k)
						continue;
					send_command("yxblockpath\n");
					for (i = 0; i < piecenum; i++)
					{
						sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
						send_command(_command);
					}
					for (i = 0; i < len - 1; i++)
					{
						sprintf(_command, "%d,%d\n", xl[i], yl[i]);
						send_command(_command);
					}
					sprintf(_command, "%d,%d\n", j, k);
					send_command(_command);
					send_command("done\n");
				}
			}
		}
	}
	else if (yixin_strnicmp(command, "blockpath list", 14) == 0)
	{
		; // TODO
	}
	else if (yixin_strnicmp(command, "blockpath", 9) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE * MAX_SIZE], yl[MAX_SIZE * MAX_SIZE];
		int len = 0;

		i = 9;
		while (command[i] == '\t' || command[i] == ' ')
			i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z')
				command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z')
				break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			send_command("yxblockpath\n");
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			for (i = 0; i < len; i++)
			{
				sprintf(_command, "%d,%d\n", xl[i], yl[i]);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (yixin_strnicmp(command, "block reset", 11) == 0)
	{
		send_command("yxblockreset\n");
		memset(boardblock, 0, sizeof(boardblock));
		refresh_board();
	}
	else if (yixin_strnicmp(command, "block autoreset", 15) == 0)
	{
		if (strlen(command) >= 18)
		{
			if (command[17] == 'n' || command[17] == 'N')
			{
				blockautoreset = 1;
			}
			else
			{
				blockautoreset = 0;
			}
		}
	}
	else if (yixin_strnicmp(command, "block undo", 10) == 0)
	{
		gchar _command[80];
		do
		{
			int x, y;
			if (command[11] >= 'a' && command[11] <= 'z')
				command[11] = command[11] - 'a' + 'A';
			if (command[11] < 'A' || command[11] > 'Z')
				break;
			x = command[11] - 'A';
			y = command[12] - '0';
			if (command[13] >= '0' && command[13] <= '9')
			{
				y = y * 10 + command[13] - '0';
			}
			y--;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			send_command("yxblockundo\n");
			sprintf(_command, "%d,%d\n", boardsizeh - 1 - y, x);
			send_command(_command);
			send_command("done\n");

			boardblock[boardsizeh - 1 - y][x] = 0;
			refresh_board();
		} while (0);
	}
	else if (yixin_strnicmp(command, "block compare", 13) == 0)
	{
		gchar _command[80];
		int j;
		send_command("yxblockreset\n");
		memset(boardblock, 0, sizeof(boardblock));
		for (i = 0; i < boardsizeh; i++)
		{
			for (j = 0; j < boardsizew; j++)
			{
				if (board[i][j] == 0)
				{
					boardblock[i][j] = 1;
				}
			}
		}
		i = 13;
		while (command[i] == '\t' || command[i] == ' ')
			i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z')
				command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z')
				break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			boardblock[boardsizeh - 1 - y][x] = 0;
		}
		for (i = 0; i < boardsizeh; i++)
		{
			for (j = 0; j < boardsizew; j++)
			{
				if (boardblock[i][j] == 1)
				{
					send_command("yxblock\n");
					sprintf(_command, "%d,%d\n", i, j);
					send_command(_command);
					send_command("done\n");
				}
			}
		}
		refresh_board();
	}
	else if (yixin_strnicmp(command, "block", 5) == 0)
	{
		gchar _command[80];
		i = 0;
		do
		{
			int x, y;
			if (command[6 + i] >= 'a' && command[6 + i] <= 'z')
				command[6 + i] = command[6 + i] - 'a' + 'A';
			if (command[6 + i] < 'A' || command[6 + i] > 'Z')
				break;
			x = command[6 + i] - 'A';
			y = command[7 + i] - '0';
			if (command[8 + i] >= '0' && command[8 + i] <= '9')
			{
				y = y * 10 + command[8 + i] - '0';
				i++;
			}
			i += 2;

			y--;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			send_command("yxblock\n");
			sprintf(_command, "%d,%d\n", boardsizeh - 1 - y, x);
			send_command(_command);
			send_command("done\n");

			boardblock[boardsizeh - 1 - y][x] = 1;
		} while (isalpha(command[6 + i]));
		refresh_board();
	}
	else if (yixin_strnicmp(command, "forbid undo", 11) == 0)
	{
		gchar _command[80];
		do
		{
			int x, y, s;
			s = command[12] - '0';
			if (command[14] >= 'a' && command[14] <= 'z')
				command[14] = command[14] - 'a' + 'A';
			if (command[14] < 'A' || command[14] > 'Z')
				break;
			x = command[14] - 'A';
			y = command[15] - '0';
			if (command[16] >= '0' && command[16] <= '9')
			{
				y = y * 10 + command[16] - '0';
			}
			y--;
			if (s < 0 || s > 1 || x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			sprintf(_command, "yxforbid del %d %d %d\n", boardsizeh - 1 - y, x, s);
			send_command(_command);
		} while (0);
	}
	else if (yixin_strnicmp(command, "forbid", 6) == 0)
	{
		gchar _command[80];
		do
		{
			int x, y, s;
			s = command[7] - '0';
			if (command[9] >= 'a' && command[9] <= 'z')
				command[9] = command[9] - 'a' + 'A';
			if (command[9] < 'A' || command[9] > 'Z')
				break;
			x = command[9] - 'A';
			y = command[10] - '0';
			if (command[11] >= '0' && command[11] <= '9')
			{
				y = y * 10 + command[11] - '0';
			}
			y--;
			if (s < 0 || s > 1 || x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh)
				break;
			sprintf(_command, "yxforbid add %d %d %d\n", boardsizeh - 1 - y, x, s);
			send_command(_command);
		} while (0);
	}
	else if (yixin_strnicmp(command, "hash autoclear", 14) == 0)
	{
		if (strlen(command) >= 17)
		{
			if (command[16] == 'n' || command[16] == 'N')
			{
				hashautoclear = 1;
			}
			else
			{
				hashautoclear = 0;
			}
		}
	}
	else if (yixin_strnicmp(command, "hash clear", 10) == 0)
	{
		send_command("yxhashclear\n");
	}
	else if (yixin_strnicmp(command, "hash dump", 9) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 9 + 1);
		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxhashdump\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "hash load", 9) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 9 + 1);
		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxhashload\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "hash usage", 10) == 0)
	{
		send_command("yxshowhashusage\n");
	}
	else if (yixin_strnicmp(command, "search from", 11) == 0)
	{
		gchar _command[80];
		int depth = 1;
		sscanf(command + 11 + 1, "%d", &depth);
		sprintf(_command, "info start_depth %d\n", depth);
		send_command(_command);
	}
	else if (yixin_strnicmp(command, "key list", 8) == 0)
	{
		for (i = 0; i < hotkeynum; i++)
		{
			printf_log("%d. %s\n", i + 1, hotkeynamelist[hotkeykey[i]]);
		}
		printf_log("\n");
	}
	else if (yixin_strnicmp(command, "key edit", 8) == 0)
	{
		int n;
		if (sscanf(command + 8 + 1, "%d", &n) != EOF)
		{
			show_dialog_custom_hotkey(NULL, GINT_TO_POINTER(n - 1));
		}
	}
	else if (yixin_strnicmp(command, "key add", 7) == 0)
	{
		if (hotkeynum < MAX_HOTKEY_ITEM)
		{
			hotkeynum++;
			strcpy(hotkeycommand[hotkeynum - 1], "\n");
			hotkeykey[hotkeynum - 1] = 0;
			show_dialog_custom_hotkey(NULL, GINT_TO_POINTER(hotkeynum - 1));
		}
	}
	else if (yixin_strnicmp(command, "key remove", 10) == 0)
	{
		if (hotkeynum > 0)
		{
			hotkeynum--;
		}
	}
	else if (yixin_strnicmp(command, "toolbar edit", 12) == 0)
	{
		int n;
		if (sscanf(command + 12 + 1, "%d", &n) != EOF)
		{
			show_dialog_custom_toolbar(NULL, GINT_TO_POINTER(n - 1));
		}
	}
	else if (yixin_strnicmp(command, "toolbar add", 11) == 0)
	{
		if (toolbarnum < MAX_TOOLBAR_ITEM)
		{
			toolbarnum++;
			toolbarlng[toolbarnum - 1] = 92;
			toolbaricon[toolbarnum - 1] = strdup("dialog-question");
			strcpy(toolbarcommand[toolbarnum - 1], "\n");
			show_dialog_custom_toolbar(NULL, GINT_TO_POINTER(toolbarnum - 1));
		}
	}
	else if (yixin_strnicmp(command, "toolbar remove", 14) == 0)
	{
		if (toolbarnum > 0)
		{
			toolbarnum--;
			respawn = 1;
			yixin_quit();
		}
	}
	else if (yixin_strnicmp(command, "thinking start", 14) == 0)
	{
		start_thinking(windowmain, NULL);
	}
	else if (yixin_strnicmp(command, "thinking stop", 13) == 0)
	{
		stop_thinking(windowmain, NULL);
	}
	else if (yixin_strnicmp(command, "thinking toggle", 13) == 0)
	{
		if (isthinking)
			stop_thinking(windowmain, NULL);
		else
			start_thinking(windowmain, NULL);
	}
	else if (yixin_strnicmp(command, "undo all", 8) == 0)
	{
		change_piece(windowmain, (gpointer)0);
	}
	else if (yixin_strnicmp(command, "undo one", 8) == 0)
	{
		change_piece(windowmain, (gpointer)1);
	}
	else if (yixin_strnicmp(command, "redo one", 8) == 0)
	{
		change_piece(windowmain, (gpointer)2);
	}
	else if (yixin_strnicmp(command, "redo all", 8) == 0)
	{
		change_piece(windowmain, (gpointer)3);
	}
	else if (yixin_strnicmp(command, "bestline", 8) == 0)
	{
		printf_log("BESTLINE: %s ", bestline);
		printf_log("VAL: %d\n", bestval);
	}
	else if (yixin_strnicmp(command, "balance", 7) == 0 && (command[7] == '1' || command[7] == '2'))
	{
		gchar _command[80];
		int s;
		int t;
		t = command[7] - '0';
		if (sscanf(command + 8, "%d", &s) != 1)
			s = 0;
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
		sprintf(_command, "yxbalance%s %d\n", t == 1 ? "one" : "two", s);
		send_command(_command);
	}
	else if (yixin_strnicmp(command, "nbest", 5) == 0)
	{
		gchar _command[80];
		int s;
		if (sscanf(command + 5, "%d", &s) != 1)
			s = 2;
		sprintf(_command, "info nbestsym %d\n", nbestsym);
		send_command(_command);
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
		sprintf(_command, "yxnbest %d\n", s);
		send_command(_command);
	}
	else if (yixin_strnicmp(command, "searchdefend", 9) == 0)
	{
		gchar _command[80];
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
		sprintf(_command, "yxsearchdefend\n");
		send_command(_command);
	}
	else if (yixin_strnicmp(command, "dbsetbestmove", 13) == 0)
	{
		gchar _command[80];
		send_command("yxsetbestmovedatabase\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbclearbestmove", 15) == 0)
	{
		gchar _command[80];
		send_command("yxclearbestmovedatabase\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbeditcomment", 13) == 0)
	{
		gchar _command[4096];
		if (strlen(command) >= 15)
		{
			const char cmdhead[] = "yxedittextdatabase \"";
			strcpy(_command, cmdhead);
			int i = sizeof(cmdhead) - 1;
			char *comment = __invT(command + 13 + 1);
			for (int j = 0; comment[j] && i < sizeof(_command) - 3; j++)
			{
				if (comment[j] == '"')
				{
					_command[i++] = '\\';
					_command[i++] = '"';
				}
				else if (comment[j] == '\\')
				{
					_command[i++] = '\\';
					_command[i++] = '\\';
				}
				else
					_command[i++] = comment[j];
			}
			_command[i++] = '"';
			_command[i] = 0;
			g_free(comment);

			send_command(_command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");

			execute_command("dbtext");
		}
	}
	else if (yixin_strnicmp(command, "dbeditlabelpoint", 16) == 0)
	{
		gchar _command[80];
		int len = strlen(command);
		if (len >= 14 && piecenum >= 1)
		{
			char boardtext[64];
			if (len >= 64)
				command[64] = '\0'; // only allow up to 64 characters
			memset(boardtext, 0, sizeof(boardtext));
			sscanf(command + 16 + 1, "%s", boardtext);

			int x = movepath[piecenum - 1] / boardsizew;
			int y = movepath[piecenum - 1] % boardsizew;
			char *text = __invT(boardtext);
			sprintf(_command, "yxeditlabeldatabase %d,%d %s\n", x, y, text);
			g_free(text);

			send_command(_command);
			for (i = 0; i < piecenum - 1; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");

			show_database();
		}
	}
	else if (yixin_strnicmp(command, "dbeditlabel", 11) == 0)
	{
		gchar _command[80];
		int len = strlen(command);
		if (len >= 14)
		{
			char boardtext[64];
			char xcoord;
			int ycoord;
			if (len >= 64)
				command[64] = '\0'; // only allow up to 64 characters
			memset(boardtext, 0, sizeof(boardtext));
			sscanf(command + 11 + 1, "%c %d %s", &xcoord, &ycoord, boardtext);
			if (xcoord >= 'a' && xcoord <= 'z')
				xcoord = xcoord - 'a' + 'A'; // convert to upper case

			int x = boardsizeh - ycoord, y = xcoord - 'A';
			char *text = __invT(boardtext);
			sprintf(_command, "yxeditlabeldatabase %d,%d %s\n", x, y, text);
			g_free(text);

			send_command(_command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");

			show_database();
		}
	}
	else if (yixin_strnicmp(command, "dbedittag", 9) == 0)
	{
		gchar _command[80];
		if (strlen(command) >= 10)
		{
			sprintf(_command, "yxedittvddatabase 1 %hhd 0 0\n", (command[10] == 0 || command[10] == '\n' || command[10] == '\r') ? -1 : command[10]);
			send_command(_command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (yixin_strnicmp(command, "dbeditval", 9) == 0)
	{
		gchar _command[80];
		if (strlen(command) >= 10)
		{
			short val = 0;
			sscanf(command + 10, "%hd", &val);
			sprintf(_command, "yxedittvddatabase 2 -1 %d 0\n", val);
			send_command(_command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (yixin_strnicmp(command, "dbeditdep", 9) == 0)
	{
		gchar _command[80];
		if (strlen(command) >= 10)
		{
			short dep = 0;
			sscanf(command + 10, "%hd", &dep);
			sprintf(_command, "yxedittvddatabase 4 -1 0 %d\n", dep);
			send_command(_command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (yixin_strnicmp(command, "dbtopos", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 7 + 1);

		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxdbtopos\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "dbtotxt all", 11) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 11 + 1);

		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxdbtotxtall\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "dbtotxt", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 7 + 1);

		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxdbtotxt\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "txttodb", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 7 + 1);

		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxtxttodb\n");
			send_command(_command);
		}
	}
	else if (yixin_strnicmp(command, "libtodb", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 7 + 1);

		if (command[7] == 0 || command[7] == '\n' || command[7] == '\r')
		{
			const char *libfilename = show_dialog_libselect(NULL, windowmain, 0);
			if (libfilename)
			{
				send_command("yxlibtodb\n");
				send_command(libfilename);
				show_database();
			}
		}
		else
		{
			i = strlen(_command);
			while (i > 0)
			{
				if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
				{
					_command[i - 1] = 0;
					i--;
				}
				else
					break;
			}
			_command[i] = '\n';
			_command[i + 1] = 0;
			if (i > 0)
			{
				send_command("yxlibtodb\n");
				send_command(_command);
				show_database();
			}
		}
	}
	else if (yixin_strnicmp(command, "dbsave", 6) == 0)
	{
		send_command("yxsavedatabase\n");
	}
	else if (yixin_strnicmp(command, "dbset", 5) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 5 + 1);
		if (command[5] == 0 || command[5] == '\n' || command[5] == '\r' || command[5] == 'o')
		{
			const char *dbfilename = show_dialog_dbselect(NULL, windowmain, command[5] == 'o' ? 0 : 1);
			if (dbfilename)
			{
				send_command("yxsetdatabase\n");
				send_command(dbfilename);
				show_database();
			}
		}
		else
		{
			i = strlen(_command);
			while (i > 0)
			{
				if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
				{
					_command[i - 1] = 0;
					i--;
				}
				else
					break;
			}
			_command[i] = '\n';
			_command[i + 1] = 0;
			if (i > 0)
			{
				send_command("yxsetdatabase\n");
				send_command(_command);
				show_database();
			}
		}
	}
	else if (yixin_strnicmp(command, "draw", 4) == 0)
	{
		send_command("yxdraw\n");
	}
	else if (yixin_strnicmp(command, "resign", 6) == 0)
	{
		send_command("yxresign\n");
	}
	else if (yixin_strnicmp(command, "dbcheck", 5) == 0)
	{
		send_command("yxdbcheck\n");
	}
	else if (yixin_strnicmp(command, "dbfix", 5) == 0)
	{
		send_command("yxdbfix\n");
	}
	else if (yixin_strnicmp(command, "dbval", 5) == 0)
	{
		gchar _command[80];
		send_command("yxquerydatabaseone\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbtext", 6) == 0)
	{
		gchar _command[80];
		send_command("yxquerydatabasetext\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel one", 9) == 0)
	{
		gchar _command[80];
		send_command("yxdeletedatabaseone\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all nonwl", 15) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command(yixin_strnicmp(command, "dbdel all nonwl recursive", 25) == 0 ? "yxdeletedatabaseall nonwlrecursive\n" : "yxdeletedatabaseall nonwl\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all wlnostep", 18) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command(yixin_strnicmp(command, "dbdel all wlnostep recursive", 28) == 0 ? "yxdeletedatabaseall wlnosteprecursive\n" : "yxdeletedatabaseall wlnostep\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all wlinstep", 18) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		int recursive = yixin_strnicmp(command, "dbdel all wlinstep recursive", 28) == 0;
		int matestep = 1;
		sscanf(command + (recursive ? 29 : 19), "%d", &matestep);
		sprintf(_command,
				(recursive ? "yxdeletedatabaseall wlinsteprecursive %d\n" : "yxdeletedatabaseall wlinstep %d\n"),
				matestep);
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all wl", 12) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command(yixin_strnicmp(command, "dbdel all wl recursive", 22) == 0 ? "yxdeletedatabaseall wlrecursive\n" : "yxdeletedatabaseall wl\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all w", 11) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command(yixin_strnicmp(command, "dbdel all w recursive", 21) == 0 ? "yxdeletedatabaseall wrecursive\n" : "yxdeletedatabaseall w\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all l", 11) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command(yixin_strnicmp(command, "dbdel all l recursive", 21) == 0 ? "yxdeletedatabaseall lrecursive\n" : "yxdeletedatabaseall l\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbdel all", 9) == 0)
	{
		if (!show_dbdelall_query())
			return;
		gchar _command[80];
		send_command("yxdeletedatabaseall\n");
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew);
			send_command(_command);
		}
		send_command("done\n");
	}
	else if (yixin_strnicmp(command, "dbmerge", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 7 + 1);
		if (command[7] == 0 || command[7] == '\n' || command[7] == '\r')
		{
			const char *dbfilename = show_dialog_dbselect(NULL, windowmain, 0);
			if (dbfilename)
			{
				send_command("yxdbmerge\n");
				send_command(dbfilename);
				show_database();
			}
		}
		else
		{
			i = strlen(_command);
			while (i > 0)
			{
				if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
				{
					_command[i - 1] = 0;
					i--;
				}
				else
					break;
			}
			_command[i] = '\n';
			_command[i + 1] = 0;
			if (i > 0)
			{
				send_command("yxdbmerge\n");
				send_command(_command);
				show_database();
			}
		}
	}
	else if (yixin_strnicmp(command, "dbsplit", 7) == 0)
	{
		gchar _command[80];
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);

		sprintf(_command, "%s", command + 7 + 1);
		if (command[7] == 0 || command[7] == '\n' || command[7] == '\r')
		{
			const char *dbfilename = show_dialog_dbselect(NULL, windowmain, 1);
			if (dbfilename)
			{
				send_command("yxdbsplit\n");
				send_command(dbfilename);
			}
		}
		else
		{
			i = strlen(_command);
			while (i > 0)
			{
				if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
				{
					_command[i - 1] = 0;
					i--;
				}
				else
					break;
			}
			_command[i] = '\n';
			_command[i + 1] = 0;
			if (i > 0)
			{
				send_command("yxdbsplit\n");
				send_command(_command);
			}
		}
	}
	else if (yixin_strnicmp(command, "makebook", 8) == 0)
	{
		; // TODO
	}
	else if (yixin_strnicmp(command, "print features", 14) == 0)
	{
		send_command("yxprintfeature\n");
	}
	else if (yixin_strnicmp(command, "send board", 10) == 0)
	{
		gchar _command[80];
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i < piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
					movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
	}
	else if (yixin_strnicmp(command, "dbrefresh", 9) == 0)
	{
		char command[80];
		sprintf(command, "info usedatabase %d\n", usedatabase);
		send_command(command);
		sprintf(command, "info database_readonly %d\n", databasereadonly);
		send_command(command);
		refresh_board();
	}
	else
	{
		if (language)
		{
			printf_log(clanguage[58]);
		}
		else
		{
			printf_log("To get help, type help and press Enter");
		}
		printf_log("\n");
	}
}

gboolean key_command(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GtkTextIter start, end;
	gchar *command;

	if (event->keyval == GDK_KEY_Return && !(event->state & GDK_CONTROL_MASK))
	{
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextcommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffertextcommand), &start, &end, FALSE);

		custom_function(command);

		gtk_text_buffer_delete(buffertextcommand, &start, &end);
		g_free(command);
	}
	return FALSE;
}

void dbcomment_changed(GtkWidget *widget, gpointer data)
{
	GtkTextIter start, end;
	gchar *comment, *comment_utf8;

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextdbcomment), &start, &end);
	comment_utf8 = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffertextdbcomment), &start, &end, FALSE);
	comment = __invT(comment_utf8);
	g_free(comment_utf8);

	static gchar _command[65536];
	const char cmdhead[] = "yxedittextdatabase \"";
	strcpy(_command, cmdhead);

	int i = sizeof(cmdhead) - 1;
	for (int j = 0; comment[j] && i < sizeof(_command) - 3; j++)
	{
		if (comment[j] == '"')
		{
			_command[i++] = '\\';
			_command[i++] = '"';
		}
		else if (comment[j] == '\\')
		{
			_command[i++] = '\\';
			_command[i++] = '\\';
		}
		else
			_command[i++] = comment[j];
	}
	_command[i++] = '"';
	_command[i] = 0;
	g_free(comment);

	send_command(_command);
	for (i = 0; i < piecenum; i++)
	{
		sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
		send_command(_command);
	}
	send_command("done\n");
}

void textpos_changed(GtkWidget *widget, gpointer data)
{
	if (isthinking)
		return;

	GtkTextIter start, end;
	gchar *posstr, *command;
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextpos), &start, &end);
	posstr = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffertextpos), &start, &end, FALSE);

	int j = 0;
	for (int i = 0; posstr[i]; i++)
	{
		if (posstr[i] != ' ')
			posstr[j++] = posstr[i];
	}
	posstr[j] = '\0';

	int valid = 1;
	for (int i = 0; posstr[i];)
	{
		if (!((posstr[i] >= 'a' && posstr[i] <= 'z') || (posstr[i] >= 'A' && posstr[i] <= 'Z')))
		{
			valid = 0;
			break;
		}
		i++;
		if (posstr[i] < '0' || posstr[i] > '9')
		{
			valid = 0;
			break;
		}
		i++;
		if (posstr[i] >= '0' && posstr[i] <= '9')
			i++;
	}

	if (valid)
	{
		command = g_strdup_printf("putpos %s", posstr);
		execute_command(command);
		g_free(command);
	}
	g_free(posstr);
}

void textpos_button_clicked(GtkWidget *button, gpointer data)
{
	if (GPOINTER_TO_INT(data) == 1)
		execute_command("getposclipboard");
	else if (GPOINTER_TO_INT(data) == 2 && !isthinking)
		execute_command("putposclipboard");
}

gboolean on_windows_close(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	yixin_quit();
	return usedatabase != 0;
}

void save_setting()
{
	FILE *out;
	int i;
	char s[80];

	if ((out = fopen("settings.txt", "w")) != NULL)
	{
		fprintf(out, "%d %d\t;board size (10 ~ %d)\n", rboardsizeh, rboardsizew, MAX_SIZE);
		fprintf(out, "%d\t;language (0: English, 1,2,...: custom)\n", rlanguage);
		fprintf(out, "%d\t;rule (0: freestyle, 1: standard, 2: free renju, 3: swap after 1st move, 5: soosorv, 6: swap-2)\n",
				specialrule == 4 ? 6 : (specialrule == 3 ? 5 : (specialrule == 2 ? 3 : inforule)));
		fprintf(out, "%d\t;computer play black (0: no, 1: yes)\n", computerside & 1);
		fprintf(out, "%d\t;computer play white (0: no, 1: yes)\n", computerside >> 1);
		fprintf(out, "%d\t;level (0: unlimited time 1: custom level 2-: predefined level)\n", levelchoice);
		fprintf(out, "%lld\t;time limit (turn)\n", timeoutturn / 1000);
		fprintf(out, "%lld\t;time limit (match)\n", timeoutmatch / 1000);
		fprintf(out, "%d\t;max depth\n", maxdepth);
		fprintf(out, "%lld\t;max node\n", maxnode);
		fprintf(out, "%d\t;style (rash 0 ~ %d cautious)\n", cautionfactor, CAUTION_NUM);
		fprintf(out, "%d\t;toolbar style (0: only icon, 1: both icon and words, 2: both with horizontally stacked)\n", showtoolbarboth);
		fprintf(out, "%d\t;show log (0: no, 1: yes)\n", showlog);
		fprintf(out, "%d\t;show number (0: no, 1: yes)\n", shownumber);
		fprintf(out, "%d\t;show analysis (0: no, 1: yes)\n", showanalysis);
		fprintf(out, "%d\t;show warning (0: no, 1: yes)\n", showwarning);
		fprintf(out, "%d\t;block autoreset (0: no, 1: yes)\n", blockautoreset);
		fprintf(out, "%d\t;number of threads\n", threadnum);
		fprintf(out, "%d\t;hash size (MB)\n", hashsizemb);
		fprintf(out, "%d\t;blockpath autoreset (0: no, 1: yes)\n", blockpathautoreset);
		fprintf(out, "%d\t;pondering (0: off, 1: on)\n", infopondering);
		fprintf(out, "%d\t;checkmate in global search (0: no, 1: vct, 2: vc2)\n", infovcthread);
		fprintf(out, "%d\t;hash autoclear (0: no, 1: yes)\n", hashautoclear);
		fprintf(out, "%d\t;toolbar postion (0: left vertical, 1: right horizontal)\n", toolbarpos);
		fprintf(out, "%d\t;show clock (0: no, 1: yes)\n", showclock);
		fprintf(out, "%d\t;time increment per move\n", increment);
		fprintf(out, "%d\t;show forbidden moves\n", showforbidden);
		fprintf(out, "%d\t;check timeout\n", checktimeout);
		fprintf(out, "%d\t;use database moves (0: no, 1: yes)\n", usedatabase);
		fprintf(out, "%d\t;enable database read-only mode (0: no, 1: yes)\n", databasereadonly);
		fprintf(out, "%d\t;show database baord texts (0: no, 1: yes)\n", showboardtext);
		fprintf(out, "%d\t;show database delall confirmation (0: no, 1: yes)\n", showdbdelconfirm);
		fprintf(out, "%d\t;record debug log\n", recorddebuglog);
		fprintf(out, "%d\t;log area horizontal scale\n", (int)(hdpiscale * 100 + 1e-10));
		fprintf(out, "%d\t;symmetric nbest for the 5th moves\n", nbestsym);
		fprintf(out, "%d\t;lossing move color saturation (0~100)\n", losssaturation);
		fprintf(out, "%d\t;winning move color saturation (0~100)\n", winsaturation);
		fprintf(out, "%d\t;min winrate color saturation (0~100)\n", minsaturation);
		fprintf(out, "%d\t;max winrate color saturation (0~100)\n", maxsaturation);
		fprintf(out, "%d\t;value of color (0~100)\n", colorvalue);
		fclose(out);
	}
	for (i = 0; i < toolbarnum; i++)
	{
		sprintf(s, "function/toolbar%d.txt", i + 1);
		if ((out = fopen(s, "w")) != NULL)
		{
			fprintf(out, "%d\n", toolbarlng[i]);
			fprintf(out, "%s\n", toolbaricon[i]);
			fprintf(out, "%s\n", toolbarcommand[i]);
			fclose(out);
		}
	}
	for (i = 0; i < hotkeynum; i++)
	{
		sprintf(s, "function/hotkey%d.txt", i + 1);
		if ((out = fopen(s, "w")) != NULL)
		{
			fprintf(out, "%d\n", hotkeykey[i]);
			fprintf(out, "%s\n", hotkeycommand[i]);
			fclose(out);
		}
	}
}

void yixin_quit()
{
	send_command("end\n");
}

void clock_label_refresh()
{
	char t[80];
	int h_turn, m_turn, s_turn;
	int h_match, m_match, s_match;
	int64_t tl_turn, tl_match;

	if ((timercomputerturn / 60 / 60 / 1000) >= 100)
	{
		h_turn = 99;
		m_turn = 59;
		s_turn = 59;
	}
	else
	{
		h_turn = (timercomputerturn / 60 / 60 / 1000) % 100;
		m_turn = (timercomputerturn / 60 / 1000) % 60;
		s_turn = (timercomputerturn / 1000) % 60;
	}
	if (((timercomputermatch + timercomputerturn) / 60 / 60 / 1000) >= 100)
	{
		h_match = 99;
		m_match = 59;
		s_match = 59;
	}
	else
	{
		h_match = ((timercomputermatch + timercomputerturn) / 60 / 60 / 1000) % 100;
		m_match = ((timercomputermatch + timercomputerturn) / 60 / 1000) % 60;
		s_match = ((timercomputermatch + timercomputerturn) / 1000) % 60;
	}
	sprintf(t, " Used: %02d:%02d:%02d / %02d:%02d:%02d ", h_turn, m_turn, s_turn, h_match, m_match, s_match);
	gtk_label_set_label(clocklabel[0], t);

	if ((timerhumanturn / 60 / 60 / 1000) >= 100)
	{
		h_turn = 99;
		m_turn = 59;
		s_turn = 59;
	}
	else
	{
		h_turn = (timerhumanturn / 60 / 60 / 1000) % 100;
		m_turn = (timerhumanturn / 60 / 1000) % 60;
		s_turn = (timerhumanturn / 1000) % 60;
	}
	if (((timerhumanmatch + timerhumanturn) / 60 / 60 / 1000) >= 100)
	{
		h_match = 99;
		m_match = 59;
		s_match = 59;
	}
	else
	{
		h_match = ((timerhumanmatch + timerhumanturn) / 60 / 60 / 1000) % 100;
		m_match = ((timerhumanmatch + timerhumanturn) / 60 / 1000) % 60;
		s_match = ((timerhumanmatch + timerhumanturn) / 1000) % 60;
	}
	sprintf(t, " Used: %02d:%02d:%02d / %02d:%02d:%02d ", h_turn, m_turn, s_turn, h_match, m_match, s_match);
	gtk_label_set_label(clocklabel[1], t);

	if (levelchoice != 1)
	{
		h_turn = h_match = 99;
		m_turn = m_match = 59;
		s_turn = s_match = 59;
	}
	else
	{
		tl_match = timeoutmatch - (timercomputermatch + timercomputerturn) + timercomputerincrement;
		tl_turn = min(timeoutturn - timercomputerturn, tl_match);
		if (tl_turn < 0)
			tl_turn = 0;
		if (tl_match < 0)
			tl_match = 0;
		if ((tl_turn / 60 / 60 / 1000) >= 100)
		{
			h_turn = 99;
			m_turn = 59;
			s_turn = 59;
		}
		else
		{
			h_turn = (tl_turn / 60 / 60 / 1000) % 100;
			m_turn = (tl_turn / 60 / 1000) % 60;
			s_turn = (tl_turn / 1000) % 60;
		}
		if ((tl_match / 60 / 60 / 1000) >= 100)
		{
			h_match = 99;
			m_match = 59;
			s_match = 59;
		}
		else
		{
			h_match = (tl_match / 60 / 60 / 1000) % 100;
			m_match = (tl_match / 60 / 1000) % 60;
			s_match = (tl_match / 1000) % 60;
		}
	}
	sprintf(t, " Left: %02d:%02d:%02d / %02d:%02d:%02d ", h_turn, m_turn, s_turn, h_match, m_match, s_match);
	gtk_label_set_label(clocklabel[2], t);

	if (levelchoice != 1)
	{
		h_turn = h_match = 99;
		m_turn = m_match = 59;
		s_turn = s_match = 59;
	}
	else
	{
		tl_match = timeoutmatch - (timerhumanmatch + timerhumanturn) + timerhumanincrement;
		tl_turn = min(timeoutturn - timerhumanturn, tl_match);
		if (tl_turn < 0)
			tl_turn = 0;
		if (tl_match < 0)
			tl_match = 0;
		if ((tl_turn / 60 / 60 / 1000) >= 100)
		{
			h_turn = 99;
			m_turn = 59;
			s_turn = 59;
		}
		else
		{
			h_turn = (tl_turn / 60 / 60 / 1000) % 100;
			m_turn = (tl_turn / 60 / 1000) % 60;
			s_turn = (tl_turn / 1000) % 60;
		}
		if ((tl_match / 60 / 60 / 1000) >= 100)
		{
			h_match = 99;
			m_match = 59;
			s_match = 59;
		}
		else
		{
			h_match = (tl_match / 60 / 60 / 1000) % 100;
			m_match = (tl_match / 60 / 1000) % 60;
			s_match = (tl_match / 1000) % 60;
		}
	}
	sprintf(t, " Left: %02d:%02d:%02d / %02d:%02d:%02d ", h_turn, m_turn, s_turn, h_match, m_match, s_match);
	gtk_label_set_label(clocklabel[3], t);

	if (checktimeout && timeoutflag == 0)
	{
		if ((h_turn == 0 && m_turn == 0 && s_turn == 0) ||
			(h_match == 0 && m_match == 0 && s_match == 0))
		{
			timeoutflag = 1;
			show_dialog_timeout(windowmain);
		}
	}
}

gboolean clock_timer_update()
{
	int t;
	if (timerstatus == 0)
	{
		timerhumanmatch += timerhumanturn;
		timerhumanturn = 0;
		timercomputermatch += timercomputerturn;
		timercomputerturn = 0;
	}
	else if (timerstatus == 1)
	{
		timerhumanmatch += timerhumanturn;
		timerhumanturn = 0;
		t = (clock() - timerstart) / (CLOCKS_PER_SEC / 1000);
		if (t < timercomputerturn)
		{
			timercomputermatch += timercomputerturn;
			timercomputerturn = 0;
		}
		timercomputerturn = t;
	}
	else if (timerstatus == 2)
	{
		timercomputermatch += timercomputerturn;
		timercomputerturn = 0;
		t = (clock() - timerstart) / (CLOCKS_PER_SEC / 1000);
		if (t < timerhumanturn)
		{
			timerhumanmatch += timerhumanturn;
			timerhumanturn = 0;
		}
		timerhumanturn = t;
	}
	clock_label_refresh();
	return TRUE;
}

void clock_timer_change_status(int status)
{
	if (timerstatus == 0)
		return;
	if (timerstatus == status)
		return;
	if (status == 0)
	{
		timerstatus = 0;
		return;
	}
	if (status == 1 && isthinking)
	{
		timerstatus = 1;
	}
	else if (status == 2 && !isthinking)
	{
		timerstatus = 2;
	}
	timerstart = clock();
}

void clock_timer_init()
{
	if (isthinking)
	{
		timerstart = clock();
		timerstatus = 1;
	}
	else
	{
		timerstart = clock();
		timerstatus = 2;
	}
	timercomputermatch = 0;
	timerhumanmatch = 0;
	timercomputerturn = 0;
	timerhumanturn = 0;
	timercomputerincrement = 0;
	timerhumanincrement = 0;
	timeoutflag = 0;
	clock_label_refresh();
}

GdkPixbuf *copy_subpixbuf(GdkPixbuf *_src, int src_x, int src_y, int width, int height)
{
	GdkPixbuf *dst, *src;
	int sample, channels;
	gboolean alpha;
	GdkColorspace colorspace;

	guchar *pixels1, *pixels2;
	int rowstride1, rowstride2;
	int i, j;

	src = gdk_pixbuf_new_subpixbuf(_src, src_x, src_y, width, height);
	sample = gdk_pixbuf_get_bits_per_sample(src);
	channels = gdk_pixbuf_get_n_channels(src);
	alpha = gdk_pixbuf_get_has_alpha(src);
	colorspace = gdk_pixbuf_get_colorspace(src);
	dst = gdk_pixbuf_new(colorspace, alpha, sample, width, height);

	pixels1 = gdk_pixbuf_get_pixels(src);
	pixels2 = gdk_pixbuf_get_pixels(dst);

	rowstride1 = gdk_pixbuf_get_rowstride(src);
	rowstride2 = gdk_pixbuf_get_rowstride(dst);
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			guchar *p1, *p2;
			p1 = pixels1 + i * rowstride1 + j * channels;
			p2 = pixels2 + i * rowstride2 + j * channels;
			memcpy(p2, p1, channels);
		}
	}
	g_object_unref(src);
	src = NULL;
	return dst;
}

gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	isctrlpressed = (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK;
	int hasmask = (event->state & GDK_CONTROL_MASK) != 0;

	for (int i = 0; i < hotkeynum; i++)
	{
		int keymask = (hotkeykeylist[hotkeykey[i]][0] & GDK_CONTROL_MASK) != 0;
		if (!(hasmask ^ keymask) && event->keyval == hotkeykeylist[hotkeykey[i]][1])
		{
			hotkey_function(widget, GINT_TO_POINTER(i));
			return TRUE;
		}
	}

	return FALSE;
}

gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	isctrlpressed = (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK;
	return FALSE;
}

void create_windowclock()
{
	GtkWidget *vbox;

	windowclock = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(windowclock), FALSE);
	gtk_window_set_deletable(GTK_WINDOW(windowclock), FALSE);
	g_signal_connect(G_OBJECT(windowclock), "delete_event", G_CALLBACK(windowclock_delete), NULL);
	gtk_window_set_title(GTK_WINDOW(windowclock), "Clock");
	gtk_window_set_type_hint(GTK_WINDOW(windowclock), GDK_WINDOW_TYPE_HINT_DIALOG);

	clocklabel[0] = gtk_label_new(" Used: 00:00:00 / 00:00:00 ");
	clocklabel[1] = gtk_label_new(" Used: 00:00:00 / 00:00:00 ");
	clocklabel[2] = gtk_label_new(" Left: 00:00:00 / 00:00:00 ");
	clocklabel[3] = gtk_label_new(" Left: 00:00:00 / 00:00:00 ");

	playerlabel[0] = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(playerlabel[0]), "<big><b>Computer</b></big>");
	// gtk_label_set_markup(GTK_LABEL(playerlabel[0]), "<big><b>Yixin</b></big>");

	playerlabel[1] = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(playerlabel[1]), "<big><b>Human</b></big>");
	// gtk_label_set_markup(GTK_LABEL(playerlabel[1]), "<big><b>Lin Shu-Hsuan</b></big>");
	// gtk_label_set_markup(GTK_LABEL(playerlabel[1]), "<big><b>Rudolf Dupszki</b></big>");

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_box_pack_start(GTK_BOX(vbox), playerlabel[0], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), clocklabel[0], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), clocklabel[2], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), playerlabel[1], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), clocklabel[1], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), clocklabel[3], FALSE, FALSE, 3);

	gtk_container_add(GTK_CONTAINER(windowclock), vbox);
	clock_timer_init();

	g_timeout_add(200, clock_timer_update, NULL);

	if (showclock)
	{
		gtk_widget_show_all(windowclock);
		gtk_window_move(windowclock, 0, 0);
	}
}

gboolean create_menu_proxy(GtkToolItem *tool_item, gpointer user_data)
{
	int i = GPOINTER_TO_INT(user_data);
	GtkWidget *menu_item = gtk_menu_item_new();
	GtkWidget *menu_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

	GtkStyleContext *context = gtk_widget_get_style_context(menu_item);
	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider,
									"menuitem { padding: 4px 0px; }", -1, NULL);
	gtk_style_context_add_provider(context,
								   GTK_STYLE_PROVIDER(provider),
								   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(provider);

	GtkWidget *menu_image = gtk_image_new_from_icon_name(toolbaricon[i] ? toolbaricon[i] : "image-missing", GTK_ICON_SIZE_MENU);
	GtkWidget *menu_label = gtk_label_new(_T(clanguage[toolbarlng[i]]));

	gtk_box_pack_start(GTK_BOX(menu_box), menu_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(menu_box), menu_label, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(menu_item), menu_box);
	gtk_widget_show_all(menu_item);

	g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(toolbar_function), GINT_TO_POINTER(i));
	gtk_tool_item_set_proxy_menu_item(tool_item, g_strdup_printf("overflow-proxy-%d", i), menu_item);
}

void create_windowmain()
{
	GtkWidget *menubar, *menugame, *menuplayers, *menuview, *menuhelp, *menurule, *menulanguage;
	GtkWidget *menuitemgame, *menuitemplayers, *menuitemview, *menuitemhelp;
	GtkWidget *menuitemnewgame, *menuitemload, *menuitemsave, *menuitemrule, *menuitemsize, *menuitemquit,
		*menuitemrule1, *menuitemrule2, *menuitemrule3, *menuitemrule4, *menuitemrule5, *menuitemrule6, *menuitemrule7, *menuitemrule8;
	GtkWidget *menuitemcomputerplaysblack, *menuitemcomputerplayswhite, *menuitemchecktimeout, *menuitemsettings;
	GtkWidget *menuitemlanguage, *menuitemenglish, *menuitemcustomlng[16] = {0};
	GtkWidget *menuitemnumeration, *menuitemlog, *menuitemanalysis, *menuitemclock, *menuitemforbidden;
	GtkWidget *menuitemabout, *menuitemdatabase, *menuitemdbreadonly, *menuitemboardtext;

	GtkToolItem *tools[MAX_TOOLBAR_ITEM];

	GtkWidget *textcommand, *btncopypos, *btnsetpos;

	GdkPixbuf *pixbuf;

	GtkWidget *hbox[2], *vbox[2];

	int size, sample, channels, rowstride;
	gboolean alpha;
	GdkColorspace colorspace;
	GdkPixbuf *background;

	char lnglis[16][64];

	int i, j, k, l;

	windowmain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(windowmain), FALSE);
	g_signal_connect(G_OBJECT(windowmain), "delete-event", G_CALLBACK(on_windows_close), NULL);
	gtk_window_set_title(GTK_WINDOW(windowmain), "Yixin");
	tableboard = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(tableboard), 0);
	gtk_grid_set_column_spacing(GTK_GRID(tableboard), 0);
	gtk_grid_set_row_homogeneous(GTK_GRID(tableboard), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(tableboard), TRUE);

	GtkWidget *eventbox_board = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(eventbox_board), tableboard);
	gtk_widget_add_events(eventbox_board, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(eventbox_board), "button_press_event", G_CALLBACK(on_button_press_windowmain), NULL);

	pixbuf = gdk_pixbuf_new_from_file(piecepicname, NULL);
	size = gdk_pixbuf_get_height(pixbuf);
	sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
	alpha = gdk_pixbuf_get_has_alpha(pixbuf);
	colorspace = gdk_pixbuf_get_colorspace(pixbuf);
	background = gdk_pixbuf_new_subpixbuf(pixbuf, size * 16, 3, 1, 1);
	pixbufboard[0][0] = copy_subpixbuf(pixbuf, 0, 0, size, size);
	channels = gdk_pixbuf_get_n_channels(pixbufboard[0][0]);
	rowstride = gdk_pixbuf_get_rowstride(pixbufboard[0][0]);

	pixbufboard[1][0] = copy_subpixbuf(pixbuf, size, 0, size, size);
	for (i = 2; i <= 4; i++)
	{
		guchar *pixels1, *pixels2;

		pixbufboard[i][0] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
		pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i - 1][0]);
		pixels2 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
		for (j = 0; j < size; j++)
		{
			for (k = 0; k < size; k++)
			{
				guchar *p1, *p2;
				p1 = pixels1 + j * rowstride + k * channels;
				p2 = pixels2 + k * rowstride + (size - 1 - j) * channels;
				memcpy(p2, p1, channels);
			}
		}
	}
	pixbufboard[5][0] = copy_subpixbuf(pixbuf, size * 2, 0, size, size);
	for (i = 6; i <= 8; i++)
	{
		guchar *pixels1, *pixels2, *pixels3;

		pixbufboard[i][0] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
		pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i - 1][0]);
		pixels2 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
		pixels3 = gdk_pixbuf_get_pixels(background);
		for (j = 0; j < size; j++)
		{
			for (k = 0; k < size; k++)
			{
				guchar *p1, *p2;
				if (size % 2 == 1 || i != 7)
				{
					p1 = pixels1 + j * rowstride + k * channels;
					p2 = pixels2 + k * rowstride + (size - 1 - j) * channels;
				}
				else
				{
					p1 = pixels1 + ((j + 1) % size) * rowstride + k * channels;
					p2 = pixels2 + k * rowstride + (size - 1 - j) * channels;
				}
				memcpy(p2, p1, channels);
			}
		}
	}
	for (i = 0; i <= 8; i++)
	{
		for (j = 1; j <= 2; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size * (j + 2), 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for (k = 0; k < size; k++)
			{
				for (l = 0; l < size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k * rowstride + l * channels;
					p2 = pixels2 + k * rowstride + l * channels;
					p3 = pixels3;
					p4 = pixels4 + k * rowstride + l * channels;
					if (memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
		for (j = 3; j <= 4; j++)
		{
			GdkPixbuf *pbt1, *pbt2;
			guchar *pixels1, *pixels2, *pixels3, *pixels4, *pixels5;
			pbt1 = copy_subpixbuf(pixbuf, size * (j + 4), 0, size, size);
			pbt2 = copy_subpixbuf(pixbuf, size * 6, 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt1);
			pixels3 = gdk_pixbuf_get_pixels(pbt2);
			pixels4 = gdk_pixbuf_get_pixels(background);
			pixels5 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for (k = 0; k < size; k++)
			{
				for (l = 0; l < size; l++)
				{
					guchar *p1, *p2, *p3, *p4, *p5;
					p1 = pixels1 + k * rowstride + l * channels;
					p2 = pixels2 + k * rowstride + l * channels;
					p3 = pixels3 + k * rowstride + l * channels;
					p4 = pixels4;
					p5 = pixels5 + k * rowstride + l * channels;
					if (memcmp(p3, p4, channels) != 0 || memcmp(p1, p4, channels) == 0)
						memcpy(p5, p2, channels);
					else
						memcpy(p5, p1, channels);
				}
			}
			g_object_unref(pbt1);
			pbt1 = NULL;
			g_object_unref(pbt2);
			pbt2 = NULL;
		}
		for (j = 5; j <= 6; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size * 5, 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][j - 2]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for (k = 0; k < size; k++)
			{
				for (l = 0; l < size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k * rowstride + l * channels;
					p2 = pixels2 + k * rowstride + l * channels;
					p3 = pixels3;
					p4 = pixels4 + k * rowstride + l * channels;
					if (memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
		for (j = 7; j <= 13; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size * (j + 2), 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for (k = 0; k < size; k++)
			{
				for (l = 0; l < size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k * rowstride + l * channels;
					p2 = pixels2 + k * rowstride + l * channels;
					p3 = pixels3;
					p4 = pixels4 + k * rowstride + l * channels;
					if (memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
	}

	for (i = 0; i < boardsizeh; i++)
	{
		for (j = 0; j < boardsizew; j++)
		{
			if (i == 0 && j == 0)
				imgtypeboard[i][j] = 1;
			else if (i == 0 && j == boardsizew - 1)
				imgtypeboard[i][j] = 2;
			else if (i == boardsizeh - 1 && j == boardsizew - 1)
				imgtypeboard[i][j] = 3;
			else if (i == boardsizeh - 1 && j == 0)
				imgtypeboard[i][j] = 4;
			else if (i == 0)
				imgtypeboard[i][j] = 5;
			else if (j == boardsizew - 1)
				imgtypeboard[i][j] = 6;
			else if (i == boardsizeh - 1)
				imgtypeboard[i][j] = 7;
			else if (j == 0)
				imgtypeboard[i][j] = 8;
			else if (i == boardsizeh / 2 && j == boardsizew / 2)
				imgtypeboard[i][j] = 9;
			else if (i == boardsizeh / 4 && j == boardsizew / 4)
				imgtypeboard[i][j] = 9;
			else if (i == boardsizeh / 4 && j == boardsizew - 1 - boardsizew / 4)
				imgtypeboard[i][j] = 9;
			else if (i == boardsizeh - 1 - boardsizeh / 4 && j == boardsizew / 4)
				imgtypeboard[i][j] = 9;
			else if (i == boardsizeh - 1 - boardsizeh / 4 && j == boardsizew - 1 - boardsizew / 4)
				imgtypeboard[i][j] = 9;
			else
				imgtypeboard[i][j] = 0;
		}
	}
	for (i = 0; i < boardsizeh; i++)
	{
		char tlabel[3];
		sprintf(tlabel, "%d", boardsizeh - i);
		labelboard[0][i] = gtk_label_new(tlabel);
		gtk_grid_attach(GTK_GRID(tableboard), labelboard[0][i], boardsizew, i, 1, 1);
	}
	for (i = 0; i < boardsizew; i++)
	{
		char tlabel[3];
		sprintf(tlabel, "%c", 'A' + i);
		labelboard[1][i] = gtk_label_new(tlabel);
		gtk_grid_attach(GTK_GRID(tableboard), labelboard[1][i], i, boardsizeh, 1, 1);
	}
	for (i = 0; i < boardsizeh; i++)
	{
		for (j = 0; j < boardsizew; j++)
		{
			if (imgtypeboard[i][j] <= 8)
				imageboard[i][j] = gtk_image_new_from_pixbuf(pixbufboard[imgtypeboard[i][j]][0]);
			else
				imageboard[i][j] = gtk_image_new_from_pixbuf(pixbufboard[0][1]);
			gtk_grid_attach(GTK_GRID(tableboard), imageboard[i][j], j, i, 1, 1);
		}
	}

	g_object_unref(background);
	background = NULL;
	g_object_unref(pixbuf);
	pixbuf = NULL;

	menubar = gtk_menu_bar_new();
	menugame = gtk_menu_new();
	menuplayers = gtk_menu_new();
	menuview = gtk_menu_new();
	menuhelp = gtk_menu_new();
	menurule = gtk_menu_new();
	menulanguage = gtk_menu_new();

	menuitemgame = gtk_menu_item_new_with_label(language == 0 ? "Game" : _T(clanguage[59]));
	menuitemplayers = gtk_menu_item_new_with_label(language == 0 ? "Players" : _T(clanguage[60]));
	menuitemview = gtk_menu_item_new_with_label(language == 0 ? "View" : _T(clanguage[61]));
	menuitemhelp = gtk_menu_item_new_with_label(language == 0 ? "Help" : _T(clanguage[62]));
	menuitemnewgame = gtk_menu_item_new_with_label(language == 0 ? "New" : _T(clanguage[63]));
	menuitemload = gtk_menu_item_new_with_label(language == 0 ? "Load" : _T(clanguage[64]));
	menuitemsave = gtk_menu_item_new_with_label(language == 0 ? "Save" : _T(clanguage[65]));
	menuitemrule = gtk_menu_item_new_with_label(language == 0 ? "Rule" : _T(clanguage[66]));
	menuitemsize = gtk_menu_item_new_with_label(language == 0 ? "Board Size" : _T(clanguage[67]));
	menuitemnumeration = gtk_check_menu_item_new_with_label(language == 0 ? "Numeration" : _T(clanguage[69]));
	menuitemlog = gtk_check_menu_item_new_with_label(language == 0 ? "Log" : _T(clanguage[70]));
	menuitemanalysis = gtk_check_menu_item_new_with_label(language == 0 ? "Analysis" : _T(clanguage[71]));
	menuitemclock = gtk_check_menu_item_new_with_label(language == 0 ? "Clock" : _T(clanguage[94]));
	menuitemforbidden = gtk_check_menu_item_new_with_label(language == 0 ? "Forbidden Move" : _T(clanguage[97]));
	menuitemdatabase = gtk_check_menu_item_new_with_label(language == 0 ? "Use Database" : _T(clanguage[103]));
	menuitemdbreadonly = gtk_check_menu_item_new_with_label(language == 0 ? "Database Readonly" : _T(clanguage[112]));
	menuitemboardtext = gtk_check_menu_item_new_with_label(language == 0 ? "Board Text" : _T(clanguage[113]));
	menuitemlanguage = gtk_menu_item_new_with_label(language == 0 ? "Language" : _T(clanguage[72]));
	menuitemquit = gtk_menu_item_new_with_label(language == 0 ? "Quit" : _T(clanguage[73]));
	menuitemabout = gtk_menu_item_new_with_label(language == 0 ? "About" : _T(clanguage[74]));
	menuitemsettings = gtk_menu_item_new_with_label(language == 0 ? "Settings" : _T(clanguage[75]));

	menuitemrule1 = gtk_radio_menu_item_new_with_label(NULL, language == 0 ? "Freestyle Gomoku" : _T(clanguage[76]));
	menuitemrule2 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language == 0 ? "Standard Gomoku" : _T(clanguage[77]));
	menuitemrule3 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language == 0 ? "Free Renju" : _T(clanguage[78]));
	menuitemrule5 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language == 0 ? "Swap After First Move" : _T(clanguage[80]));
	menuitemrule6 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language == 0 ? "Swap2" : _T(clanguage[100]));
	menuitemrule8 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language == 0 ? "Soosorv-8 Opening Rule" : _T(clanguage[102]));

	menuitemenglish = gtk_radio_menu_item_new_with_label(NULL, "English");
	for (i = 1; i < 16; i++)
	{
		FILE *in;
		char s[80];
		sprintf(s, "language/%d.lng", i);
		if ((in = fopen(s, "r")) != NULL)
		{
			while (fgets(s, sizeof(s), in))
			{
				int l = strlen(s);
				int p;
				while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r'))
				{
					s[l - 1] = 0;
					l--;
				}
				if (l == 0)
					break;
				if (s[0] == ';')
					break;
				sscanf(s, "%d", &p);
				if (p == 84)
				{
					for (j = 0; j < l && s[j] != '='; j++)
						;
					strcpy(lnglis[i], s + j + 1);
					break;
				}
			}
			fclose(in);
			menuitemcustomlng[i] = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemenglish)), _T(lnglis[i]));
		}
	}

	switch (inforule)
	{
	case 0:
		if (specialrule == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule1), TRUE);
		else if (specialrule == 2)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule5), TRUE);
		break;
	case 1:
		if (specialrule == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule2), TRUE);
		else if (specialrule == 4)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule6), TRUE);
		break;
	case 2:
		if (specialrule == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule3), TRUE);
		else if (specialrule == 3)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule8), TRUE);
		break;
	}
	set_rule();

	if (language == 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemenglish), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcustomlng[language]), TRUE);

	if (shownumber)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemnumeration), TRUE);
	}
	if (showlog)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemlog), TRUE);
	}
	if (showanalysis)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemanalysis), TRUE);
	}
	if (showforbidden)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemforbidden), TRUE);
	}
	if (showboardtext)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemboardtext), TRUE);
	}
	if (showclock)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemclock), TRUE);
	}
	if (usedatabase)
	{
		usedatabase = 0;
		use_database(NULL, NULL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemdatabase), TRUE);
	}
	if (databasereadonly)
	{
		databasereadonly = 0;
		set_database_readonly(NULL, NULL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemdbreadonly), TRUE);
	}
	menuitemcomputerplaysblack = gtk_check_menu_item_new_with_label(language == 0 ? "Computer Plays Black" : _T(clanguage[81]));
	menuitemcomputerplayswhite = gtk_check_menu_item_new_with_label(language == 0 ? "Computer Plays White" : _T(clanguage[82]));
	menuitemchecktimeout = gtk_check_menu_item_new_with_label(language == 0 ? "Check Timeout" : _T(clanguage[99]));
	if (computerside & 1)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplaysblack), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplaysblack), FALSE);
	if (computerside >> 1)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplayswhite), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplayswhite), FALSE);
	if (checktimeout)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemchecktimeout), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemchecktimeout), FALSE);
	change_side_menu(3, menuitemcomputerplaysblack);
	change_side_menu(4, menuitemcomputerplayswhite);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemgame), menugame);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemplayers), menuplayers);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemview), menuview);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemhelp), menuhelp);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemrule), menurule);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemlanguage), menulanguage);

	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule1);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule2);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule3);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule5);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule6);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule8);

	gtk_menu_shell_append(GTK_MENU_SHELL(menulanguage), menuitemenglish);
	for (i = 1; i < 16; i++)
	{
		if (menuitemcustomlng[i] != NULL)
			gtk_menu_shell_append(GTK_MENU_SHELL(menulanguage), menuitemcustomlng[i]);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemnewgame);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemload);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemsave);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemrule);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemsize);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemdatabase);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemdbreadonly);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemquit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemnumeration);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemlog);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemanalysis);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemforbidden);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemboardtext);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemclock);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemlanguage);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemcomputerplaysblack);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemcomputerplayswhite);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemchecktimeout);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemsettings);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuhelp), menuitemabout);

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemgame);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemplayers);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemhelp);

	g_signal_connect(G_OBJECT(menuitemnewgame), "activate", G_CALLBACK(new_game_resetclock), NULL);
	g_signal_connect(G_OBJECT(menuitemload), "activate", G_CALLBACK(show_dialog_load), windowmain);
	g_signal_connect(G_OBJECT(menuitemsave), "activate", G_CALLBACK(show_dialog_save), windowmain);
	g_signal_connect(G_OBJECT(menuitemnumeration), "activate", G_CALLBACK(view_numeration), NULL);
	g_signal_connect(G_OBJECT(menuitemlog), "activate", G_CALLBACK(view_log), NULL);
	g_signal_connect(G_OBJECT(menuitemanalysis), "activate", G_CALLBACK(view_analysis), NULL);
	g_signal_connect(G_OBJECT(menuitemclock), "activate", G_CALLBACK(view_clock), NULL);
	g_signal_connect(G_OBJECT(menuitemforbidden), "activate", G_CALLBACK(view_forbidden), NULL);
	g_signal_connect(G_OBJECT(menuitemdatabase), "activate", G_CALLBACK(use_database), NULL);
	g_signal_connect(G_OBJECT(menuitemdbreadonly), "activate", G_CALLBACK(set_database_readonly), NULL);
	g_signal_connect(G_OBJECT(menuitemboardtext), "activate", G_CALLBACK(view_boardtext), NULL);
	g_signal_connect(G_OBJECT(menuitemquit), "activate", G_CALLBACK(yixin_quit), NULL);
	g_signal_connect(G_OBJECT(menuitemabout), "activate", G_CALLBACK(show_dialog_about), GTK_WINDOW(windowmain));
	g_signal_connect(G_OBJECT(menuitemsettings), "activate", G_CALLBACK(show_dialog_settings), GTK_WINDOW(windowmain));
	g_signal_connect(G_OBJECT(menuitemrule1), "activate", G_CALLBACK(change_rule), (gpointer)0);
	g_signal_connect(G_OBJECT(menuitemrule2), "activate", G_CALLBACK(change_rule), (gpointer)1);
	g_signal_connect(G_OBJECT(menuitemrule3), "activate", G_CALLBACK(change_rule), (gpointer)2);
	g_signal_connect(G_OBJECT(menuitemrule5), "activate", G_CALLBACK(change_rule), (gpointer)4);
	g_signal_connect(G_OBJECT(menuitemrule6), "activate", G_CALLBACK(change_rule), (gpointer)6);
	g_signal_connect(G_OBJECT(menuitemrule8), "activate", G_CALLBACK(change_rule), (gpointer)5);
	g_signal_connect(G_OBJECT(menuitemsize), "activate", G_CALLBACK(show_dialog_size), 0);
	g_signal_connect(G_OBJECT(menuitemcomputerplaysblack), "activate", G_CALLBACK(change_side), (gpointer)1);
	g_signal_connect(G_OBJECT(menuitemcomputerplayswhite), "activate", G_CALLBACK(change_side), (gpointer)2);
	g_signal_connect(G_OBJECT(menuitemchecktimeout), "activate", G_CALLBACK(change_timeoutcheck), NULL);
	g_signal_connect(G_OBJECT(menuitemenglish), "activate", G_CALLBACK(change_language), (gpointer)0);
	for (i = 1; i < 16; i++)
	{
		if (menuitemcustomlng[i] != NULL)
			g_signal_connect(G_OBJECT(menuitemcustomlng[i]), "activate", G_CALLBACK(change_language), GINT_TO_POINTER(i));
	}

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar), TRUE);
	if (showtoolbarboth == 0)
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	else if (showtoolbarboth == 1)
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);

	for (i = 0; i < toolbarnum; ++i)
	{
		GtkToolItem *tool_btn = gtk_tool_button_new(NULL, NULL);
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(tool_btn), toolbaricon[i]);
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(tool_btn), _T(clanguage[toolbarlng[i]]));
		g_signal_connect(G_OBJECT(tool_btn), "clicked", G_CALLBACK(toolbar_function), GINT_TO_POINTER(i));
		g_signal_connect(G_OBJECT(tool_btn), "create-menu-proxy", G_CALLBACK(create_menu_proxy), GINT_TO_POINTER(i));
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_btn, -1);
	}

	PangoFontDescription *boardtextfontdesc = pango_font_description_from_string(fontname[0]);
	GtkCssProvider *css_providerboardtext = gtk_css_provider_new();
	font_familyboardtext = pango_font_description_get_family(boardtextfontdesc);
	font_sizeboardtext = pango_font_description_get_size(boardtextfontdesc) / PANGO_SCALE;
	if (font_sizeboardtext == 0)
		font_sizeboardtext = 10;

	textdbcomment = gtk_text_view_new();
	buffertextdbcomment = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textdbcomment));
	scrolledtextdbcomment = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledtextdbcomment), textdbcomment);
	gtk_widget_set_size_request(scrolledtextdbcomment, (int)(hdpiscale * 400), (int)(hdpiscale * 100));
	PangoFontDescription *dbcommentfontdesc = pango_font_description_from_string(fontname[2]);
	GtkCssProvider *css_providerdbcomment = gtk_css_provider_new();
	const char *font_familydbcomment = pango_font_description_get_family(dbcommentfontdesc);
	int font_sizedbcomment = pango_font_description_get_size(dbcommentfontdesc) / PANGO_SCALE;
	gchar *css_data = g_strdup_printf("textview { font-family: '%s'; font-size: %dpt; }",
									  font_familydbcomment ? font_familydbcomment : "Sans", font_sizedbcomment > 0 ? font_sizedbcomment : 10);
	gtk_css_provider_load_from_data(css_providerdbcomment, css_data, -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(textdbcomment),
								   GTK_STYLE_PROVIDER(css_providerdbcomment), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(css_providerdbcomment);
	g_free(css_data);
	pango_font_description_free(dbcommentfontdesc);
	g_signal_connect(buffertextdbcomment, "end-user-action", G_CALLBACK(dbcomment_changed), NULL);
	if (databasereadonly)
		gtk_text_view_set_editable(GTK_TEXT_VIEW(textdbcomment), 0);

	textlog = gtk_text_view_new();
	PangoFontDescription *logfontdesc = pango_font_description_from_string(fontname[1]);
	GtkCssProvider *css_providerlog = gtk_css_provider_new();
	const char *font_familylog = pango_font_description_get_family(logfontdesc);
	int font_sizelog = pango_font_description_get_size(logfontdesc) / PANGO_SCALE;
	gchar *css_data2 = g_strdup_printf("textview { font-family: '%s'; font-size: %dpt; }",
									   font_familylog ? font_familylog : "Sans", font_sizelog > 0 ? font_sizelog : 10);
	gtk_css_provider_load_from_data(css_providerlog, css_data2, -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(textlog),
								   GTK_STYLE_PROVIDER(css_providerlog), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(css_providerlog);
	g_free(css_data2);
	pango_font_description_free(logfontdesc);
	gtk_text_view_set_editable(textlog, 0);

	buffertextlog = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textlog));
	scrolledtextlog = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledtextlog), textlog);
	gtk_widget_set_size_request(scrolledtextlog, (int)(hdpiscale * 400), max(0, size * boardsizeh - (int)(hdpiscale * 200) - (toolbarpos == 1 ? 50 : 0)));

	textcommand = gtk_text_view_new();
	buffertextcommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledtextcommand = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledtextcommand), textcommand);
	gtk_widget_set_size_request(scrolledtextcommand, (int)(hdpiscale * 400), (int)(hdpiscale * 50));
	g_signal_connect(textcommand, "key-release-event", G_CALLBACK(key_command), NULL);

	textpos = gtk_text_view_new();
	buffertextpos = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textpos));
	scrolledtextpos = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledtextpos), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scrolledtextpos), textpos);
	gtk_widget_set_size_request(scrolledtextpos, (int)(hdpiscale * 400), (int)(hdpiscale * 50));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textpos), GTK_WRAP_CHAR);
	PangoFontDescription *posfontdesc = pango_font_description_from_string(fontname[3]);
	GtkCssProvider *css_providerpos = gtk_css_provider_new();
	const char *font_familypos = pango_font_description_get_family(posfontdesc);
	int font_sizepos = pango_font_description_get_size(posfontdesc) / PANGO_SCALE;
	gchar *css_data3 = g_strdup_printf("textview { font-family: '%s'; font-size: %dpt; }",
									   font_familypos ? font_familypos : "Sans", font_sizepos > 0 ? font_sizepos : 10);
	gtk_css_provider_load_from_data(css_providerpos, css_data3, -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(textpos),
								   GTK_STYLE_PROVIDER(css_providerpos), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(css_providerpos);
	g_free(css_data3);
	pango_font_description_free(posfontdesc);
	g_signal_connect(buffertextpos, "end-user-action", G_CALLBACK(textpos_changed), NULL);

	btncopypos = gtk_button_new();
	btnsetpos = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(btncopypos), gtk_image_new_from_icon_name("edit-copy", GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image(GTK_BUTTON(btnsetpos), gtk_image_new_from_icon_name("edit-paste", GTK_ICON_SIZE_BUTTON));
	g_signal_connect(btncopypos, "clicked", G_CALLBACK(textpos_button_clicked), GINT_TO_POINTER(1));
	g_signal_connect(btnsetpos, "clicked", G_CALLBACK(textpos_button_clicked), GINT_TO_POINTER(2));

	posbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(posbar), scrolledtextpos, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(posbar), btncopypos, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(posbar), btnsetpos, FALSE, FALSE, 2);

	vbox[0] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	if (toolbarpos == 1)
	{
		g_object_set(toolbar, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
		GtkWidget *toolbar_scrolled = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(toolbar_scrolled),
									   GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
		int max_toolbar_width = (int)(hdpiscale * 400);
		gtk_widget_set_size_request(toolbar_scrolled, max_toolbar_width, -1);
		gtk_container_add(GTK_CONTAINER(toolbar_scrolled), toolbar);
		gtk_box_pack_start(GTK_BOX(vbox[0]), toolbar_scrolled, FALSE, FALSE, 3);
	}
	gtk_box_pack_start(GTK_BOX(vbox[0]), posbar, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox[0]), scrolledtextdbcomment, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox[0]), scrolledtextlog, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(vbox[0]), scrolledtextcommand, FALSE, FALSE, 3);

	hbox[0] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	if (toolbarpos == 0)
	{
		g_object_set(toolbar, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
		GtkWidget *toolbar_scrolled = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(toolbar_scrolled),
									   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		int max_toolbar_height = max(200, size * boardsizeh - 150);
		gtk_widget_set_size_request(toolbar_scrolled, -1, max_toolbar_height);
		gtk_container_add(GTK_CONTAINER(toolbar_scrolled), toolbar);
		gtk_box_pack_start(GTK_BOX(hbox[0]), toolbar_scrolled, FALSE, FALSE, 3);
	}
	gtk_box_pack_start(GTK_BOX(hbox[0]), eventbox_board, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[0]), vbox[0], FALSE, FALSE, 3);

	vboxwindowmain = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(vboxwindowmain), menubar, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vboxwindowmain), hbox[0], FALSE, FALSE, 3);

	gtk_container_add(GTK_CONTAINER(windowmain), vboxwindowmain);

	g_signal_connect(G_OBJECT(windowmain), "key-press-event", G_CALLBACK(key_press), NULL);
	g_signal_connect(G_OBJECT(windowmain), "key-release-event", G_CALLBACK(key_release), NULL);

	gtk_widget_show_all(windowmain);

	gtk_window_set_position(windowmain, GTK_WIN_POS_CENTER_ALWAYS);

	if (!showlog)
	{
		gtk_widget_hide(posbar);
		gtk_widget_hide(scrolledtextdbcomment);
		gtk_widget_hide(scrolledtextlog);
		gtk_widget_hide(scrolledtextcommand);
		gtk_widget_hide(toolbar);
	}
	else if (!usedatabase)
	{
		gtk_widget_hide(scrolledtextdbcomment);
	}
}

gboolean iochannelout_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	gchar *string, *rawstring;
	gsize size;
	int x, y, x2, y2;
	int i;
	char command[80];

	if (cond & G_IO_HUP)
	{
		g_io_channel_unref(channel);
		return FALSE;
	}
	do
	{
		g_io_channel_read_line(channel, &rawstring, &size, NULL, NULL);

		if (commandmodel == 1)
		{
			print_log(rawstring);
			g_free(rawstring);
			continue;
		}

		if (debuglog != NULL)
		{
			fprintf(debuglog, "RECEIVE_COMMAND [%s,%s,%s,%s]: %s\n",
					gtk_label_get_text(clocklabel[0]),
					gtk_label_get_text(clocklabel[1]),
					gtk_label_get_text(clocklabel[2]),
					gtk_label_get_text(clocklabel[3]),
					rawstring);
			fflush(debuglog);
		}

		string = g_strndup(rawstring, size);
		for (i = 0; i < (int)size; i++)
		{
			if (string[i] >= 'a' && string[i] <= 'z')
				string[i] = string[i] - 'a' + 'A'; // convert to upper case
		}

		if (strncmp(string, "OK", 2) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "MESSAGE SWAP2 MOVE1", 19) == 0)
		{
			char *p = string + 19 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
		}
		else if (strncmp(string, "MESSAGE SWAP2 MOVE2", 19) == 0)
		{
			char *p = string + 19 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
		}
		else if (strncmp(string, "MESSAGE SWAP2 MOVE3", 19) == 0)
		{
			char *p = string + 19 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
			timercomputerincrement += increment;
			show_dialog_swap_query2(windowmain);
		}
		else if (strncmp(string, "MESSAGE SWAP2 SWAP1 NO", 22) == 0)
		{
			swap2done = 1;

			isthinking = 1;
			clock_timer_change_status(1);
			isneedrestart = 0;
			sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
			send_command(command);
			if (hashautoclear)
				send_command("yxhashclear\n");
			sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
			send_command(command);
			sprintf(command, "board\n");
			send_command(command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
				send_command(command);
			}
			sprintf(command, "done\n");
			send_command(command);
		}
		else if (strncmp(string, "MESSAGE SWAP2 SWAP1 YES", 23) == 0)
		{
			swap2done = 1;
			isthinking = 0;
			clock_timer_change_status(2);
			timercomputerincrement += increment;
			show_dialog_swap_info(windowmain);
		}
		else if (strncmp(string, "MESSAGE SWAP2 MOVE4", 19) == 0)
		{
			char *p = string + 19 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
		}
		else if (strncmp(string, "MESSAGE SWAP2 MOVE5", 19) == 0)
		{
			char *p = string + 19 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			swap2done = 1;
			isthinking = 0;
			clock_timer_change_status(2);
			timercomputerincrement += increment;
			refresh_board();
			show_dialog_swap_query(windowmain);
		}
		else if (strncmp(string, "MESSAGE SWAP2 SWAP2 YES", 23) == 0)
		{
			printf_log("Computer chooses black\n");
			isthinking = 0;
			clock_timer_change_status(2);
			timercomputerincrement += increment;
		}
		else if (strncmp(string, "MESSAGE SWAP2 SWAP2 NO", 22) == 0)
		{
			printf_log("Computer chooses white\n");
			if (computerside == 2)
			{
				change_side_menu(1, NULL);
				change_side_menu(-2, NULL);
			}
			else
			{
				change_side_menu(-1, NULL);
				change_side_menu(2, NULL);
			}
			isthinking = 1;
			clock_timer_change_status(1);
			isneedrestart = 0;
			sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
			send_command(command);
			if (hashautoclear)
				send_command("yxhashclear\n");
			sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
			send_command(command);
			sprintf(command, "board\n");
			send_command(command);
			for (i = 0; i < piecenum; i++)
			{
				sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
						movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
				send_command(command);
			}
			sprintf(command, "done\n");
			send_command(command);
		}
		else if (strncmp(string, "MESSAGE SOOSORV MOVE1", 21) == 0)
		{
			char *p = string + 21 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
		}
		else if (strncmp(string, "MESSAGE SOOSORV MOVE2", 21) == 0)
		{
			char *p = string + 21 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
		}
		else if (strncmp(string, "MESSAGE SOOSORV MOVE3", 21) == 0)
		{
			char *p = string + 21 + 1;
			sscanf(p, "%d %d", &y, &x);
			make_move(y, x);
			refresh_board();
			timercomputerincrement += increment;
			show_dialog_swap_query(windowmain);
		}
		else if (strncmp(string, "MESSAGE SOOSORV SWAP1", 21) == 0)
		{
			char *p = string + 21 + 1;
			if (*p == 'Y')
			{
				timercomputerincrement += increment;
				show_dialog_swap_info(windowmain);
			}
			else //*p == 'N'
			{
				char command[80];
				sprintf(command, "yxsoosorvstep3\n");
				send_command(command);
				for (i = 0; i < piecenum; i++)
				{
					sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
							movepath[i] % boardsizew);
					send_command(command);
				}
				send_command("done\n");
			}
		}
		else if (strncmp(string, "MESSAGE SOOSORV MOVE4", 21) == 0)
		{
			char *p = string + 21 + 1;
			sscanf(p, "%d %d %d", &y, &x, &move5N);
			make_move(y, x);
			refresh_board();
			if (!refreshboardflag2)
				refreshboardflag = 1;
			timercomputerincrement += increment;
			show_dialog_swap_query(windowmain);
		}
		else if (strncmp(string, "MESSAGE SOOSORV SWAP2", 21) == 0)
		{
			char *p = string + 21 + 1;
			if (*p == 'Y')
			{
				timercomputerincrement += increment;
				show_dialog_swap_info(windowmain);
			}
			else //*p == 'N'
			{
				char command[80];
				sprintf(command, "yxsoosorvstep5 %d\n", move5N);
				send_command(command);
				for (i = 0; i < piecenum; i++)
				{
					sprintf(command, "%d,%d\n", movepath[i] / boardsizew,
							movepath[i] % boardsizew);
					send_command(command);
				}
				send_command("done\n");
			}
		}
		else if (strncmp(string, "MESSAGE SOOSORV MOVE5", 21) == 0)
		{
			char *p = string + 21;
			if (*p == 'C')
			{
				sscanf(p + 2, "%d %d", &y, &x);
				while (piecenum >= 5)
				{
					change_piece(windowmain, 1);
				}
				make_move(y, x);
				refresh_board();

				isthinking = 1;
				clock_timer_change_status(1);
				isneedrestart = 0;
				sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
				send_command(command);
				if (hashautoclear)
					send_command("yxhashclear\n");
				sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
				send_command(command);
				sprintf(command, "board\n");
				send_command(command);
				for (i = 0; i < piecenum; i++)
				{
					sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
							movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
					send_command(command);
				}
				sprintf(command, "done\n");
				send_command(command);
			}
			else
			{
				p += 1;
				if (*p == 'R') //"REFRESH"
				{
					refreshboardflag = 1;
				}
				else if (*p == 'D') //"DONE"
				{
					timercomputerincrement += increment;
					refresh_board();
					refreshboardflag2 = 1;
					if (move5N == 1)
						refreshboardflag = 0;
				}
				else
				{
					sscanf(p, "%d %d", &y, &x);
					make_move(y, x);
				}
			}
		}
		else if (strncmp(string, "MESSAGE DATABASE", 16) == 0)
		{
			char *p = string + 16 + 1;
			char *rawp = rawstring + 16 + 1;
			if (*p == 'R') //"REFRESH"
			{
				clear_board_tag();
			}
			else if (*p == 'D') //"DONE"
			{
				refresh_board();
			}
			else if (*p == 'O') //"ONE"
			{
				int tag, val, depth, bound;
				char displayLabel[80];
				sprintf(displayLabel, "");
				sscanf(p + 4, "%d %d %d %d %79s", &tag, &val, &depth, &bound, displayLabel);
				if (strlen(displayLabel) > 0)
					printf_log("%s: ", displayLabel);
				if (tag <= 0)
					printf_log("tag=(null)  ");
				else
					printf_log("tag=%c  ", tag);
				const char *boundtext = bound == 3 ? "exact" : bound == 2 ? "lower"
														   : bound == 1	  ? "upper"
																		  : "none";
				printf_log("val=%d  depth=%d  bound=%s\n", val, depth, boundtext);
			}
			else if (*p == 'T') //"TEXT"
			{
				GtkTextIter start, end;
				gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextdbcomment), &start, &end);
				gtk_text_buffer_delete(buffertextdbcomment, &start, &end);

				char *text_utf8 = _T(rawp + 5), *t = text_utf8;
				if (*t == '"')
				{
					++t;
					int len = 0;
					while (t[len] != '"')
					{
						if (t[len] == '\0')
						{
							gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextdbcomment), &end, t, len);
							g_free(rawstring);
							g_free(text_utf8);
							g_io_channel_read_line(channel, &rawstring, &size, NULL, NULL);
							t = text_utf8 = _T(rawstring);
							len = 0;
							continue;
						}
						else if (t[len] == '\\')
						{
							gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextdbcomment), &end, t, len);
							if (t[len + 1] == '"')
								gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextdbcomment), &end, "\"", 1);
							else if (t[len + 1] == '\\')
								gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextdbcomment), &end, "\\", 1);
							t += len + 2;
							len = 0;
							continue;
						}
						len++;
					}
					if (len)
						gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextdbcomment), &end, t, len);
				}
				g_free(text_utf8);
			}
			else if (*p == 'L') //"LOAD"
			{
				p += 5;
				if (strncmp(p, "START", 5) == 0)
				{
					char *filename = _T(rawp + 11);
					char *title = g_strdup_printf("Yixin (%.*s)", (int)strcspn(filename, "\n") - 1, filename);
					gtk_window_set_title(GTK_WINDOW(windowmain), title);
					g_free(title);
					if (loadingdialog == NULL)
					{
						loadingdialog = gtk_message_dialog_new(GTK_WINDOW(windowmain), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
															   GTK_BUTTONS_NONE, "%s %s ......", language == 0 ? "Loading database from" : _T(clanguage[120]), _T(rawp + 11));
						gtk_window_set_deletable(GTK_WINDOW(loadingdialog), FALSE);
						gtk_widget_show(GTK_DIALOG(loadingdialog));
					}
				}
				else if (strncmp(p, "DONE", 4) == 0)
				{
					if (loadingdialog)
					{
						gtk_widget_destroy(loadingdialog);
						loadingdialog = NULL;
					}
				}
			}
			else if (*p == 'S') //"SAVE"
			{
				p += 5;
				if (strncmp(p, "START", 5) == 0)
				{
					gtk_window_set_deletable(GTK_WINDOW(windowmain), FALSE);
					if (savingdialog == NULL)
					{
						savingdialog = gtk_message_dialog_new(GTK_WINDOW(windowmain), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
															  GTK_BUTTONS_NONE, "%s %s ......", language == 0 ? "Saving database to" : _T(clanguage[115]), _T(rawp + 11));
						gtk_window_set_deletable(GTK_WINDOW(savingdialog), FALSE);
						gtk_widget_show(GTK_DIALOG(savingdialog));
					}
				}
				else if (strncmp(p, "DONE", 4) == 0)
				{
					gtk_window_set_deletable(GTK_WINDOW(windowmain), TRUE);
					if (savingdialog)
					{
						gtk_widget_destroy(savingdialog);
						savingdialog = NULL;
					}
				}
			}
			else
			{
				int tag, offset;
				sscanf(p, "%d %d %d %*d %*d %*d %*d %n", &y, &x, &tag, &offset);
				boardtag[y][x] = tag;
				sscanf(rawp + offset, "%6s", boardtext[y][x]);
				boardtagclear = 0;
			}
		}
		else if (strncmp(string, "MESSAGE REALTIME", 16) == 0)
		{
			clear_board_tag();
			char *p = string + 16 + 1;
			if (*p == 'B') //"BEST"
			{
				int oldX = boardbestX, oldY = boardbestY;
				p += 5;
				sscanf(p, "%d,%d", &boardbestY, &boardbestX);
				if (oldX >= 0 && oldY >= 0)
					refresh_board_area(oldX, oldY, oldX + 1, oldY + 1);
				refresh_board_area(boardbestX, boardbestY, boardbestX + 1, boardbestY + 1);
			}
			else if (*p == 'V') //"VAL"
			{
				p += 4;
				sscanf(p, "%d", &bestval);
			}
			else if (*p == 'L') //"LOSE"
			{
				p += 5;
				sscanf(p, "%d,%d", &y, &x);
				boardlose[y][x] = 1;
				refresh_board_area(x, y, x + 1, y + 1);
			}
			else if (*p == 'P' && *(p + 1) == 'V') //"PV"
			{
				p += 3;
				strcpy(bestline, p);
			}
			else if (*p == 'P' && *(p + 1) == 'O') //"POS"
			{
				p += 4;
				sscanf(p, "%d,%d", &y, &x);
				boardpos[y][x] = 2;
				refresh_board_area(x, y, x + 1, y + 1);
			}
			else if (*p == 'R') //"REFRESH"
			{
				for (int y = 0; y < boardsizeh; y++)
					for (int x = 0; x < boardsizew; x++)
					{
						if (boardpos[y][x])
						{
							boardpos[y][x] = 0;
							refresh_board_area(x, y, x + 1, y + 1);
						}
					}
			}
			else if (*p == 'D') //"DONE"
			{
				p += 5;
				sscanf(p, "%d,%d", &y, &x);
				if (boardpos[y][x] == 2)
					boardpos[y][x] = 1;
				refresh_board_area(x, y, x + 1, y + 1);
			}
		}
		else if (strncmp(string, "MESSAGE INFO", 12) == 0)
		{
			char *p = string + 12 + 1;
			if (strncmp(p, "MAX_THREAD_NUM", 14) == 0) // MAX_THREAD_NUM
			{
				p += 15;
				sscanf(p, "%d", &maxthreadnum);
			}
			else if (strncmp(p, "MAX_HASH_SIZE", 13) == 0) // MAX_HASH_SIZE
			{
				p += 14;
				int maxhashsize;
				sscanf(p, "%d", &maxhashsize);
				maxhashsizemb = maxhashsize <= 10 ? 1 : (1 << (maxhashsize - 10));
			}
		}
		else if (strncmp(string, "MESSAGE", 7) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "DETAIL", 6) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "DEBUG", 5) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "ERROR", 5) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "UNKNOWN", 7) == 0)
		{
			printf_log("%s", string);
		}
		else if (strncmp(string, "FORBID", 6) == 0)
		{
			memset(forbid, 0, sizeof(forbid));
			for (i = 7; string[i] != '.'; i += 4)
			{
				y = (string[i] - '0') * 10 + string[i + 1] - '0';
				x = (string[i + 2] - '0') * 10 + string[i + 3] - '0';
				forbid[y][x] = 1;
			}
			// printf_log("%s", string);
			refresh_board();
		}
		else
		{
			y = x = y2 = x2 = -999;
			sscanf(string, "%d,%d %d,%d", &y, &x, &y2, &x2);
			if (y != -999 && x != -999)
			{
				if (isneedomit > 0)
				{
					isneedomit--;
				}
				else
				{
					timercomputerincrement += increment;
					isthinking = 0;
					clock_timer_change_status(2);

					printf_log("\n");
					if (blockautoreset)
					{
						send_command("yxblockreset\n");
						memset(boardblock, 0, sizeof(boardblock));
						refresh_board();
					}
					if (blockpathautoreset)
					{
						send_command("yxblockpathreset\n");
					}
					if (is_legal_move(y, x))
					{
						make_move(y, x);
						if (y2 != -999 && x2 != -999 && is_legal_move(y2, x2))
							make_move(y2, x2);
						show_database();
					}
					else
					{
						isgameover = 1;
					}
					if (inforule == 2 && (computerside & 1) == 0)
					{
						sprintf(command, "yxshowforbid\n");
						send_command(command);
					}

					if (!isgameover && (((computerside & 1) && piecenum % 2 == 0) || ((computerside & 2) && piecenum % 2 == 1)))
					{
						isthinking = 1;
						clock_timer_change_status(1);
						sprintf(command, "INFO time_left %lld\n", timeoutmatch - timercomputermatch + timercomputerincrement);
						send_command(command);
						if (hashautoclear)
							send_command("yxhashclear\n");
						sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
						send_command(command);
						sprintf(command, "board\n");
						send_command(command);
						for (i = 0; i < piecenum; i++)
						{
							sprintf(command, "%d,%d,%d\n", movepath[i] / boardsizew,
									movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
							send_command(command);
						}
						sprintf(command, "done\n");
						send_command(command);
					}
				}
			}
		}

		g_free(string);
		g_free(rawstring);
	} while (g_io_channel_get_buffer_condition(channel) == G_IO_IN);
	return TRUE; /* TRUE means that the recalling function still runs later, while FALSE indicates it no longer runs */
}
gboolean iochannelerr_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	if (cond & G_IO_HUP)
	{
		g_io_channel_unref(channel);
		return FALSE;
	}
	return TRUE;
}
int64_t read_int_from_file(FILE *in)
{
	int64_t num = 0;
	int flag = 0;
	char c;

	while (fscanf(in, "%c", &c) != EOF)
	{
		if (flag & 1)
		{
			if (c == '\n')
				flag &= ~1;
		}
		else
		{
			if (c == ';')
			{
				flag |= 1;
				if (flag & 2)
					return (flag & 4) ? -num : num;
			}
			else if (c <= '9' && c >= '0')
			{
				num *= 10;
				num += c - '0';
				flag |= 2;
			}
			else if (c == '-')
			{
				num = 0;
				flag |= 2;
				flag |= 4;
			}
			else
			{
				if (flag & 2)
					return (flag & 4) ? -num : num;
			}
		}
	}
	return (flag & 4) ? -num : num;
}
void load_setting(int def_boardsizeh, int def_boardsizew, int def_language, int def_toolbar)
{
	FILE *in;
	char s[1024];
	int t;
	int i;

	if ((in = fopen("settings.txt", "r")) != NULL)
	{
		boardsizeh = read_int_from_file(in);
		if (def_boardsizeh >= 5 && def_boardsizeh <= MAX_SIZE)
		{
			boardsizeh = def_boardsizeh;
		}
		if (boardsizeh > MAX_SIZE || boardsizeh < 5)
			boardsizeh = 15;
		boardsizew = read_int_from_file(in);
		if (def_boardsizew >= 5 && def_boardsizew <= MAX_SIZE)
		{
			boardsizew = def_boardsizew;
		}
		if (boardsizew > MAX_SIZE || boardsizew < 5)
			boardsizew = 15;
		language = read_int_from_file(in);
		rboardsizeh = boardsizeh;
		rboardsizew = boardsizew;
		rlanguage = language;
		if (def_language >= 0 && def_language <= 1)
			language = def_language;
		if (language < 0)
			language = 0;
		inforule = read_int_from_file(in);
		if (inforule < 0 || inforule > 6)
			inforule = 0;
		if (inforule == 3)
		{
			inforule = 0;
			specialrule = 2;
		}
		if (inforule == 4)
		{
			inforule = 2;
			specialrule = 1;
		}
		if (inforule == 5)
		{
			inforule = 2;
			specialrule = 3;
		}
		if (inforule == 6)
		{
			inforule = 1;
			specialrule = 4;
		}
		computerside = 0;
		t = read_int_from_file(in);
		if (t == 1)
			computerside |= 1;
		t = read_int_from_file(in);
		if (t == 1)
			computerside |= 2;
		levelchoice = read_int_from_file(in);
		if (levelchoice < 0 || levelchoice > 11)
			levelchoice = 1;
		timeoutturn = read_int_from_file(in) * 1000;
		if (timeoutturn <= 0)
			timeoutturn = 1000000;
		timeoutmatch = read_int_from_file(in) * 1000;
		if (timeoutmatch <= 0)
			timeoutmatch = 1000000000;
		maxdepth = read_int_from_file(in);
		if (maxdepth < 2 || maxdepth > boardsizeh * boardsizew)
			maxdepth = boardsizeh * boardsizew;
		maxnode = read_int_from_file(in);
		if (maxnode < 1000)
			maxnode = 1000000000;
		cautionfactor = read_int_from_file(in);
		if (cautionfactor < 0 || cautionfactor > CAUTION_NUM)
			cautionfactor = 1;
		showtoolbarboth = read_int_from_file(in);
		if (def_toolbar >= 0 && def_toolbar <= 2)
			showtoolbarboth = def_toolbar;
		if (showtoolbarboth < 0 || showtoolbarboth > 2)
			showtoolbarboth = 1;
		showlog = read_int_from_file(in);
		if (showlog < 0 || showlog > 1)
			showlog = 1;
		shownumber = read_int_from_file(in);
		if (shownumber < 0 || shownumber > 1)
			shownumber = 1;
		showanalysis = read_int_from_file(in);
		if (showanalysis < 0 || showanalysis > 1)
			showanalysis = 1;
		showwarning = read_int_from_file(in);
		if (showwarning < 0 || showwarning > 1)
			showwarning = 1;
		blockautoreset = read_int_from_file(in);
		if (blockautoreset < 0 || blockautoreset > 1)
			blockautoreset = 0;
		threadnum = read_int_from_file(in);
		if (threadnum < 1 /*|| threadnum > maxthreadnum*/)
			threadnum = 1;
		hashsizemb = read_int_from_file(in);
		if (hashsizemb < 0 /*|| hashsizemb > maxhashsizemb*/)
			hashsizemb = 256;
		blockpathautoreset = read_int_from_file(in);
		if (blockpathautoreset < 0 || blockpathautoreset > 1)
			blockpathautoreset = 0;
		infopondering = read_int_from_file(in);
		if (infopondering < 0 || infopondering > 1)
			infopondering = 0;
		infovcthread = read_int_from_file(in);
		if (infovcthread < 0 || infovcthread > 2)
			infovcthread = 0;
		hashautoclear = read_int_from_file(in);
		if (hashautoclear < 0 || hashautoclear > 1)
			hashautoclear = 0;
		toolbarpos = read_int_from_file(in);
		if (toolbarpos < 0 || toolbarpos > 1)
			toolbarpos = 1;
		showclock = read_int_from_file(in);
		if (showclock < 0 || showclock > 1)
			showclock = 1;
		increment = read_int_from_file(in);
		if (increment < 0 || increment > 200000)
			increment = 0;
		showforbidden = read_int_from_file(in);
		if (showforbidden < 0 || showforbidden > 1)
			showforbidden = 1;
		checktimeout = read_int_from_file(in);
		if (checktimeout < 0 || checktimeout > 1)
			checktimeout = 1;
		usedatabase = read_int_from_file(in);
		if (usedatabase < 0 || usedatabase > 1)
			usedatabase = 1;
		databasereadonly = read_int_from_file(in);
		if (databasereadonly < 0 || databasereadonly > 1)
			databasereadonly = 0;
		showboardtext = read_int_from_file(in);
		if (showboardtext < 0 || showboardtext > 1)
			showboardtext = 1;
		showdbdelconfirm = read_int_from_file(in);
		if (showdbdelconfirm < 0 || showdbdelconfirm > 1)
			showdbdelconfirm = 1;
		recorddebuglog = read_int_from_file(in);
		if (recorddebuglog < 0 || recorddebuglog > 1)
			recorddebuglog = 0;
		t = read_int_from_file(in);
		if (t > 0 && t < 1000)
			hdpiscale = t / 100.0;
		t = read_int_from_file(in);
		if (t >= 0 && t <= 1)
			nbestsym = t;
		losssaturation = read_int_from_file(in);
		if (losssaturation < 0 || losssaturation > 100)
			losssaturation = 0;
		winsaturation = read_int_from_file(in);
		if (winsaturation < 0 || winsaturation > 100)
			winsaturation = 0;
		minsaturation = read_int_from_file(in);
		if (minsaturation < 0 || minsaturation > 100)
			minsaturation = 0;
		maxsaturation = read_int_from_file(in);
		if (maxsaturation < 0 || maxsaturation > 100)
			maxsaturation = 0;
		colorvalue = read_int_from_file(in);
		if (colorvalue < 0 || colorvalue > 100)
			colorvalue = 100;
		fclose(in);
	}
	for (i = 0; i < MAX_TOOLBAR_ITEM; i++)
	{
		sprintf(s, "function/toolbar%d.txt", i + 1);
		if ((in = fopen(s, "r")) != NULL)
		{
			int j = 0;
			char icon[80];
			fscanf(in, "%d", &toolbarlng[i]);
			fscanf(in, "%s", &icon);
			toolbaricon[i] = strdup(icon);

			while (fgets(toolbarcommand[i] + j, MAX_TOOLBAR_COMMAND_LEN, in))
			{
				j += strlen(toolbarcommand[i] + j);
				while (j > 0 && (toolbarcommand[i][j - 1] == '\n' || toolbarcommand[i][j - 1] == '\r'))
					j--;
				toolbarcommand[i][j] = '\n';
				j++;
				toolbarcommand[i][j] = 0;
			}
			fclose(in);
		}
		else
		{
			toolbarnum = i;
			break;
		}
	}
	for (i = 0; i < MAX_HOTKEY_ITEM; i++)
	{
		sprintf(s, "function/hotkey%d.txt", i + 1);
		if ((in = fopen(s, "r")) != NULL)
		{
			int j = 0;
			fscanf(in, "%d", &hotkeykey[i]);

			while (fgets(hotkeycommand[i] + j, MAX_HOTKEY_COMMAND_LEN, in))
			{
				j += strlen(hotkeycommand[i] + j);
				while (j > 0 && (hotkeycommand[i][j - 1] == '\n' || hotkeycommand[i][j - 1] == '\r'))
					j--;
				hotkeycommand[i][j] = '\n';
				j++;
				hotkeycommand[i][j] = 0;
			}
			fclose(in);
		}
		else
		{
			hotkeynum = i;
			break;
		}
	}

	sprintf(s, "piece_%d.bmp", max(boardsizeh, boardsizew));
	if ((in = fopen(s, "rb")) != NULL)
	{
		strcpy(piecepicname, s);
		fclose(in);
	}
	piecenum = 0;
	memset(movepath, -1, sizeof(movepath));

	clanguage = (char *)malloc(1024 * sizeof(char *));
	for (i = 0; i < 1024; i++)
		clanguage[i] = NULL;

	sprintf(s, "language/%d.lng", language);
	if ((in = fopen(s, "r")) != NULL)
	{
		while (fgets(s, sizeof(s), in))
		{
			int l = strlen(s);
			int p;
			while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r'))
			{
				s[l - 1] = 0;
				l--;
			}
			if (l == 0)
				continue;
			if (s[0] == ';')
				continue;
			sscanf(s, "%d", &p);
			for (i = 0; i < l && s[i] != '='; i++)
				;
			clanguage[p] = strdup(s + i + 1);
		}
		fclose(in);
	}
	else
	{
		language = 0;
	}

	if ((in = fopen("settings_font.txt", "r")) != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			fgets(s, sizeof(s), in);
			s[strcspn(s, "\n")] = 0;
			char *semicolon = strchr(s, ';');
			if (semicolon)
				*semicolon = '\0';
			strcpy(fontname[i], s);
		}
	}
}
static void childexit_watch(GPid pid, gint status, gpointer *data)
{
#ifdef G_OS_WIN32
	gchar *argv[] = {"Yixin.exe", NULL};
#else
	gchar *argv[] = {"./Yixin", NULL};
#endif
	g_spawn_close_pid(pid);

	save_setting();
	if (clanguage != NULL)
	{
		int i;
		for (i = 0; i < 1024; i++)
		{
			if (clanguage[i] != NULL)
				free(clanguage[i]);
		}
		free(clanguage);
	}
	if (debuglog != NULL)
		fclose(debuglog);
	if (respawn)
		g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, NULL, NULL);
	gtk_main_quit();
}

void load_engine()
{
#ifdef G_OS_WIN32
	gchar *argv[] = {"engine.exe", NULL};
#else
	gchar *argv[] = {"./engine", NULL};
#endif
	GPid pid;
	gint in, out, err;
	gboolean ret;
	GError *error = NULL;

	ret = g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
								   NULL, NULL, &pid, &in, &out, &err, &error);
	if (!ret)
	{
		FILE *out;
		out = fopen("ERROR.txt", "w");
		fprintf(out, "%s\n", error->message);
		fclose(out);
		g_error_free(error);

		g_error("Cannot load engine!");
		return;
	}
#ifdef G_OS_WIN32
	iochannelin = g_io_channel_win32_new_fd(in);
	iochannelout = g_io_channel_win32_new_fd(out);
	iochannelerr = g_io_channel_win32_new_fd(err);
#else
	iochannelin = g_io_channel_unix_new(in);
	iochannelout = g_io_channel_unix_new(out);
	iochannelerr = g_io_channel_unix_new(err);
#endif

	// use binary encoding for pipes
	g_io_channel_set_encoding(iochannelin, NULL, &error);
	g_io_channel_set_encoding(iochannelout, NULL, &error);
	g_io_channel_set_encoding(iochannelerr, NULL, &error);

	g_child_watch_add(pid, (GChildWatchFunc)childexit_watch, NULL);
	g_io_add_watch(iochannelout, G_IO_IN | G_IO_PRI | G_IO_HUP, (GIOFunc)iochannelout_watch, NULL);
	// g_io_add_watch_full(iochannelout, G_PRIORITY_HIGH, G_IO_IN | G_IO_HUP, (GIOFunc)iochannelout_watch, NULL, NULL);
	g_io_add_watch(iochannelerr, G_IO_IN | G_IO_PRI | G_IO_HUP, (GIOFunc)iochannelerr_watch, NULL);

	if (recorddebuglog)
	{
		debuglog = fopen("debuglog.txt", "at+");
		if (debuglog != NULL)
		{
			fprintf(debuglog, "----------new record----------\n");
		}
	}
}
void init_engine()
{
	char command[80];
	send_command("info show_detail 1\n");
	send_command("yxshowinfo\n");
	sprintf(command, "START %d %d\n", boardsizew, boardsizeh);
	send_command(command);
	set_level(levelchoice);
	set_cautionfactor(cautionfactor);
	set_threadnum(threadnum);
	set_hashsize(hashsizemb);
	set_pondering(infopondering);
	setvcthread(infovcthread);
}
int main(int argc, char **argv)
{
	static GOptionEntry options[] =
		{
			{"size", 's', 0, G_OPTION_ARG_INT, NULL,
			 "Board size to use", "Integer"},
			{"lang", 'l', 0, G_OPTION_ARG_INT, NULL,
			 "language", "Integer"},
			{"toolbar", '\0', 0, G_OPTION_ARG_INT, NULL,
			 "Tool bar style", "Integer"},
			{NULL},
		};
	GError *error = NULL;
	gint boardsizeh = -1;
	gint boardsizew = -1;
	gint language = -1;
	gint toolbar = -1;

	options[0].arg_data = &boardsizeh;
	options[1].arg_data = &boardsizew;
	options[2].arg_data = &language;
	options[3].arg_data = &toolbar;

	gtk_init_with_args(&argc, &argv, NULL, options, NULL, &error);
	srand((unsigned)time(NULL));
	load_setting(boardsizeh, boardsizew, language, toolbar);
	load_engine();
	init_engine();
	gtk_window_set_default_icon(gdk_pixbuf_new_from_file("icon.ico", NULL)); /* set the default icon for all windows */
	create_windowclock();
	create_windowmain();
	show_welcome();
	show_database();
	gtk_main();
	return 0;
}
