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
#include <common/KMutex.h>

// glib
#include <glib-object.h>
#include <gio/gio.h>
#include <gio/gnetworking.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

KMutex gKMutex;
#define MAX_LOG_BUFFER_LEN 10 * 1024
GSource *sources[65535];

bool gRunning[4];
int gTotalTimes = 10000;

void log(const char *format, ...) {
	char logBuffer[MAX_LOG_BUFFER_LEN] = {0};
	char bitBuffer[128] = {0};

	//get current time
	time_t stm = time(NULL);
	struct tm tTime;
	localtime_r(&stm,&tTime);

	struct timeval tv;
	gettimeofday(&tv, NULL);

	snprintf(bitBuffer, sizeof(bitBuffer) - 1, "[ %d-%02d-%02d %02d:%02d:%02d.%03d tid:%-6d ] ",
			tTime.tm_year+1900, tTime.tm_mon+1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec, tv.tv_usec / 1000,
			(int)syscall(SYS_gettid)
			);

    //get va_list
    va_list	agList;
    va_start(agList, format);
    vsnprintf(logBuffer, MAX_LOG_BUFFER_LEN - 1, format, agList);
    va_end(agList);

    strcat(logBuffer, "\n");

    printf(bitBuffer);
    printf(logBuffer);
}

#define MAX_EVENTS 10000

typedef struct IOSource {
    GSource source;
    int fd;
    bool canRead;
    int total;
} IOSource;

static gboolean prepare(GSource *source, gint *timeout) {
//	log("prepare ");
	IOSource *isrc = (IOSource *)source;
//	if ( false ) {
	if ( isrc->canRead ) {
		return TRUE;
	} else {
		*timeout = 1000;
	}
    return FALSE;
}

static gboolean check(GSource *source) {
//	log("check");
	IOSource *isrc = (IOSource *)source;
	if ( isrc->canRead ) {
		return TRUE;
	}
    return FALSE;
}

static gboolean dispatch(GSource *source, GSourceFunc callback, gpointer user_data) {
	IOSource *isrc = (IOSource *)source;
//	log("dispatch ", (int)syscall(SYS_gettid));

	isrc->canRead = false;
	int ret = 0;
	char buff[1024] = {0};
	while (true) {
    	ret = read(isrc->fd, buff, sizeof(buff));
        if (-1 == ret) {
            if (EAGAIN != errno) {
            	log("recv clientfd(%d) error %d ", isrc->fd, errno);
            	close (isrc->fd);
            } else {
//            	log("recv clientfd(%d) nothing ", isrc->fd);
            }
            break;
        } else if (!ret) {
        	log("recv clientfd(%d) finish ", isrc->fd);
        	close (isrc->fd);
            break;
        } else {
        	isrc->total += ret;
//        	log("recv clientfd(%d) %d, total:%d", isrc->fd, ret, isrc->total);
        }
	}
    return TRUE;
}

void task_thread (
		GTask *task,
		gpointer source_object,
		gpointer task_data,
		GCancellable *cancellable) {
//	int *param = (int *)source_object;
	log("[%d] task_thread", (int)syscall(SYS_gettid));
	g_task_return_boolean (task, TRUE);
}

void task_finish(GObject *source_object, GAsyncResult *res, gpointer user_data) {
//	int *param = (int *)source_object;
	log("task_finish ", (int)syscall(SYS_gettid));
}


gboolean timeout_cb (gpointer user_data) {
	log("timeout_cb ", (int)syscall(SYS_gettid));

	GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
	g_task_return_boolean (task, TRUE);
	g_object_unref (task);

	return G_SOURCE_REMOVE;
}

void timeout_data_destroy (gpointer user_data) {
	log("timeout_data_destroy ", (int)syscall(SYS_gettid));
}

void* deamon_thread(void *data) {
	log("deamon ", (int)syscall(SYS_gettid));

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
	log("worker %d", (int)syscall(SYS_gettid));
//	GMainLoop* loop = (GMainLoop *)data;

//	GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
//	g_task_set_source_tag (task, (void *)loop_thread);
//	g_task_return_boolean (task, TRUE);
//	g_task_run_in_thread(task, task_thread);
//	g_object_unref (task);

//	GMainContext* ctx = g_main_loop_get_context(loop);
//	GSource *source = g_timeout_source_new_seconds(1);
//	g_source_set_name (source, "source1");
//	g_source_set_callback(source, timeout_cb, NULL, (GDestroyNotify)timeout_data_destroy);
//	g_source_attach(source, ctx);

//    GSourceFuncs source_funcs = {prepare, check, dispatch, NULL};
//    GSource *source = g_source_new(&source_funcs, sizeof(IOSource));
//    sprintf(((IOSource *)source)->text, "Hello world!");
//    g_source_attach(source, ctx);
//    g_source_unref(source);

//    g_main_loop_run(loop);

	bool *running = (bool *)data;
    guint8 tmp[1024] = {0};
    GOutputVector buffer = { tmp, sizeof(tmp) };

	while (true) {
		if (*running) {
			for (int t = 0; t < gTotalTimes; t++) {
				gssize ret = 0;
				GError *gerr = NULL;

				GSocket *gsock = g_socket_new (G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
					G_SOCKET_PROTOCOL_TCP, NULL);
				ret = g_socket_send_message(gsock, NULL, &buffer,
						1, NULL, 0, G_SOCKET_MSG_NONE, NULL, &gerr);

				log("worker %d, Socket %p(FD %d): Send, len %d, ret %d, errno %d, times %d\n",
						(int)syscall(SYS_gettid),
						gsock, g_socket_get_fd(gsock),
						sizeof(tmp), ret, errno,
						t);
				if (gerr) {
					g_error_free(gerr);
				}

				g_socket_close(gsock, NULL);
				g_object_unref(gsock);
			}
			*running = false;
		} else {
			sleep(1);
		}
	}

    return 0;
}

static int set_non_blocking (int fd) {
    int flags, s;
    // 获取当前flag
    flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        log("set_non_blocking F_GETFL %d error", fd);
        return -1;
    }

    flags |= O_NONBLOCK;

    // 设置flag
    s = fcntl(fd, F_SETFL, flags);
    if (-1 == s) {
        log("set_non_blocking F_SETFL %d error", fd);
        return -1;
    }
    return 0;
}

gboolean glib_io_cb_read(GSocket *gsock, GIOCondition condition, gpointer user_data) {
	bool result = TRUE;
	GError *gerr = NULL;

	while (true) {
	    gint flags = G_SOCKET_MSG_NONE;
	    gssize len;
	    char buff[1024] = {0};
	    GInputVector buffers = {buff, sizeof(buff)};

	    len = g_socket_receive_message(gsock, NULL, &buffers, 1, NULL, NULL, &flags, NULL, &gerr);
	    if (len > 0) {
//	    	log("glib_io_cb_read clientfd(%d), %s", g_socket_get_fd(gsock), buff);
	    } else if (len == 0) {
	    	log("glib_io_cb_read clientfd(%d) finish", g_socket_get_fd(gsock));
	    	result = G_SOURCE_REMOVE;
	    	break;
	    } else if (len < 0) {
	    	if (g_error_matches (gerr, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK)) {
//	    		log("glib_io_cb_read clientfd(%d) nothing", g_socket_get_fd(gsock));
	    		break;
	    	} else {
	    		log("glib_io_cb_read clientfd(%d) finish", g_socket_get_fd(gsock));
	    		result = G_SOURCE_REMOVE;
	    		break;
	    	}
	    }
	}

	if (gerr) {
		g_error_free (gerr);
	}

	if (!result) {
		g_socket_close(gsock, NULL);
	}

	return result;
}

gboolean glib_io_cb_accept(GSocket *gsock, GIOCondition condition, gpointer user_data) {
//	log("glib_io_cb_accept listenfd(%d)", g_socket_get_fd(gsock));
	GError *gerr = NULL;
	GSocket *gsock_client = g_socket_accept(gsock, NULL, &gerr);
	if ( gsock_client ) {
		log("glib_io_cb_accept clientfd(%d)", g_socket_get_fd(gsock_client));
		g_socket_set_blocking(gsock_client, false);

		struct sockaddr addr;
		GSocketAddress *gaddr = g_socket_get_remote_address(gsock_client, NULL);
		if (gaddr == NULL || !g_socket_address_to_native(gaddr, &addr, sizeof (addr), NULL)) {
			g_socket_close (gsock, NULL);
			g_object_unref (gsock);
		}
		g_object_unref (gaddr);

		GMainContext *ctx = (GMainContext *)user_data;
	    GSource *source = g_socket_create_source (gsock_client, G_IO_IN, NULL);
	    g_source_set_callback(source, (GSourceFunc) G_CALLBACK (glib_io_cb_read), (gpointer)ctx, NULL);
	    g_source_attach (source, ctx);
	} else {
		log("glib_io_cb_accept listenfd(%d) error %s", g_socket_get_fd(gsock), gerr->message);
		g_socket_close(gsock, NULL);
		return G_SOURCE_REMOVE;
	}

	if (gerr) {
		g_error_free (gerr);
	}

    return TRUE;
}

int main(int argc, char *argv[]) {
	printf("############## glib-tester ############## \n");
	srand(time(0));

	log("run");

	GMainContext* context = NULL;//g_main_context_new();
	GMainLoop *loop = g_main_loop_new (context, FALSE);
	GThread *thread = g_thread_new("worker", &deamon_thread, loop);

	GMainContext* context1 = g_main_context_new();
	GMainLoop *loop1 = g_main_loop_new (context1, FALSE);
	GThread *thread1 = g_thread_new("worker-1", &worker_thread, &gRunning[0]);
	GMainContext* context2 = g_main_context_new();
	GMainLoop *loop2 = g_main_loop_new (context2, FALSE);
	GThread *thread2 = g_thread_new("worker-2", &worker_thread, &gRunning[1]);
//	GMainContext* context3 = g_main_context_new();
//	GMainLoop *loop3 = g_main_loop_new (context3, FALSE);
//	GThread *thread3 = g_thread_new("worker-3", &worker_thread, loop3);


//    GSocket *gsock = g_socket_new (G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_STREAM,
//        G_SOCKET_PROTOCOL_TCP, NULL);
//    GInetAddress *address = g_inet_address_new_from_string("0.0.0.0");
//    GSocketAddress *socket_address = g_inet_socket_address_new(address, 12345);
//    g_socket_set_blocking (gsock, false);
//    g_socket_set_option (gsock, IPPROTO_TCP, TCP_NODELAY, TRUE, NULL);
//    g_socket_set_option (gsock, SOL_SOCKET, SO_REUSEADDR, TRUE, NULL);
//    g_socket_bind (gsock, socket_address, FALSE, NULL) && g_socket_listen (gsock, NULL);
//    log("listenfd(%d)", g_socket_get_fd(gsock));
//
//    GSource *source = g_socket_create_source (gsock, G_IO_IN, NULL);
//    g_source_set_callback(source, (GSourceFunc) G_CALLBACK (glib_io_cb_accept), (gpointer)context1, NULL);
//    g_source_attach (source, context1);


//	// 创建socket
//    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
//    int flag = 1;
//    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
//    log("setsockopt listenfd(%d) %d", listenfd, ret);
//
//    // 绑定的地址
//    struct sockaddr_in server_addr = { 0 };
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(12345);
//    server_addr.sin_addr.s_addr = INADDR_ANY;
////    const char * const local_addr = "0.0.0.0";
////    inet_aton (local_addr, &(server_addr.sin_addr));
//    ret = bind(listenfd, (const struct sockaddr *)&server_addr, sizeof (server_addr));
//    log("bind listenfd(%d) %d", listenfd, ret);
//    ret = set_non_blocking(listenfd);
//    ret = listen(listenfd, 1024);
//    log("listen listenfd(%d) %d", listenfd, ret);
//
//	// 创建epoll
//	struct epoll_event ev, event[MAX_EVENTS];
//    int epollfd = epoll_create1(EPOLL_CLOEXEC);
//	ev.events = EPOLLIN | EPOLLET;
//	ev.data.fd = listenfd;
//	if ( epoll_ctl( epollfd, EPOLL_CTL_ADD, listenfd, &ev ) == -1 ) {
//		log("poll_ctl listenfd(%d) error", listenfd);
//	    exit( EXIT_FAILURE );
//	}
//
//	while(true) {
//		int nfds = epoll_wait(epollfd, event, MAX_EVENTS, -1);
//	    for ( int i = 0; i < nfds; ++i ) {
//	    	uint32_t events = event[i].events;
//	    	if ( events & EPOLLERR || events & EPOLLHUP || (! events & EPOLLIN) ) {
//	    		if ( event[i].data.fd == listenfd ) {
//	    			log("epoll listenfd(%d) has error %d ", event[i].data.fd, events);
//	    			close (event[i].data.fd);
//	    			break;
//	    		} else {
//	    			log("epoll clientfd(%d) has error %d ", event[i].data.fd, events);
//	            	IOSource *source = (IOSource *)sources[event[i].data.fd];
//	    	        source->canRead = true;
//	    	        sources[event[i].data.fd] = NULL;
//	    			continue;
//	    		}
//	    	} else if ( event[i].data.fd == listenfd ) {
//                struct sockaddr in_addr = { 0 };
//                socklen_t in_addr_len = sizeof (in_addr);
//	            int clientfd = accept(listenfd, (struct sockaddr *) &in_addr, &in_addr_len);
//	            if ( clientfd == -1 ) {
//	            	log("accept error");
//	            	break;
//	                exit( EXIT_FAILURE );
//	            }
//	            log("accept clientfd(%d) ", clientfd);
//	            ret = set_non_blocking( clientfd );
//	            ev.events = EPOLLIN | EPOLLET;
//	            ev.data.fd = clientfd;
//	            if ( epoll_ctl( epollfd, EPOLL_CTL_ADD, clientfd, &ev ) == -1 ) {
//	            	log("epoll_ctl clientfd(%d) ", clientfd);
//	                exit( EXIT_FAILURE );
//	            }
//
//	            GSourceFuncs source_funcs = {prepare, check, dispatch, NULL};
//	            GSource *source = g_source_new(&source_funcs, sizeof(IOSource));
//	            ((IOSource *)source)->fd = clientfd;
//	            ((IOSource *)source)->canRead = false;
//	            ((IOSource *)source)->total = 0;
//	            sources[clientfd] = source;
//
//	            g_source_attach(source, context1);
//	            g_source_unref(source);
//	        } else {
//	        	IOSource *isrc = (IOSource *)sources[event[i].data.fd];
//	        	isrc->canRead = true;
////	        	log("epoll_wait can read clientfd(%d)", event[i].data.fd);
////				int ret = 0;
////				char buff[1600] = {0};
////				while (true) {
////					ret = read(isrc->fd, buff, sizeof(buff));
////					if (-1 == ret) {
////						if (EAGAIN != errno) {
////							log("recv clientfd(%d) error %d ", isrc->fd, errno);
////							close (isrc->fd);
////						} else {
//////			            	log("recv clientfd(%d) nothing ", isrc->fd);
////							isrc->canRead = false;
////						}
////						break;
////					} else if (!ret) {
////						log ("recv clientfd(%d) finish ", isrc->fd);
////						isrc->canRead = false;
////						close (isrc->fd);
////						break;
////					} else {
////						isrc->total += ret;
//////			        	log("recv clientfd(%d) %d, total:%d", isrc->fd, ret, isrc->total);
////					}
////				}
////				isrc->canRead = false;
//	        }
//	    }
//	}

	char c;
	for(int i = 0; i < 4; i++) {
		gRunning[i] = false;
	}

//	GWeakRef ref;
//	GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
//	g_weak_ref_init(&ref, task);
//	g_object_unref(task);
//	GTask *task_weak = (GTask *)g_weak_ref_get(&ref);

	GSList *l = NULL;
	for (int i = 0; i < 10; i++) {
		GTask *task = g_task_new (NULL, NULL, task_finish, NULL);
		l = g_slist_append (l, task);
		log("add:%d task:%p", i, task);
	}

	GSList *d;
	int i;
	for (i = 0, d = l; d; i++) {
		GSList *next = d->next;
		GTask *task = (GTask *)d->data;
		l = g_slist_remove(l, task);
		log("remove:%d, task:%p", i, task);
		d = next;
	}

	while (true) {
		printf("please input any key to continue.\n");
		scanf("%c", &c);
		for(int i = 0; i < 4; i++) {
			gRunning[i] = true;
		}
	}


	return EXIT_SUCCESS;
}
