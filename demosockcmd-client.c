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
#define LOG_TAG "demosockcmd-client"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sockcmd/sockcmd.h>

sockcmd_t hsockcmd;

int demosockcmd_quit(void)
{
    int status = 0;
    char cmd[BUFFER_MAX];
    char reply[REPLY_MAX];

    memset(cmd, 0, sizeof(cmd));
    memset(reply, 0, sizeof(reply));

    snprintf(cmd, sizeof(cmd), "QUIT");

    status = sockcmd_client_execute(hsockcmd, cmd, reply);

    if (status == 0) {
        status = atoi(reply);
    }

    return status;
}

int demosockcmd_ping(void)
{
    int status = 0;
    char cmd[BUFFER_MAX];
    char reply[REPLY_MAX];

    memset(cmd, 0, sizeof(cmd));
    memset(reply, 0, sizeof(reply));

    snprintf(cmd, sizeof(cmd), "Ping");

    status = sockcmd_client_execute(hsockcmd, cmd, reply);

    if (status == 0) {
        status = atoi(reply);
    }

    return status;
}

int demosockcmd_add(int a, int b)
{
    int status = 0;
    char cmd[BUFFER_MAX];
    char reply[REPLY_MAX];

    memset(cmd, 0, sizeof(cmd));
    memset(reply, 0, sizeof(reply));

    snprintf(cmd, sizeof(cmd), "Add %d %d", a, b);

    status = sockcmd_client_execute(hsockcmd, cmd, reply);

    if (status == 0) {
        status = atoi(reply);
    }

    return status;
}

int demosockcmd_run(const char *cmd, char *reply)
{
    int status = 0;
    char reply_tmp[REPLY_MAX];

    memset(reply_tmp, 0, sizeof(reply_tmp));

    status = sockcmd_client_execute(hsockcmd, cmd, (reply)? reply: reply_tmp);

    if (status == 0) {
        status = atoi((reply)? reply: reply_tmp);
    }

    return status;
}

int main(int argc, char** argv)
{
    int i;
    char cmd[2048] = "";

    (void)argc;
    (void)argv;

    sockcmd_client_init(&hsockcmd, "demosockcmd");

    if (argc == 1) {
        int i = 0;
        while (1) {
            printf("#%d\n", i++);
            printf("Ping: %d\n", demosockcmd_ping());
            printf("3 + 7 = %d\n", demosockcmd_add(3, 7));

            usleep(50 * 1000);
        }
    } else {
        for (i = 1; i < argc; i++) {
            strcat(cmd, argv[i]);
            if (i != (argc - 1)) {
                strcat(cmd, " ");
            }
        }
        printf("CMD: %s %d\n", cmd, demosockcmd_run(cmd, NULL));
    }

    sockcmd_client_term(hsockcmd);

    return 0;
}
