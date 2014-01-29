/* See LICENSE file for copyright and license details.
 */

#undef GTK_DISABLE_SINGLE_INCLUDES

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <include/capi/cef_app_capi.h>

#include "arg.h"

char *argv0;

enum { AtomFind, AtomGo, AtomUri, AtomLast };

typedef union Arg Arg;
union Arg {
	gboolean b;
	gint i;
	const void *v;
};

typedef struct Client {
	GtkWidget *win, *vbox;
	char *title;
	struct Client *next;
	const char *uri;
} Client;

static Display *dpy;
static Atom atoms[AtomLast];
static Client *clients = NULL;
static gboolean showxid = FALSE;
static GdkNativeWindow embed = 0;

static void cleanup(void);
static void destroyclient(Client *c);
static void destroywin(GtkWidget *w, Client *c);
static void die(const char *errstr, ...);
static void loaduri(Client *c, Arg *arg);
static Client * newclient(void);
static void setup(void);
static void sigchld(int unused);
static void usage(void);

void
cleanup(void) {
	while(clients)
		destroyclient(clients);
}

void
destroyclient(Client *c) {
	Client *p;

	gtk_widget_destroy(c->win);

	for(p = clients; p && p->next != c; p = p->next);
	if(p) {
		p->next = c->next;
	} else {
		clients = c->next;
	}
	free(c);
	if(clients == NULL)
		gtk_main_quit();
}

void
destroywin(GtkWidget *w, Client *c) {
	destroyclient(c);
}

void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void
loaduri(Client *c, Arg *arg) {
}

Client *
newclient(void) {
	Client *c;
	GdkGeometry hints = { 1, 1 };

	if(!(c = calloc(1, sizeof(Client))))
		die("Cannot malloc!\n");

	/* WIndow */
	if(embed) {
		c->win = gtk_plug_new(embed);
	} else {
		c->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_wmclass(GTK_WINDOW(c->win), "turf", "Turf");
		gtk_window_set_role(GTK_WINDOW(c->win), "Turf");
	}
	gtk_window_set_default_size(GTK_WINDOW(c->win), 800, 600);
	g_signal_connect(G_OBJECT(c->win),
			"destroy",
			G_CALLBACK(destroywin), c);

	c->vbox = gtk_vbox_new(FALSE, 0);

	/* cef init */
	cef_settings_t *settings;
	if(!(settings = malloc(sizeof(cef_settings_t))))
		die("Cannot malloc\n");

	cef_initialize(NULL, settings, NULL);

	/* cef set as child (vbox) */

	gtk_container_add(GTK_CONTAINER(c->win), c->vbox);


	gtk_widget_show_all(GTK_WIDGET(c->win));
	gtk_window_set_geometry_hints(GTK_WINDOW(c->win), NULL, &hints,
			GDK_HINT_MIN_SIZE);
	gdk_window_set_events(GTK_WIDGET(c->win)->window, GDK_ALL_EVENTS_MASK);
	/* process X events for window - aka AtomFind/AtomGo
	 * gdk_window_add_filter(GTK_WIDGET(c->win)->window, processx, c);
	 * setatom(c, AtomFind, "");
	 * setatom(c, AtomUri, "about:blank");
	 */

	c->title = NULL;
	c->next = clients;
	clients = c;

	if(showxid) {
		gdk_display_sync(gtk_widget_get_display(c->win));
		printf("%u\n",
			(guint)GDK_WINDOW_XID(GTK_WIDGET(c->win)->window));
		fflush(NULL);
		if (fclose(stdout) != 0) {
			die("Error closing stdout");
		}
	}

	return c;
}

void
setup(void) {
	sigchld(0);

	gtk_init(NULL, NULL);
	gtk_gl_init(NULL, NULL);

	dpy = GDK_DISPLAY();

	/* atoms */
	atoms[AtomFind] = XInternAtom(dpy, "_TURF_FIND", False);
	atoms[AtomGo] = XInternAtom(dpy, "_TURF_GO", False);
	atoms[AtomUri] = XInternAtom(dpy, "_TURF_URI", False);
}

void
sigchld(int unused) {
	if(signal(SIGCHLD, sigchld) == SIG_ERR)
		die("Can't install SIGCHLD handler");
	while(0 < waitpid(-1, NULL, WNOHANG));
}

void
usage(void) {
	die("usage: %s [-vx] [-e xid] [uri]\n", basename(argv0));
}

int
main(int argc, char *argv[]) {
	Arg arg;

	memset(&arg, 0, sizeof(arg));

	/* command line args */
	ARGBEGIN {
	case 'e':
		embed = strtol(EARGF(usage()), NULL, 0);
		break;
	case 'v':
		die("turf-"VERSION", ©2014 turf engineers, "
				"see LICENSE for details\n");
	case 'x':
		showxid = TRUE;
		break;
	default:
		usage();
	} ARGEND;
	if(argc > 0)
		arg.v = argv[0];

	setup();
	newclient();
	if(arg.v)
		loaduri(clients, &arg);

	gtk_main();
	cleanup();

	return EXIT_SUCCESS;
}

