#include "vm/page.h"
#include <debug.h>
#include <stdbool.h>
#include <string.h>
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include <stdio.h>

static unsigned vm_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
	/* search vm_entry with hash_elem */
	struct vm_entry *vm = hash_entry(e, struct vm_entry, elem);
	unsigned hash_val = hash_int((int)(vm->vaddr));	// returned value of the hash.
	return hash_val;
}

static bool vm_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
	/* compare each vaddr, and if b's is bigger, return true. */
	struct vm_entry *va = hash_entry(a, struct vm_entry, elem);
	struct vm_entry *vb = hash_entry(b, struct vm_entry, elem);

	return (unsigned)(va->vaddr) < (unsigned)(vb->vaddr);
}

static void vm_destroy_func (struct hash_elem *e, void *aux UNUSED)
{
	struct vm_entry *ve = hash_entry(e, struct vm_entry, elem);
	if(ve->is_loaded)
	{
		palloc_free_page(pagedir_get_page(thread_current()->pagedir, ve->vaddr));
		pagedir_clear_page(thread_current()->pagedir, ve->vaddr);
	}
	free(ve);	// suggest that ve is created by malloc.
}

void vm_init (struct hash *vm)
{
	/* Initialize hash. */
	bool success = hash_init(vm, vm_hash_func, vm_less_func, NULL);
	ASSERT (success);
}

bool insert_vme (struct hash *vm, struct vm_entry *vme)
{
	if(!vm || !vme)	return false;	// exception dealing.
	struct hash_elem *hep = hash_insert(vm, &vme->elem);
	// if it succeeded, return NULL.
	return hep == NULL;
}

bool delete_vme (struct hash *vm, struct vm_entry *vme)
{
	if(!vm || !vme)	return false;	// exception dealing.
	struct hash_elem *hep = hash_delete(vm, &vme->elem);
	// if it succeeded, return sometihing not NULL.
	return hep != NULL;
}

struct vm_entry *find_vme (void *vaddr)
{
	struct vm_entry *ve = (struct vm_entry*)malloc(sizeof(struct vm_entry));
	ve->vaddr = pg_round_down(vaddr);	// extract the page number of virtual address.

	struct hash_elem *he = hash_find(&thread_current()->vm, &ve->elem);	// and search by it.
	free(ve);

	return he != NULL ? hash_entry(he, struct vm_entry, elem) : NULL;
}

void vm_destroy (struct hash *vm)
{
	hash_destroy(vm, vm_destroy_func);
}

bool load_file (void* kaddr, struct vm_entry *vme)
{
	ASSERT(vme != NULL);
	ASSERT(kaddr);

	if( file_read_at( vme->file, kaddr, vme->read_bytes, vme->offset ) != (int)vme->read_bytes )
	{
		printf("load_file failed.\n");
		return false;
	}

	/* fill with zero into remained space of buffer. */
	memset( kaddr+vme->read_bytes, 0, vme->zero_bytes );
	return true;
}
