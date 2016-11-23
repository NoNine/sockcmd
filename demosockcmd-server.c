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
#define LOG_TAG "demosockcmd-server"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cutils/log.h>

#include <sockcmd/sockcmd.h>

SOCKCMD_METHOD(demosockcmd_ping, "Ping", 0);
SOCKCMD_METHOD(demosockcmd_add, "Add", 2);

sockcmd_t hsockcmd;

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    sockcmd_server_init(&hsockcmd, "demosockcmd");

    sockcmd_server_add_method(hsockcmd, &demosockcmd_ping);
    sockcmd_server_add_method(hsockcmd, &demosockcmd_add);

    sockcmd_server_loop(hsockcmd);

    sockcmd_server_term(hsockcmd);

    return 0;
}

static int do_demosockcmd_ping(char **arg, char *reply)
{
    (void)arg;
    (void)reply;
    return 0;
}

static int do_demosockcmd_add(char **arg, char *reply)
{
    int a;
    int b;

    a = atoi(arg[0]);
    b = atoi(arg[1]);

    /*
     * The result can be returned by relpy,
     * but it's a bit complex to parse the string.
     */
    /* snprintf(reply, REPLY_MAX, "%d", a + b); */

    return a + b;
}
