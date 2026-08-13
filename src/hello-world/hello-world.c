/* hello-world - example daemon for the Infix spin
 *
 * Periodically writes a greeting to the system log.  Runs in the
 * foreground, as finit expects, which handles supervision and restart.
 */
#include <signal.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#define CONFIG_FILE	"/etc/hello-world.conf"
#define INTERVAL	60

static volatile sig_atomic_t running = 1;

static void sig_exit(int signo)
{
	running = 0;
}

/*
 * The configuration file is shipped by the rootfs overlay, see
 * board/common/rootfs/etc/ in the spin.  A real daemon would read its
 * settings here; this example only verifies the file is in place.
 */
static int load_config(void)
{
	FILE *fp;

	fp = fopen(CONFIG_FILE, "r");
	if (!fp) {
		syslog(LOG_WARNING, "missing %s, using defaults", CONFIG_FILE);
		return -1;
	}

	fclose(fp);
	return 0;
}

int main(void)
{
	signal(SIGTERM, sig_exit);
	signal(SIGINT, sig_exit);

	openlog("hello-world", LOG_PID, LOG_DAEMON);
	load_config();

	while (running) {
		syslog(LOG_INFO, "Hello World!");
		sleep(INTERVAL);
	}

	syslog(LOG_INFO, "exiting");
	closelog();

	return 0;
}
