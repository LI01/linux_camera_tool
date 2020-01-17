/*
  * This file is part of the Linux Camera Tool 
 * Copyright (c) 2020 Leopard Imaging Inc.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * ioctl transfer file descriptor is for controlling a STREAMS decice
 * error: [Errno 25] Inappropriate ioctl for device --> I_SENDFD
 * only solution is to use unix socket sendmsg, SCM_RIGHTS(send or 
 * receive a set of open file descriptors from another process.), msg_control
 * domain socket couldn't pass file descriptor, need some kernel operation
 * it supports sendmsg, and stream
 * 
 * The kernel internals that support FD passing are actually quite simple 
 * - POSIX already require that two processes be able to share the same 
 * underlying reference to a file because of the semantics of the fork call. 
 * Adding some ability to share arbitrary file descriptors between two processes 
 * then is far more about how you ask the kernel than the actual file descriptor 
 * sharing operating. In linux, file descriptors can be passed through local 
 * network sockets. The sender constructs a mystic-looking sendmsg call, placing 
 * the file descriptor in the control field of that operation. The kernel pulls 
 * the file descriptor out of the control field, allocates a file descriptor in 
 * the target process which references the same file object and then sticks the 
 * file descriptor in a queue for the receiving process to fetch
 * ref: http://poincare.matf.bg.ac.rs/~ivana/courses/ps/sistemi_knjige/pomocno/apue/APUE/0201433079/ch17lev1sec4.html
 
 *  Author: Danyu L                                                           
 *  Last edit: 2019/09     
 */
#include "../includes/shortcuts.h"
#include "../includes/fd_socket.h"
/**
 * struct msghdr {
 *      void *msg_name;         // optional address
 *      socklen_t msg_namelen;  //size of address
 *      struct iovect *msg_iov; // scatter/gather array
 *      size_t msg_iovlen;      // # of elements in msg_iov
 *      void *msg_control;      // ancillary data
 *      size_t msg_controllen;  // ancillary data buffer len
 *      int msg_flags;          // flags on received message
 * };
 * msghdr->msg_iov     ------- iovect->iov_base 
 * msghdr->msg_control ------- cmsghdr->cmsg_len = 16
 */

/**
 * Sends given file descriptior via given socket
 *
 * @param socket to be used for fd sending
 * @param fd to be sent
 * @return sendmsg result
 *
 * @note socket should be (PF_UNIX, SOCK_DGRAM)
 */
void send_fd(int socket, int send_fd) {
    char dummy = 0;
    struct msghdr msg;
    struct iovec iov;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    CLEAR(cmsgbuf);
    CLEAR(iov);
    iov.iov_base = &dummy;
    iov.iov_len = sizeof(dummy);
    CLEAR(msg);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = CMSG_LEN(sizeof(int));

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    *(int*) CMSG_DATA(cmsg) = send_fd;

    int ret = sendmsg(socket, &msg, 0);

    if (ret == -1) {
        printf("sendmsg failed with %s\r\n", strerror(errno));
    }
}

/**
 * Receives file descriptor using given socket
 *
 * @param socket to be used for fd reception
 * @return received file descriptor; -1 if failed
 *
 * @note socket should be (PF_UNIX, SOCK_DGRAM)
 */
int recv_fd(int socket) {

    int len;
    int recv_fd;
    char buf[1];
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    char cms[CMSG_SPACE(sizeof(int))];
    CLEAR(cms);
    CLEAR(iov);
    CLEAR(msg);
    CLEAR(cmsg);
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    
    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    msg.msg_control = (caddr_t) cms;
    msg.msg_controllen = sizeof cms;

    len = recvmsg(socket, &msg, 0);

    if (len < 0) {
        printf("recvmsg failed with %s\r\n", strerror(errno));
        return -1;
    }

    if (len == 0) {
        printf("recvmsg failed no data\r\n");
        return -1;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    memmove(&recv_fd, CMSG_DATA(cmsg), sizeof(int));
    return recv_fd;
}

