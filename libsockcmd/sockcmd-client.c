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
#define LOG_TAG "sockcmd-client"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cutils/sockets.h>
#include <cutils/log.h>

#include <sockcmd/sockcmd.h>
#include "sockcmd-common.h"

struct sockcmd_client {
    char *sockname;
};

int sockcmd_client_init(sockcmd_t *phandle, const char *sockname)
{
    struct sockcmd_client *ctx;

    if (sockname == NULL) {
        ALOGE("Socket name is NOT specified.\n");
        return -1;
    }

    ctx = (struct sockcmd_client *)malloc(sizeof(struct sockcmd_client));
    if (ctx == NULL) {
        ALOGE("No free memory.\n");
        return -1;
    }

    memset(ctx, 0, sizeof(struct sockcmd_client));

    ctx->sockname = strdup(sockname);
    if (ctx->sockname == NULL) {
        ALOGE("No free memory.\n");
        goto error0;
    }

    *phandle = (sockcmd_t)ctx;

    return 0;

error0:
    free(ctx);
    return -1;
}

int sockcmd_client_term(sockcmd_t handle)
{
    struct sockcmd_client *ctx = (struct sockcmd_client *)handle;

    if (ctx == NULL) {
        return -1;
    }

    free(ctx->sockname);
    free(ctx);

    return 0;
}

int sockcmd_client_execute(sockcmd_t handle, const char *cmd, char *reply)
{
    struct sockcmd_client *ctx = (struct sockcmd_client *)handle;
    int status = 0;
    int s = -1;
    struct sockaddr_un addr;
    unsigned short count;
    struct pollfd pollfds[1];
    int nr = 0;

    if (ctx == NULL) {
        return -1;
    }

    ALOGV("Execute('%s')\n", cmd);

    s = socket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (s == -1) {
        ALOGE("Create socket failed.\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_LOCAL;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
             ANDROID_SOCKET_DIR, ctx->sockname);

    if (TEMP_FAILURE_RETRY(connect(s, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un))) == -1) {
        ALOGE("Connect to remote failed: %s\n", strerror(errno));
        goto error0;
    }

    count = strlen(cmd);
    if (TEMP_FAILURE_RETRY(send(s, &count, sizeof(count), 0)) != sizeof(count) ||
        TEMP_FAILURE_RETRY(send(s, cmd, count, 0)) != count) {
        ALOGE("Send command failed: %s\n", strerror(errno));
        goto error0;
    }

    /* Reply. */
    pollfds[0].fd = s;
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;
    nr = TEMP_FAILURE_RETRY(poll(pollfds, 1, 250));
    if (nr <= 0) {
        ALOGE("Timeout waiting for reply. %s\n", strerror(errno));
        goto error0;
    }

    if (readx(s, &count, sizeof(count))) {
        ALOGE("Read size failed.\n");
        goto error0;
    }
    if ((count < 1) || (count >= REPLY_MAX)) {
        ALOGE("Invalid size %d.\n", count);
        goto error0;
    }
    if (readx(s, reply, count)) {
        ALOGE("Read reply failed.\n");
        goto error0;
    }
    reply[count] = '\0';

    ALOGV("Reply: %s.\n", reply);

    close(s);
    return 0;

error0:
    close(s);
    return -1;
}
