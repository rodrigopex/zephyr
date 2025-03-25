/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#if !defined(__ZEPHYR__)

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#else

#include <zephyr/net/socket.h>
#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#include <zephyr/net/tls_credentials.h>
#include "ca_certificate.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#include "net_sample_common.h"

#endif /* __ZEPHYR__ */

#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

#define NET_EVENT_WIFI_MASK (NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

/* STA Mode Configuration */
#define WIFI_SSID "NETUNO"   /* Replace `SSID` with WiFi ssid. */
#define WIFI_PSK  "11311118" /* Replace `PASSWORD` with Router password. */

static struct net_if *sta_iface;
static struct wifi_connect_req_params sta_config;

/* HTTP server to connect to */
#define HTTP_HOST "google.com"
/* Port to connect to, as string */
#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
#define HTTP_PORT "443"
#else
#define HTTP_PORT "80"
#endif
/* HTTP path to request */
#define HTTP_PATH "/"

#define SSTRLEN(s) (sizeof(s) - 1)
#define CHECK(r)                                                                                   \
	{                                                                                          \
		if (r < 0) {                                                                       \
			LOG_DBG("Error: %d", (int)r);                                              \
			exit(1);                                                                   \
		}                                                                                  \
	}

#define REQUEST "GET " HTTP_PATH " HTTP/1.1\r\nHost: " HTTP_HOST "\r\n\r\n"

#define RESPONSE_BUFFER_SIZE 1024
static char response[RESPONSE_BUFFER_SIZE];

static struct net_mgmt_event_callback cb;
K_SEM_DEFINE(sem_connected_to_internet, 0, 1);

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
			       struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT: {
		LOG_DBG("Connected to %s", WIFI_SSID);
		k_sem_give(&sem_connected_to_internet);
		break;
	}
	case NET_EVENT_WIFI_DISCONNECT_RESULT: {
		LOG_DBG("Disconnected from %s", WIFI_SSID);
		break;
	}
	default:
		break;
	}
}
void dump_addrinfo(const struct addrinfo *ai)
{
	LOG_DBG("addrinfo @%p: ai_family=%d, ai_socktype=%d, ai_protocol=%d, "
		"sa_family=%d, sin_port=%x",
		ai, ai->ai_family, ai->ai_socktype, ai->ai_protocol, ai->ai_addr->sa_family,
		ntohs(((struct sockaddr_in *)ai->ai_addr)->sin_port));
}

static int connect_to_wifi(void)
{
	if (!sta_iface) {
		LOG_DBG("STA: interface no initialized");
		return -EIO;
	}

	sta_config.ssid = (const uint8_t *)WIFI_SSID;
	sta_config.ssid_length = strlen(WIFI_SSID);
	sta_config.psk = (const uint8_t *)WIFI_PSK;
	sta_config.psk_length = strlen(WIFI_PSK);
	sta_config.security = WIFI_SECURITY_TYPE_PSK;
	sta_config.channel = WIFI_CHANNEL_ANY;
	sta_config.band = WIFI_FREQ_BAND_2_4_GHZ;

	LOG_DBG("Connecting to SSID: %s", sta_config.ssid);

	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, sta_iface, &sta_config,
			   sizeof(struct wifi_connect_req_params));
	if (ret) {
		LOG_DBG("Unable to Connect to (%s)", WIFI_SSID);
	}

	return ret;
}

int https_get()
{
	static struct addrinfo hints;
	struct addrinfo *res;
	int st, sock;

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	tls_credential_add(CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate,
			   sizeof(ca_certificate));
#endif

	LOG_DBG("Preparing HTTP GET request for http://" HTTP_HOST ":" HTTP_PORT HTTP_PATH);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	st = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
	LOG_DBG("getaddrinfo status: %d", st);

	if (st != 0) {
		LOG_DBG("Unable to resolve address, quitting");
		return -ENETUNREACH;
	}

	dump_addrinfo(res);

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	sock = socket(res->ai_family, res->ai_socktype, IPPROTO_TLS_1_2);
#else
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#endif
	CHECK(sock);
	LOG_DBG("sock = %d", sock);

	struct timeval timeout_optval = {
		.tv_sec = 5,
		.tv_usec = 0,
	};
	CHECK(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout_optval, sizeof timeout_optval));

#if defined(CONFIG_NET_SOCKETS_SOCKOPT_TLS)
	sec_tag_t sec_tag_opt[] = {
		CA_CERTIFICATE_TAG,
	};
	CHECK(setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_opt, sizeof(sec_tag_opt)));

	CHECK(setsockopt(sock, SOL_TLS, TLS_HOSTNAME, HTTP_HOST, sizeof(HTTP_HOST)))
#endif
	LOG_WRN("Connecting to server...");
	CHECK(connect(sock, res->ai_addr, res->ai_addrlen));

	LOG_DBG("Connected! Sending request...");
	CHECK(send(sock, REQUEST, SSTRLEN(REQUEST), 0));

	int i = 0;
	while (1) {
		++i;
		int len = recv(sock, response, RESPONSE_BUFFER_SIZE - 1, 0);

		if (len == -1) {
			break;
		}

		if (len < -1) {
			LOG_ERR("Could not recive bytes: err=%d", len);
			break;
		}

		if (len == 0) {
			LOG_DBG(" *** #fragments=%d", i);
			break;
		}

		response[len] = 0;
		LOG_DBG("Response (fragment=%d):\n%s", i, response);
	}

	(void)close(sock);

	LOG_WRN("Close socket");

	return 0;
}

int main(void)
{
	k_sleep(K_SECONDS(5));

	net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&cb);

	/* Get STA interface in AP-STA mode. */
	sta_iface = net_if_get_wifi_sta();

	connect_to_wifi();

	LOG_DBG("Waiting to connect to wifi...");
	k_sem_take(&sem_connected_to_internet, K_FOREVER);

	while (1) {
		int err = https_get();
		if (err) {
			LOG_ERR("Could not get the https request: %d", err);
		}
		k_msleep(5000);
	}

	return 0;
}
