/* hello-world - example daemon for the Infix spin
 *
 * Greets an audience in the system log once a minute.  Who to greet is
 * configuration, and how many times it has greeted them is operational
 * state -- both described by the example-hello YANG model shipped by
 * this spin, see yang/example-hello@2026-08-26.yang.
 *
 * The daemon talks to sysrepo, the datastore behind Infix's CLI,
 * NETCONF, and RESTCONF interfaces.  Two subscriptions is all it takes:
 * one to be told when the configuration changes, one to answer when
 * someone asks for the state.  Nothing in the CLI knows about this
 * daemon; it picks the model up from sysrepo on its own.
 *
 * Runs in the foreground, as finit expects, which handles supervision
 * and restart.
 */
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <libyang/libyang.h>
#include <sysrepo.h>

#define MODULE		"example-hello"
#define XPATH_HELLO	"/example-hello:hello"
#define XPATH_AUDIENCE	XPATH_HELLO "/audience"
#define XPATH_GREETINGS	XPATH_HELLO "/greetings"
#define INTERVAL	60

static volatile sig_atomic_t running = 1;
static char     audience[64] = "world";
static uint64_t greetings;

static void sig_exit(int signo)
{
	running = 0;
}

/*
 * Read the audience from the running datastore.  The leaf has a default
 * in the YANG model, so this returns "world" on a system that has never
 * been configured -- no fallback needed here.
 */
static void read_audience(sr_session_ctx_t *sess)
{
	sr_val_t *val = NULL;
	int rc;

	rc = sr_get_item(sess, XPATH_AUDIENCE, 0, &val);
	if (rc || !val) {
		syslog(LOG_WARNING, "cannot read %s (%s), still greeting '%s'",
		       XPATH_AUDIENCE, sr_strerror(rc), audience);
		return;
	}

	snprintf(audience, sizeof(audience), "%s", val->data.string_val);
	sr_free_val(val);

	syslog(LOG_NOTICE, "greeting '%s' from now on", audience);
}

/*
 * Called once a 'set hello audience ...' has been committed, i.e., after
 * 'leave' in the CLI or a NETCONF <commit>.
 */
static int change_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *module,
		     const char *xpath, sr_event_t event, uint32_t request_id, void *priv)
{
	if (event == SR_EV_DONE)
		read_audience(priv);

	return SR_ERR_OK;
}

/*
 * Called when someone reads operational data, e.g., 'show hello'.  We
 * hand back a data tree with the counter in it; sysrepo merges it into
 * whatever the caller asked for.
 */
static int oper_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *module,
		   const char *path, const char *request_xpath, uint32_t request_id,
		   struct lyd_node **parent, void *priv)
{
	const struct ly_ctx *ctx;
	sr_conn_ctx_t *conn;
	char buf[32];

	conn = sr_session_get_connection(session);
	ctx = sr_acquire_context(conn);

	snprintf(buf, sizeof(buf), "%llu", (unsigned long long)greetings);
	if (!*parent)
		lyd_new_path(NULL, ctx, XPATH_HELLO, NULL, 0, parent);
	lyd_new_path(*parent, NULL, XPATH_GREETINGS, buf, 0, NULL);

	sr_release_context(conn);

	return SR_ERR_OK;
}

int main(void)
{
	sr_subscription_ctx_t *sub = NULL;
	sr_session_ctx_t *sess;
	sr_conn_ctx_t *conn;
	int rc;

	signal(SIGTERM, sig_exit);
	signal(SIGINT, sig_exit);

	openlog("hello-world", LOG_PID, LOG_DAEMON);

	rc = sr_connect(0, &conn);
	if (rc) {
		syslog(LOG_ERR, "cannot connect to sysrepo: %s", sr_strerror(rc));
		return 1;
	}

	rc = sr_session_start(conn, SR_DS_RUNNING, &sess);
	if (rc) {
		syslog(LOG_ERR, "cannot start sysrepo session: %s", sr_strerror(rc));
		return 1;
	}

	read_audience(sess);

	/* Tell me when the configuration changes ... */
	rc = sr_module_change_subscribe(sess, MODULE, XPATH_HELLO, change_cb, sess,
					0, SR_SUBSCR_DONE_ONLY, &sub);
	if (rc) {
		syslog(LOG_ERR, "cannot subscribe to %s: %s", XPATH_HELLO, sr_strerror(rc));
		return 1;
	}

	/* ... and ask me when someone wants the state. */
	rc = sr_oper_get_subscribe(sess, MODULE, XPATH_GREETINGS, oper_cb, NULL, 0, &sub);
	if (rc) {
		syslog(LOG_ERR, "cannot subscribe to %s: %s", XPATH_GREETINGS, sr_strerror(rc));
		return 1;
	}

	while (running) {
		syslog(LOG_INFO, "Hello %s!", audience);
		greetings++;
		sleep(INTERVAL);
	}

	syslog(LOG_INFO, "exiting");
	sr_unsubscribe(sub);
	sr_disconnect(conn);
	closelog();

	return 0;
}
