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
 *
 * sockcmd:
 *      A simple socket based RPC implementation.
 *      It's useful when binder is not available, e.g., Recovery environment.
 *      And sometimes, binder is just too complex for a simple use case.
 *
 *      It refers to "installd.c" in Android heavily.
 */

#ifndef __SOCKCMD_H__
#define __SOCKCMD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Common definitions. */
#define BUFFER_MAX  1024    /* input buffer for commands */
#define TOKEN_MAX   8       /* max number of arguments in buffer */
#define REPLY_MAX   256     /* largest reply allowed */

typedef void *sockcmd_t;

/* Client side. */
int sockcmd_client_init(sockcmd_t *phandle, const char *sockname);
int sockcmd_client_term(sockcmd_t handle);
int sockcmd_client_execute(sockcmd_t handle, const char *cmd, char *reply);


/* Server side. */
#define SOCKCMD_METHOD_FUNC(name, arg, reply) \
    static int do_##name(char **arg, char *reply)

#define SOCKCMD_METHOD_NAME(name) extern struct sockcmd_method name

#define SOCKCMD_METHOD(name, strname, numargs) \
    SOCKCMD_METHOD_FUNC(name, arg, reply); \
    struct sockcmd_method name = {NULL, strname, numargs, do_##name}

struct sockcmd_method {
    struct sockcmd_method *next;
    const char *name;
    unsigned numargs;
    int (*func)(char **arg, char *reply);
};

int sockcmd_server_init(sockcmd_t *phandle, const char *sockname);
int sockcmd_server_term(sockcmd_t handle);
int sockcmd_server_add_method(sockcmd_t handle, struct sockcmd_method *method);
int sockcmd_server_loop(sockcmd_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __SOCKCMD_H__ */
