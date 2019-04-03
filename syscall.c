#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  printf ("system call!\n");
  thread_exit ();
}

/*Terminates Pintos by calling shutdown_power_off() 
  (declared in devices/shutdown.h). 
  This should be seldom used, 
  because you lose some information about possible deadlock situations, etc.*/
void halt (void){
	//not implemented
	return;
}

/*Terminates the current user program, returning status to the kernel.
  If the process's parent waits for it (see below),
  this is the status that will be returned. Conventionally,
  a status of 0 indicates success and nonzero values indicate errors.*/
void exit (int status){
	//not implemented
	return;
}

/*Runs the executable whose name is given in cmd_line, passing any given arguments,
  and returns the new process's program id (pid). Must return pid -1,
  which otherwise should not be a valid pid, if the program cannot load or run for any reason.
  Thus, the parent process cannot return from the exec until it knows
  whether the child process successfully loaded its executable.
  You must use appropriate synchronization to ensure this.*/
pid_t exec (const char *cmd_line){
	//not implemented
	return 0;
}

/*Waits for a child process pid and retrieves the child's exit status.
  If pid is still alive, waits until it terminates.
  Then, returns the status that pid passed to exit.
  If pid did not call exit(), but was terminated by the kernel 
  (e.g. killed due to an exception), wait(pid) must return -1.
  It is perfectly legal for a parent process to wait for child processes
  that have already terminated by the time the parent calls wait,
  but the kernel must still allow the parent to retrieve its child's exit status,
  or learn that the child was terminated by the kernel.*/
int wait (pid_t pid){
	//not implemented
  	return 0;
}

/* Creates a new file called file initially initial_size bytes in size. 
  Returns true if successful, false otherwise. 
  Creating a new file does not open it: opening the new file is 
  a separate operation which would require a open system call.*/
bool create (const char *file, unsigned initial_size){
	//not implemented
	return true;
}

/* Deletes the file called file. 
  Returns true if successful, false otherwise. 
  A file may be removed regardless of whether it is open or closed, 
  and removing an open file does not close it. 
  See FAQ: Removing an Open File, for details.*/
bool remove (const char *file){
	//not implemented
	return true;
}

/* Opens the file called file. 
  Returns a nonnegative integer handle called a 
  "file descriptor" (fd), or -1 if the file could not be opened.*/
int open (const char *file){
	//not implemented
	return 0;
}

/* Returns the size, in bytes, of the file open as fd.*/
int filesize (int fd){
	//not implemented
	return 0;
}

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read (0 at end of file),
   or -1 if the file could not be read (due to a condition other than end of file).
   Fd 0 reads from the keyboard using input_getc().*/
int read (int fd, void *buffer, unsigned size){
	//not implemented
	return 0;
}

/* Writes size bytes from buffer to the open file fd.
  Returns the number of bytes actually written, 
  which may be less than size if some bytes could not be written*/
int write (int fd, const void *buffer, unsigned size){
	//not implemented
	return 0;
}

/*Changes the next byte to be read or written in open file fd to position,
  expressed in bytes from the beginning of the file.
  (Thus, a position of 0 is the file's start.)*/
void seek (int fd, unsigned position){
	//not implemented
	return;
}

/*Returns the position of the next byte to be read or written in open file fd,
  expressed in bytes from the beginning of the file.*/
unsigned tell (int fd){
	//not implemented
	return 0;
}

/*Closes file descriptor fd.
  Exiting or terminating a process implicitly
  closes all its open file descriptors,
  as if by calling this function for each one.*/
void close (int fd){
	//not implemented
	return;
}
