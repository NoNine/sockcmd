/*
 * Copyright (C) 2016   Min Chen <chen.min@whaley.cn>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "sockcmd-server"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <cutils/sockets.h>
#include <cutils/log.h>

#include <sockcmd/sockcmd.h>
#include "sockcmd-common.h"

struct sockcmd_server {
    char *sockname;
    struct sockcmd_method *method_list;
};

SOCKCMD_METHOD(sockcmd_quit, "QUIT", 0);

static int sockcmd_server_execute(struct sockcmd_server *ctx, int s, char *cmd);

int sockcmd_server_init(sockcmd_t *phandle, const char *sockname)
{
    struct sockcmd_server *ctx;

    if (sockname == NULL) {
        ALOGE("Socket name is NOT specified.\n");
        return -1;
    }

    ctx = (struct sockcmd_server *)malloc(sizeof(struct sockcmd_server));
    if (ctx == NULL) {
        ALOGE("No free memory.\n");
        return -1;
    }

    memset(ctx, 0, sizeof(struct sockcmd_server));

    ctx->sockname = strdup(sockname);
    if (ctx->sockname == NULL) {
        ALOGE("No free memory.\n");
        goto error0;
    }

    *phandle = (sockcmd_t)ctx;

    sockcmd_server_add_method(*phandle, &sockcmd_quit);

    return 0;

error0:
    free(ctx);
    return -1;
}

int sockcmd_server_term(sockcmd_t handle)
{
    struct sockcmd_server *ctx = (struct sockcmd_server *)handle;

    if (ctx == NULL) {
        return -1;
    }

    free(ctx->sockname);
    free(ctx);

    return 0;
}

int sockcmd_server_add_method(sockcmd_t handle, struct sockcmd_method *method)
{
    struct sockcmd_server *ctx = (struct sockcmd_server *)handle;
    struct sockcmd_method *p;

    if (ctx == NULL) {
        return -1;
    }

    if (method == NULL) {
        return -1;
    }

    for (p = ctx->method_list; p != NULL && p != method; p = p->next) ;

    if (p == method) {
        return 0;
    }

    method->next = ctx->method_list;
    ctx->method_list = method;

    return 0;
}

int sockcmd_server_loop(sockcmd_t handle)
{
    struct sockcmd_server *ctx = (struct sockcmd_server *)handle;
    char buf[BUFFER_MAX];
    struct sockaddr addr;
    socklen_t alen;
    int lsocket, s;
    unsigned short count;
    struct pollfd pollfds[1];
    int nr = 0;
    int quit = 0;

    if (ctx == NULL) {
        return -1;
    }

    lsocket = android_get_control_socket(ctx->sockname);
    if (lsocket < 0) {
        ALOGE("Get socket failed: %s\n", strerror(errno));
        return -1;
    }
    if (listen(lsocket, 5)) {
        ALOGE("Listen on socket failed: %s\n", strerror(errno));
        return -1;
    }
    fcntl(lsocket, F_SETFD, FD_CLOEXEC);

    for (;;) {
        alen = sizeof(addr);
        s = accept(lsocket, &addr, &alen);
        if (s < 0) {
            ALOGE("Accept failed: %s\n", strerror(errno));
            continue;
        }
        fcntl(s, F_SETFD, FD_CLOEXEC);

        ALOGI("New connection.\n");

        pollfds[0].fd = s;
        pollfds[0].events = POLLIN;
        pollfds[0].revents = 0;
        nr = TEMP_FAILURE_RETRY(poll(pollfds, 1, 250));
        if (nr <= 0) {
            ALOGE("Timeout waiting for cmd: %s\n", strerror(errno));
            goto close_conn;
        }

        if (readx(s, &count, sizeof(count))) {
            ALOGE("Read size failed.\n");
            goto close_conn;
        }
        if ((count < 1) || (count >= BUFFER_MAX)) {
            ALOGE("Invalid size %d.\n", count);
            goto close_conn;
        }
        if (readx(s, buf, count)) {
            ALOGE("Read command failed.\n");
            goto close_conn;
        }
        buf[count] = 0;

        /* "QUIT" is a magic command to notify the server to quit. */
        if (strcmp(buf, "QUIT") == 0) {
            quit = 1;
        }

        sockcmd_server_execute(ctx, s, buf);

        /* Connection teardown. */
        pollfds[0].fd = s;
        pollfds[0].events = POLLHUP;
        pollfds[0].revents = 0;
        nr = TEMP_FAILURE_RETRY(poll(pollfds, 1, 250));
        if (nr <= 0 || (pollfds[0].revents & POLLHUP) == 0) {
            ALOGE("Timeout waiting for connection teardown. %s\n", strerror(errno));
        }

    close_conn:
        ALOGI("Closing connection.\n");
        close(s);

        if (quit) {
            ALOGI("QUIT...\n");
            break;
        }
    }

    return 0;
}

static int sockcmd_server_execute(struct sockcmd_server *ctx, int s, char *cmd)
{
    char reply[REPLY_MAX];
    char *arg[TOKEN_MAX+1];
    unsigned i;
    unsigned n = 0;
    unsigned short count;
    int ret = -1;
    struct sockcmd_method *p;

    ALOGV("Execute('%s')\n", cmd);

    /* default reply is "" */
    reply[0] = 0;

    /* n is number of args (not counting arg[0]) */
    arg[0] = cmd;
    while (*cmd) {
        if (isspace(*cmd)) {
            *cmd++ = 0;
            n++;
            arg[n] = cmd;
            if (n == TOKEN_MAX) {
                ALOGE("Too many arguments\n");
                goto done;
            }
        }
        cmd++;
    }

    for (p = ctx->method_list; p != NULL; p = p->next) {
        if (strcmp(p->name, arg[0]) == 0) {
            if (n != p->numargs) {
                ALOGE("%s requires %d arguments (%d given)\n",
                      p->name, p->numargs, n);
            } else {
                ret = p->func(arg + 1, reply);
            }
            goto done;
        }
    }
    ALOGE("Unsupported command '%s'\n", arg[0]);

done:
    if (reply[0]) {
        n = snprintf(cmd, BUFFER_MAX, "%d %s", ret, reply);
    } else {
        n = snprintf(cmd, BUFFER_MAX, "%d", ret);
    }
    if (n >= REPLY_MAX) n = REPLY_MAX - 1;
    count = n;

    ALOGV("Reply: '%s'\n", cmd);
    if (writex(s, &count, sizeof(count))) return -1;
    if (writex(s, cmd, count)) return -1;
    return 0;
}

static int do_sockcmd_quit(char **arg, char *reply)
{
    (void)arg;
    (void)reply;
    return 0;
}
