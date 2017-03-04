/*
 * Persistent Storage - pstore.h
 *
 * Copyright (C) 2010 Intel Corporation <tony.luck@intel.com>
 *
 * This code is the generic layer to export data records from platform
 * level persistent storage via a file system.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _LINUX_PSTORE_H
#define _LINUX_PSTORE_H

#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/kmsg_dump.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/types.h>

struct module;

/* types */
enum pstore_type_id {
	PSTORE_TYPE_DMESG	= 0,
	PSTORE_TYPE_MCE		= 1,
	PSTORE_TYPE_CONSOLE	= 2,
	PSTORE_TYPE_FTRACE	= 3,
	/* PPC64 partition types */
	PSTORE_TYPE_PPC_RTAS	= 4,
	PSTORE_TYPE_PPC_OF	= 5,
	PSTORE_TYPE_PPC_COMMON	= 6,
	PSTORE_TYPE_PMSG	= 7,
	PSTORE_TYPE_PPC_OPAL	= 8,
	PSTORE_TYPE_UNKNOWN	= 255
};

struct pstore_info;
/**
 * struct pstore_record - details of a pstore record entry
 * @psi:	pstore backend driver information
 * @type:	pstore record type
 * @id:		per-type unique identifier for record
 * @time:	timestamp of the record
 * @buf:	pointer to record contents
 * @size:	size of @buf
 * @ecc_notice_size:
 *		ECC information for @buf
 *
 * Valid for PSTORE_TYPE_DMESG @type:
 *
 * @count:	Oops count since boot
 * @reason:	kdump reason for notification
 * @part:	position in a multipart record
 * @compressed:	whether the buffer is compressed
 *
 */
struct pstore_record {
	struct pstore_info	*psi;
	enum pstore_type_id	type;
	u64			id;
	struct timespec		time;
	char			*buf;
	ssize_t			size;
	ssize_t			ecc_notice_size;

	int			count;
	enum kmsg_dump_reason	reason;
	unsigned int		part;
	bool			compressed;
};

struct pstore_info {
	struct module	*owner;
	char		*name;
	spinlock_t	buf_lock;	/* serialize access to 'buf' */
	char		*buf;
	size_t		bufsize;
	struct mutex	read_mutex;	/* serialize open/read/close */
	int		flags;
	int		(*open)(struct pstore_info *psi);
	int		(*close)(struct pstore_info *psi);
	ssize_t		(*read)(struct pstore_record *record);
	int		(*write)(struct pstore_record *record);
	int		(*write_buf)(enum pstore_type_id type,
			enum kmsg_dump_reason reason, u64 *id,
			unsigned int part, const char *buf, bool compressed,
			size_t size, struct pstore_info *psi);
	int		(*write_buf_user)(enum pstore_type_id type,
			enum kmsg_dump_reason reason, u64 *id,
			unsigned int part, const char __user *buf,
			bool compressed, size_t size, struct pstore_info *psi);
	int		(*erase)(enum pstore_type_id type, u64 id,
			int count, struct timespec time,
			struct pstore_info *psi);
	void		*data;
};

#define PSTORE_FLAGS_DMESG	(1 << 0)
#define PSTORE_FLAGS_FRAGILE	PSTORE_FLAGS_DMESG
#define PSTORE_FLAGS_CONSOLE	(1 << 1)
#define PSTORE_FLAGS_FTRACE	(1 << 2)
#define PSTORE_FLAGS_PMSG	(1 << 3)

extern int pstore_register(struct pstore_info *);
extern void pstore_unregister(struct pstore_info *);
extern bool pstore_cannot_block_path(enum kmsg_dump_reason reason);

#endif /*_LINUX_PSTORE_H*/
