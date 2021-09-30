/*
 * FUSE: Filesystem in Userspace
 * Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
 *
 * This program can be distributed under the terms of the GNU LGPLv2.
 * See the file COPYING.LIB
 */

#ifndef FUSE_I_H
#define FUSE_I_H

#define FUSE_USE_VERSION 31
#include "fuse_lowlevel.h"

struct fv_VuDev;
struct fv_QueueInfo;

struct fuse_req {
    struct fuse_session *se;
    uint64_t unique;
    int ctr;
    pthread_mutex_t lock;
    struct fuse_ctx ctx;
    struct fuse_chan *ch;
    int interrupted;
    unsigned int ioctl_64bit:1;
    union {
        struct {
            uint64_t unique;
        } i;
        struct {
            fuse_interrupt_func_t func;
            void *data;
        } ni;
    } u;
    struct fuse_req *next;
    struct fuse_req *prev;
};

struct fuse_notify_req {
    uint64_t unique;
    void (*reply)(struct fuse_notify_req *, fuse_req_t, fuse_ino_t,
                  const void *, const struct fuse_buf *);
    struct fuse_notify_req *next;
    struct fuse_notify_req *prev;
};

/*
 * Struct to store the inotify descriptors for the inotify instances generated
 * by virtiofsd. The refcount defines the lifetime of the inotify instance. If
 * the refcount reaches 0 then the inotify instance is deleted.
 */
struct fuse_inotify_fd {
    int fd;
    uint64_t refcount;
};

/*
 * Store all the inotify instances created by vitiofsd as wellas the mappings
 * from inodes to watches and vice versa
 */
struct fuse_inotify {
    int epoll_fd;
    int thread_running;
    pthread_t i_thread;
    /* Signal for inotify thread to cleanup */
    int cleanup_inotify;
    /* Lock to protect the inotify data */
    pthread_mutex_t i_lock;
    GHashTable *inotify_fds;
    GSList *inotify_fd_list;
    GHashTable *wd_to_inode;
    GHashTable *inode_to_wd;
};

struct fuse_session {
    char *mountpoint;
    volatile int exited;
    int fd;
    int debug;
    int deny_others;
    struct fuse_lowlevel_ops op;
    int got_init;
    struct cuse_data *cuse_data;
    void *userdata;
    uid_t owner;
    struct fuse_conn_info conn;
    struct fuse_req list;
    struct fuse_req interrupts;
    pthread_mutex_t lock;
    pthread_rwlock_t init_rwlock;
    int got_destroy;
    int broken_splice_nonblock;
    uint64_t notify_ctr;
    struct fuse_notify_req notify_list;
    size_t bufsize;
    int error;
    char *vu_socket_path;
    char *vu_socket_group;
    int   vu_listen_fd;
    int   vu_socketfd;
    struct fv_VuDev *virtio_dev;
    int thread_pool_size;
    bool notify_enabled;
    struct fuse_inotify *inotify;
};

struct fuse_chan {
    pthread_mutex_t lock;
    int ctr;
    int fd;
    struct fv_QueueInfo *qi;
};

/*
 * Keys for looking up the mappings between the inodes and inotify
 * watch descriptors
 */
struct inotify_inode_key {    /* Inode to watch mapping */
	int inotify_fd;
	fuse_ino_t nodeid;
};

struct inotify_wd_key {       /* Watch to inode mapping */
	int inotify_fd;
	int wd;
};

int fuse_send_reply_iov_nofree(fuse_req_t req, int error, struct iovec *iov,
                               int count);
void fuse_free_req(fuse_req_t req);

void fuse_session_process_buf_int(struct fuse_session *se,
                                  struct fuse_bufvec *bufv,
                                  struct fuse_chan *ch);


#define FUSE_MAX_MAX_PAGES 256
#define FUSE_DEFAULT_MAX_PAGES_PER_REQ 32

/* room needed in buffer to accommodate header */
#define FUSE_BUFFER_HEADER_SIZE 0x1000

#endif
