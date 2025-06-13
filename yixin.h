﻿#include <cairo.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>

void send_command(const char *command);
void make_move(int y, int x);
gboolean on_button_press_windowmain(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge);
int is_integer(const char *str);
void show_dialog_settings(GtkWidget *widget, gpointer data);
void show_dialog_load(GtkWidget *widget, gpointer data);
void show_dialog_save(GtkWidget *widget, gpointer data);
void show_dialog_about(GtkWidget *widget, gpointer data);
void show_dialog_move5N(GtkWidget *widget, gpointer data);
void new_game(GtkWidget *widget, gpointer data);
void change_rule(GtkWidget *widget, gpointer data);
void change_side(GtkWidget *widget, gpointer data);
void change_side_menu(int flag, GtkWidget *w);
void clock_timer_change_status(int status);
void clock_timer_init();
void execute_command(gchar *command);
gint sleep_timeout(gpointer data);
void change_piece(GtkWidget *widget, gpointer data);
void stop_thinking(GtkWidget *widget, gpointer data);
GdkPixbuf * copy_subpixbuf(GdkPixbuf *_src, int src_x, int src_y, int width, int height);
void create_windowmain();
gboolean iochannelout_watch(GIOChannel *channel, GIOCondition cond, gpointer data);
gboolean iochannelerr_watch(GIOChannel *channel, GIOCondition cond, gpointer data);
void load_setting(int def_boardsize, int def_language, int def_toolbar);
void save_setting();
void load_engine();
void init_engine();
void yixin_quit();
#define max(x, y) ((x)>(y)?(x):(y))
#define min(x, y) ((x)<(y)?(x):(y))
#define VERSION "2.1"
