/*
 * Definitions for file descriptor passing.
 *
 * Sockets in the AF_LOCAL (formerly AF_UNIX) address family may be used
 * to pass file descriptors between processes on single system. It is
 * first necessary to create the sockets between which the file descritors
 * will be send, which may be done with socketpair(2):
 *
 * 	int fds[2];
 *
 * 	rc = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
 * 	if (rc == -1)
 * 		// Handle error here
 *
 * Note that you can also use SOCK_DGRAM, though this hasn't been tested
 * with these functions.
 *
 * Sockets can also be created file socket files. These are created by
 * creating a socket with address family AF_LOCAL and then using bind(2)
 * to create an association between the socket and the Linux file namespace.
 * The address is specified by passing the path, in a struct sockaddr_un,
 * and the length of the path. Struct sockaddr_un is in the header file
 * sys/un.h.
 * (Untested).
 *
 * Once the socket has been created (or opened, if using socket type
 * files bound to a path on your filesystem, you can use the
 * sendfds() and recvfds() functions to send and receive file descriptors
 * on the socket file descriptor.
 * 
 * ref:
 * http://poincare.matf.bg.ac.rs/~ivana/courses/ps/sistemi_knjige/pomocno/apue/APUE/0201433079/ch17lev1sec4.html
 */
#pragma once
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>      
#include <sys/stat.h>


void send_fd(int socket, int send_fd);
int recv_fd(int socket);
