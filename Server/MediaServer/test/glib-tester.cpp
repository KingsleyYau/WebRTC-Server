/*
 * glib.cpp
 *
 *  Created on: 2015-1-14
 *      Author: Max.Chiu
 *      Email: Kingsleyyau@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>

#include <iostream>
#include <string>
using namespace std;

// Common
#include <common/Arithmetic.h>
#include <common/StringHandle.h>
#include <common/CommonFunc.h>
#include <common/Math.h>

// glib
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gnetworking.h>

void task_thread (
		GTask *task,
		gpointer source_object,
		gpointer task_data,
		GCancellable *cancellable) {
//	int *param = (int *)source_object;
	printf("[%d] task_thread\n", (int)syscall(SYS_gettid));
	g_task_return_boolean (task, TRUE);
}

void task_finish(GObject *source_object, GAsyncResult *res, gpointer user_data) {
//	int *param = (int *)source_object;
	printf("[%d] task_finish \n", (int)syscall(SYS_gettid));
}


gboolean timeout_cb (gpointer user_data) {
	printf("[%d] timeout_cb \n", (int)syscall(SYS_gettid));

	GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
	g_task_return_boolean (task, TRUE);
	g_object_unref (task);

	return G_SOURCE_REMOVE;
}

void timeout_data_destroy (gpointer user_data) {
	printf("[%d] timeout_data_destroy \n", (int)syscall(SYS_gettid));
}

void* deamon_thread(void *data) {
	printf("[%d] deamon \n", (int)syscall(SYS_gettid));

	GMainLoop* loop = (GMainLoop *)data;

//	GMainContext* ctx = g_main_loop_get_context(loop);
//	GSource *source = g_timeout_source_new_seconds(1);
//	g_source_set_name (source, "source1");
//	g_source_set_callback(source, timeout_cb, NULL, (GDestroyNotify)timeout_data_destroy);
//	g_source_attach(source, ctx);

    g_main_loop_run(loop);

    return 0;
}

void* worker_thread(void *data) {
	printf("[%d] worker \n", (int)syscall(SYS_gettid));
	GMainLoop* loop = (GMainLoop *)data;

//	GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
//	g_task_set_source_tag (task, (void *)loop_thread);
//	g_task_return_boolean (task, TRUE);
//	g_task_run_in_thread(task, task_thread);
//	g_object_unref (task);

	GMainContext* ctx = g_main_loop_get_context(loop);
	GSource *source = g_timeout_source_new_seconds(1);
	g_source_set_name (source, "source1");
	g_source_set_callback(source, timeout_cb, NULL, (GDestroyNotify)timeout_data_destroy);
	g_source_attach(source, ctx);

    g_main_loop_run(loop);

    return 0;
}

int main(int argc, char *argv[]) {
	printf("############## glib-tester ############## \n");
	srand(time(0));

	printf("[%d] run \n", (int)syscall(SYS_gettid));

	GMainContext* context = NULL;//g_main_context_new();
	GMainLoop *loop = g_main_loop_new (context, FALSE);
	GThread *thread = g_thread_new("worker", &deamon_thread, loop);

	GMainContext* context1 = g_main_context_new();
	GMainLoop *loop1 = g_main_loop_new (context1, FALSE);
	GThread *thread1 = g_thread_new("worker-1", &worker_thread, loop1);
//	GMainContext* context2 = g_main_context_new();
//	GMainLoop *loop2 = g_main_loop_new (context2, FALSE);
//	GThread *thread2 = g_thread_new("worker-2", &worker_thread, loop2);
//	GMainContext* context3 = g_main_context_new();
//	GMainLoop *loop3 = g_main_loop_new (context3, FALSE);
//	GThread *thread3 = g_thread_new("worker-3", &worker_thread, loop3);

	while(1) {
		sleep(1);
	}

	return EXIT_SUCCESS;
}
