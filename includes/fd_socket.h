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
#pragma once
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>      
#include <sys/stat.h>


void send_fd(int socket, int send_fd);
int recv_fd(int socket);
