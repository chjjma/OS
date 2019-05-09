#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

//added
#include "userprog/process.h"
#include "threads/synch.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

//added 
static struct file *fd_con[2000];
#define FD_MAX 2000
static int valid=1;

void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

//pintos -v -k -T 60 --qemu --fs-disk=2 -p tests/userprog/args-none -a args-none -- -q -f run 'args-none'
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

static void
syscall_handler (struct intr_frame *f) 
{
    uint32_t arg0, arg1, arg2, arg3;
	uint32_t *esp;
	esp=(uint32_t *)f->esp;
	if((unsigned int)esp>=(unsigned int)PHYS_BASE){
		exit(-1);
		return;
	}
	if(get_user(esp+1)==-1){
	    exit(-1);
	    return;
	}
	arg0 = esp[0];
	uint32_t ret=0;
	//ASSERT(0);
	//printf("system call:%d\n",arg0);
	switch(arg0){
		case SYS_HALT:                   /* Halt the operating system. */
			halt();
			break;
		case SYS_EXIT:                   /* Terminate this process. */
			if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
			    exit(-1);
			    return;
			}
    		if(get_user(esp+1)==-1){
        		    exit(-1);
       		     return;
    		}
			arg1 = esp[1];
			exit((int)arg1);
		    break;
	    case SYS_EXEC:                   /* Start another process. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if((char*)arg1==NULL||(unsigned int)(char*)arg1>=(unsigned int)PHYS_BASE||arg1=='\0'){
                exit(-1);
                return;
            }
            if(get_user(arg1)==-1){
                exit(-1);
                return;
            }
	    	ret = exec ((char *)arg1);
	    	break;
	    case SYS_WAIT:                   /* Wait for a child process to die. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
	    	ret = wait ((pid_t)arg1);
	    	break;
	    case SYS_CREATE:                 /* Create a file. */
            if((unsigned int)(esp+2)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
        		if((char*)arg1==NULL||(unsigned int)(char*)arg1>=(unsigned int)PHYS_BASE||arg1=='\0'){
			    exit(-1);
			    return;
			}
            if(get_user(arg1)==-1){
                exit(-1);
                return;
            }
            if(get_user(esp+2)==-1){
                exit(-1);
                 return;
            }
			arg2 = esp[2];
	    	ret = create((char *)arg1, (unsigned)arg2);
	    	break;
	    case SYS_REMOVE:                 /* Delete a file. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if((char*)arg1==NULL||(unsigned int)(char*)arg1>=(unsigned int)PHYS_BASE||arg1=='\0'){
                exit(-1);
                return;
            }
            if(get_user(arg1)==-1){
                exit(-1);
                return;
            }
	    	ret = remove((char *)arg1);
	    	break;
	    case SYS_OPEN:                   /* Open a file. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if((char*)arg1==NULL||(unsigned int)(char*)arg1>=(unsigned int)PHYS_BASE||arg1=='\0'){
                exit(-1);
                return;
            }
			if(get_user(arg1)==-1){
			    exit(-1);
			    return;
			}
	    	ret = open ((char *)arg1);
	    	break;
	    case SYS_FILESIZE:               /* Obtain a file's size. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
	    	ret = filesize((int)arg1);
	    	break;
	    case SYS_READ:                   /* Read from a file. */
            if((unsigned int)(esp+3)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if(get_user(esp+2)==-1){
                exit(-1);
                 return;
            }
			arg2 = esp[2];
            if((void*)arg2==NULL||(unsigned int)(void*)arg2>=(unsigned int)PHYS_BASE||arg1=='\0'){
                exit(-1);
                return;
            }
            if(get_user(arg2)==-1){
                exit(-1);
                return;
            }
            if(get_user(esp+3)==-1){
                exit(-1);
                 return;
            }
			arg3 = esp[3];
			ret = read((int)arg1, (void *)arg2, (unsigned)arg3);
	    	break;
	    case SYS_WRITE:                  /* Write to a file. */
            if((unsigned int)(esp+3)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if(get_user(esp+2)==-1){
                exit(-1);
                 return;
            }
			arg2 = esp[2];
            if((void*)arg2==NULL||(unsigned int)(void*)arg2>=(unsigned int)PHYS_BASE||arg2=='\0'){
                exit(-1);
                return;
            }
            if(get_user(arg2)==-1){
                exit(-1);
                return;
            }
            if(get_user(esp+3)==-1){
                exit(-1);
                 return;
            }
			arg3 = esp[3];
	    	ret = write((int)arg1, (void *)arg2,(unsigned)arg3);
	    	break;
	    case SYS_SEEK:                   /* Change position in a file. */
            if((unsigned int)(esp+2)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
            if(get_user(esp+2)==-1){
                exit(-1);
                 return;
            }
			arg2 = esp[2];
	    	seek((int)arg1, (unsigned)arg2);
	    	break;
	    case SYS_TELL:                   /* Report current position in a file. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
	    	ret = tell((int)arg1);
	    	break;
		case SYS_CLOSE:                  /* Close a file. */
            if((unsigned int)(esp+1)>=(unsigned int)PHYS_BASE){
                exit(-1);
                return;
            }
            if(get_user(esp+1)==-1){
                exit(-1);
                 return;
            }
			arg1 = esp[1];
			close((int)arg1);
			break;
	    default:
			//printf ("system call!\n");
			exit(-1);
			break;
  			//thread_exit ();
	    	break;
	}
	f->eax = ret;
}


/*Terminates Pintos by calling shutdown_power_off() 
  (declared in devices/shutdown.h). 
  This should be seldom used, 
  because you lose some information about possible deadlock situations, etc.*/
void halt (void){
	power_off();
}

/*Terminates the current user program, returning status to the kernel.
  If the process's parent waits for it (see below),
  this is the status that will be returned. Conventionally,
  a status of 0 indicates success and nonzero values indicate errors.*/
void exit (int status){
  //update status and exit thread
  thread_current()->exit_status = status;
  thread_exit();
}

/*Runs the executable whose name is given in cmd_line, passing any given arguments,
  and returns the new process's program id (pid). Must return pid -1,
  which otherwise should not be a valid pid, if the program cannot load or run for any reason.
  Thus, the parent process cannot return from the exec until it knows
  whether the child process successfully loaded its executable.
  You must use appropriate synchronization to ensure this.*/
pid_t exec (const char *cmd_line){	
	return process_execute(cmd_line);
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
  return process_wait(pid);
}

/* Creates a new file called file initially initial_size bytes in size. 
  Returns true if successful, false otherwise. 
  Creating a new file does not open it: opening the new file is 
  a separate o,peration which would require a open system call.*/
bool create (const char *file, unsigned initial_size){
	return filesys_create(file, initial_size);
}

/* Deletes the file called file. 
  Returns true if successful, false otherwise. 
  A file may be removed regardless of whether it is open or closed, 
  and removing an open file does not close it. 
  See FAQ: Removing an Open File, for details.*/
bool remove (const char *file){
	return filesys_remove(file);
}

/* Opens the file called file. 
  Returns a nonnegative integer handle called a 
  "file descriptor" (fd), or -1 if the file could not be opened.*/
int open (const char *file){
    static int fl=1;
    static int fdtmp=2,fd=2;
    static struct lock fd_lock;
    static struct file *file_op;
    if(fl==1){
        fl=0;
        lock_init(&fd_lock);
    }
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i] != -1){
		i++;
		if (i==100){
			return -1;
		}
    }
    if (fd==FD_MAX){
    	return -1;
	}
    file_op=filesys_open(file);
    if(file_op==NULL) return -1;
    lock_acquire(&fd_lock);
    fd = fdtmp++;
    fd_con[fd]=file_op;
    valid=fd;
    lock_release(&fd_lock);
    curr->openfd[i]=fd;
    return fd;
}

/* Returns the size, in bytes, of the file open as fd.*/
int filesize (int fd){
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i]!= fd){
        i++;
        if (i==100){
    	    exit(-1);
        }
    }    
    return file_length(fd_con[fd]);
}

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read (0 at end of file),
   or -1 if the file could not be read (due to a condition other than end of file).
   Fd 0 reads from the keyboard using input_getc().*/
int read (int fd, void *buffer, unsigned size){
    if(fd>=1) {
        int i=0;
    	struct thread *curr = thread_current();
    	while(curr->openfd[i]!= fd){
    	    i++;
    	    if (i==100){
    		exit(-1);
	    }
	}
	return file_read(fd_con[fd], buffer, size);
    }
    uint8_t k, *buf;
    unsigned i=0;
    k=input_getc();
    buf=(uint8_t *)buffer;
    buf[i++]=k;
    while(k!='\n'&&i<size){//k's condition not satisfied. may be modified.
        k=input_getc();
        buf[i++]=k;
    }
    return i;
}

/* Writes size bytes from buffer to the open file fd.
  Returns the number of bytes actually written, 
  which may be less than size if some bytes could not be written*/
int write (int fd, const void *buffer, unsigned size){
    if(fd==0||fd>valid)return -1;
    if(fd==1){
	if(size>100){
	   unsigned int k=0;
	   while (k<size){
		unsigned int min;
		min= size-k>100 ? 100 : size-k;
		putbuf(buffer+k, min);
		k+=100;
	   }
	}
        else putbuf(buffer, size);
        return size;
    }
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i]!= fd){
    	i++;
    	if (i==100){
    	    exit(-1);
    	}
    }
    return file_write(fd_con[fd], buffer, size);
}

/*Changes the next byte to be read or written in open file fd to position,
  expressed in bytes from the beginning of the file.
  (Thus, a position of 0 is the file's start.)*/
void seek (int fd, unsigned position){
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i]!= fd){
    	i++;
    	if (i==100){
            exit(-1);
    	}
    }
    file_seek(fd_con[fd], position);
    return;
}

/*Returns the position of the next byte to be read or written in open file fd,
  expressed in bytes from the beginning of the file.*/
unsigned tell (int fd){
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i]!= fd){
    	i++;
    	if (i==100){
    	    exit(-1);
	}
    }
    return file_tell(fd_con[fd]);
}

/*Closes file descriptor fd.
  Exiting or terminating a process implicitly
  closes all its open file descriptors,
  as if by calling this function for each one.*/
void close (int fd){
    int i=0;
    struct thread *curr = thread_current();
    while(curr->openfd[i]!= fd){
    	i++;
    	if (i==100){
    	    exit(-1);
	}
    }
    curr->openfd[i] = -1;
    file_close(fd_con[fd]);
    return;
}
void* fmap[2000];
int mapsize[2000];
int mapfd[2000];
struct lock map_lock;
static mapid_t map=-1;
mapid_t mmap(int fd, void *addr){
   if(fd==0||fd==1) return -1;
   if((int)addr==0) return -1;
   if(filesize(fd)==0) return -1;
   if(!is_user_vaddr(addr)) return -1;//addr not user virtual address
   int i,fdsize=filesize(fd);
   int fd_pgsize=((fdsize+PGSIZE-1)/PGSIZE)*PGSIZE;
   for(i=0;i<fd_pgsize;i++)
       if(find_page(addr+i)!=NULL)return -1;//the range of pages mapped overlaps
   //connects virtual address& palloced pages : add_connect(addr, palloc_get_multiple(PAL_USER|PAL_ZERO, fd_pgsize/PGSIZE);
   read(fd,addr,fdsize);
   lock_acquire(map_lock);
	map+=1;
	mapsize[map]=fd_pgsize/PGSIZE;
	fmap[map]=addr;
	mapfd[map]=fd;
   lock_release(map_lock);
   return map;
}

void munmap(mapid_t mapping){
    lock_acquire(map_lock);
    void* addr=fmap[mapping];
    int i, size=mapsize[mapping],fd=mapfd[mapping];
    lock_release(map_lock);
    uint32_t *pd=current_thread().pagedir;
    for(i=0;i<size;i++){
        if(pagedir_is_dirty(pd, addr+i*PGSIZE)){
	    write(fd, addr, size*PGSIZE);
	    break;
        }
    }
    palloc_free_multiple(addr, size);
}
