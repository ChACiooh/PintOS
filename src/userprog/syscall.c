#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <devices/shutdown.h>
#include <filesys/filesys.h>
#include <filesys/file.h>
#include <devices/input.h>
#include "userprog/process.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void check_address (void* adrr);
void get_argument (int** esp, int* arg, int count);

/* System call function */
void sys_halt (void);
void sys_exit (int status);
bool sys_create (const char *file, unsigned initial_size);
bool sys_remove (const char *file);
tid_t sys_exec (const char *cmd_line);
int sys_wait (tid_t tid);
int sys_open (const char *file);
int sys_filesize (int fd);
int sys_read (int fd, char *buffer, unsigned int size);
int sys_write (int fd, void *buffer, unsigned int size);
void sys_seek (int fd, unsigned int position);
int sys_tell (int fd);
void sys_close (int fd);

void
syscall_init (void) 
{
  lock_init(&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

 /* Check effectiveness of address */
void
check_address (void *addr)
{
  if((unsigned int)addr <= 0x08048000 || (unsigned int)addr >= 0xc0000000)
  {
    sys_exit(-1);
  }
}

 /* Get argument from esp function */
void
get_argument (int **esp, int *arg, int count)
{
  int i = 0;
  for(; i < count; i++)
  {
    (*esp)++;
    check_address(*esp);
    arg[i] = **esp;
  }    
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int* esp = (int*)(f->esp);
  int system_call_number = *esp;
  int arg[3];	//argument 
  check_address((void*)esp);
   /* System call */
  switch(system_call_number)
  {
    /* About Process System call */
    case SYS_HALT:
      sys_halt();
      break;
    case SYS_EXIT:
      get_argument(&esp, arg, 1);
      sys_exit(arg[0]);
      break;
    case SYS_EXEC:
      get_argument(&esp, arg, 1);
      check_address((void*)arg[0]);
      f->eax = sys_exec((const char *)arg[0]);
      break;
    case SYS_WAIT:
      get_argument(&esp, arg, 1);
      f->eax = sys_wait(arg[0]);
      break;
    /* About FILE Descriptor */
    case SYS_CREATE:
      get_argument(&esp, arg, 2);
      check_address((void*)arg[0]);
      f->eax = sys_create((const char*)arg[0], arg[1]);
      break;
    case SYS_REMOVE:
      get_argument(&esp, arg, 1);
      check_address((void*)arg[0]);
      f->eax = sys_remove((const char*)arg[0]);
      break;
    case SYS_OPEN:
      get_argument(&esp, arg, 1);
      check_address((void*)arg[0]);
      f->eax = sys_open((char*)arg[0]);
      break;
    case SYS_FILESIZE:
      get_argument(&esp, arg, 1);
      f->eax = sys_filesize(arg[0]);
      break;
    case SYS_READ:
      get_argument(&esp, arg, 3);
      check_address((void*)arg[1]);
      f->eax = sys_read(arg[0],(char*)arg[1],arg[2]);
      break;
    case SYS_WRITE:
      get_argument(&esp, arg, 3);
      check_address((void*)arg[1]);
      f->eax = sys_write(arg[0],(void*)arg[1],arg[2]); 
      break;
    case SYS_SEEK:
      get_argument(&esp, arg, 2);
      sys_seek(arg[0], arg[1]);
      break;
    case SYS_TELL:
      get_argument(&esp, arg, 1);
      f->eax = sys_tell(arg[0]);
      break;
    case SYS_CLOSE:
      get_argument(&esp, arg, 1);
      sys_close(arg[0]);
      break;
    default:	//if invaild system call -> exit(-1)
      sys_exit(-1);
  }
}

 /* Program quit */
void
sys_halt (void)
{
  shutdown_power_off();
}

 /* Thread Exit */
void
sys_exit (int status)
{
  struct thread *tmp = thread_current();
  tmp->exit_status = status;
  printf("%s: exit(%d)\n", tmp->name, status);
  thread_exit();  
}

 /* Create file */
bool
sys_create (const char *file, unsigned initial_size)
{
  bool result;
  lock_acquire(&filesys_lock);
  result = filesys_create(file, initial_size);
  lock_release(&filesys_lock);
  return result;
}

 /* Remove file */
bool
sys_remove (const char *file)
{
  bool result;
  lock_acquire(&filesys_lock);
  result = filesys_remove(file);
  lock_release(&filesys_lock);
  return result;
}

 /* Execute process */
tid_t
sys_exec (const char* cmd_line)
{
  tid_t child_id = process_execute(cmd_line);
  struct thread* child = get_child_process(child_id);
  if(child == NULL)	//if get_child_process fail -> return -1;
    return -1;
  sema_down(&(child->s_load));
  if(child->load_status == -1)
    return -1;
  return child_id;
}

 /* Wait Child process */
int
sys_wait (tid_t tid)
{
  return process_wait(tid);
}

 /* File open */
int
sys_open (const char *file)
{
  struct file *f = filesys_open(file);
  if(f == NULL)
    return -1;
  return process_add_file(f);
}

 /* Return file size */
int
sys_filesize (int fd)
{
  struct file *f;
  int len = -1;
  lock_acquire(&filesys_lock);
  f = process_get_file(fd);
  if(f != NULL)
    len = file_length(f);  
  lock_release(&filesys_lock);
  return len;
}

 /* File read */
int
sys_read (int fd, char *buffer, unsigned int size)
{
  struct file *f;
  int count = -1;
  lock_acquire(&filesys_lock);
  if(fd == 0)	//if File Descriptor is STDIN -> input by keyboard
  {
    for(count = 0; count < size; count++)
      buffer[count] = input_getc();
  }
  else	//Read from file
  {
    f = process_get_file(fd);
    if(f != NULL)
      count = file_read(f, buffer, size);
  }
  lock_release(&filesys_lock);
  return count;
}

 /* File write */
int
sys_write (int fd, void *buffer, unsigned int size)
{
  struct file *f;
  int count = -1;
  lock_acquire(&filesys_lock);
  if(fd == 1)	//if File Descriptor is STDOUT -> output by screan
  {
    putbuf((char*)buffer, size);
    count = size;
  }
  else	//Write on file
  {
    f = process_get_file(fd);
    if(f != NULL)
      count = file_write(f, buffer, size);
  }
  lock_release(&filesys_lock);
  return count;
}

 /* Move file offset */ 
void
sys_seek (int fd, unsigned int position)
{
  struct file* f;
  lock_acquire(&filesys_lock);
  f = process_get_file(fd);
  if(f != NULL)
    file_seek(f, position);
  lock_release(&filesys_lock);
}

 /* Return file offset */
int
sys_tell (int fd)
{
  struct file* f;
  int pos = -1;
  lock_acquire(&filesys_lock);
  f = process_get_file(fd);
  if(f != NULL)
    pos = file_tell(f);
  lock_release(&filesys_lock);
  return pos;
}

 /* Close file */
void
sys_close (int fd)
{
  lock_acquire(&filesys_lock);
  process_close_file(fd);
  lock_release(&filesys_lock);
}
