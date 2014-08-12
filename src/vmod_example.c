#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>

/* Typical include path would be <librdkafka/rdkafka.h>, but this program
 * is builtin from within the librdkafka source tree and thus differs. */
#include "rdkafka.h"  /* for Kafka driver */

#include "vrt.h"
#include "cache/cache.h"

#include "vcc_if.h"

static int run = 1;
static rd_kafka_t *rk;
static int exit_eof = 0;
static int quiet = 0;
static 	enum {
	OUTPUT_HEXDUMP,
	OUTPUT_RAW,
} output = OUTPUT_HEXDUMP;

rd_kafka_conf_t *conf;
rd_kafka_topic_conf_t *topic_conf;


int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	/* Kafka configuration */
        conf = rd_kafka_conf_new();

        /* Topic configuration */
        topic_conf = rd_kafka_topic_conf_new();

	return (0);
}

VCL_STRING
vmod_hello(const struct vrt_ctx *ctx, VCL_STRING name)
{
	char *p;
	unsigned u, v;
	char errstr[512];
        char *brokers = "localhost:9092";
        char *topic = "fred";

	rd_kafka_topic_t *rkt;
	
	int partition = RD_KAFKA_PARTITION_UA;

	u = WS_Reserve(ctx->ws, 0); /* Reserve some work space */
	p = ctx->ws->f;		/* Front of workspace area */
	v = snprintf(p, u, "Hello, %s", name);
	v++;
	if (v > u) {
		/* No space, reset and leave */
		WS_Release(ctx->ws, 0);
		return (NULL);
	}

	fprintf(stderr, "FLB avant release");

	/* Update work space with what we've used */
	WS_Release(ctx->ws, v);

		/*
		 * Producer
		 */
		//char buf[2048];
		//char *buf= "essai de test du Test msg sur topic fred";
		char *buf= name;
		int sendcnt = 0;

		/* Set up a message delivery report callback.
		 * It will be called once for each message, either on successful
		 * delivery to broker, or upon failure to deliver to broker. */

//                rd_kafka_conf_set_dr_cb(conf, msg_delivered);

		/* Create Kafka handle */
		if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf,
					errstr, sizeof(errstr)))) {
			fprintf(stderr,
				"%% Failed to create new producer: %s\n",
				errstr);
			exit(1);
		}
		else
		{	
			fprintf(stderr, "FLB success producer");

		}
		/* Set logger */
		//rd_kafka_set_logger(rk, logger);
		//rd_kafka_set_log_level(rk, LOG_DEBUG);

		/* Add brokers */
		if (rd_kafka_brokers_add(rk, brokers) == 0) {
			fprintf(stderr, "%% No valid brokers specified\n");
			exit(1);
		}
		else
		{
			fprintf(stderr, "FLB success broker");
		}
		/* Create topic */
		rkt = rd_kafka_topic_new(rk, topic, topic_conf);


			size_t len = strlen(buf);
			if (buf[len-1] == '\n')
				buf[--len] = '\0';

			/* Send/Produce message. */
			if (rd_kafka_produce(rkt, partition,
					     RD_KAFKA_MSG_F_COPY,
					     /* Payload and length */
					     buf, len,
					     /* Optional key and its length */
					     NULL, 0,
					     /* Message opaque, provided in
					      * delivery report callback as
					      * msg_opaque. */
					     NULL) == -1) {
				fprintf(stderr,
					"%% Failed to produce to topic %s "
					"partition %i: %s\n",
					rd_kafka_topic_name(rkt), partition,
					rd_kafka_err2str(
						rd_kafka_errno2err(errno)));
				/* Poll to handle delivery reports */
				rd_kafka_poll(rk, 0);
				exit(2);

			}

			if (!quiet)
				fprintf(stderr, "%% Sent %zd bytes to topic "
					"%s partition %i\n",
				len, rd_kafka_topic_name(rkt), partition);
			sendcnt++;
			/* Poll to handle delivery reports */
			rd_kafka_poll(rk, 0);

		 fprintf(stderr, "FLB apres poll");
		/* Poll to handle delivery reports */
		rd_kafka_poll(rk, 0);

		/* Wait for messages to be delivered */
		while (run && rd_kafka_outq_len(rk) > 0)
			rd_kafka_poll(rk, 100);

		/* Destroy topic */
		rd_kafka_topic_destroy(rkt);

		/* Destroy the handle */
		rd_kafka_destroy(rk);
		fprintf(stderr, "FLB apres destroy");
		

	return (p);
}
