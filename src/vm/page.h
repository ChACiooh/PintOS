#ifndef PAGE_H
#define PAGE_H
#pragma once

#include <hash.h>

#define VM_BIN		0
#define VM_FILE		1
#define VM_ANON		2


struct vm_entry {
	uint8_t type;		/* type of VM_BIN, VM_FILE, VM_ANON */
	void *vaddr;		/* virtual address managed by vm_entry. */
						/* and extract hash value with this. */
	bool writable;		/* enable write to the address if it is true. */

	bool is_loaded;
	struct file *file;

	/* Preserved variable for Memory Mapped File. */
	struct list_elem mmap_elem;		/* mmap list element. */

	size_t offset;
	size_t read_bytes;
	size_t zero_bytes;

	/* Preserved variable for Swapping progress. */
	size_t swap_slot;

	/* Preserved variable for 'Data Structures for vm_entry.' */
	struct hash_elem elem;		/* Hash table elemtn. */
};

void vm_init (struct hash *);

/* reserved for initialization of virtual address space. */
bool insert_vme(struct hash *, struct vm_entry *);
bool delete_vme(struct hash *, struct vm_entry *);

/* reserved for implementation of demanding page. */
struct vm_entry *find_vme (void *);

void vm_destroy (struct hash *);

bool load_file (void* kaddr, struct vm_entry *vme);

#endif	// PAGE_H_INCLUDED
