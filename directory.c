#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "filesys/free-map.h"


/* A single directory entry. */
struct dir_entry 
  {
    disk_sector_t inode_sector;         /* Sector number of header. */
    char name[NAME_MAX + 1];            /* Null terminated file name. */
    bool in_use;                        /* In use or free? */
  };

struct inode *go_dir(struct dir *d, char **name){
	char *buf = *name;
	char dirname[NAME_MAX+1];
	int i;
	for(i=0; i<NAME_MAX+1; i++){
		if (buf[i] == '\0' || buf[i] == '/'){
			int j;
			for (j=0; j<i;j++){
				dirname[j] = buf[j]; 
			}
			dirname[i] = '\0';
			char *t = buf + i;
			*name = t;
			break;
		}
	}
	if (i == NAME_MAX+1){
		return NULL;
	}
	if (strcmp(dirname,".")==0){
		return inode_reopen(d->inode);
	}
	else if (strcmp(dirname, "..")==0){
		return open_parent_dir(d->inode);
	}
	else{
		struct inode *inode = NULL;
		//printf("godir:%s %d\n",dirname, inode_get_inumber(d->inode));
		dir_lookup (d, dirname, &inode);
		return inode;
	}
}



/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool
dir_create (disk_sector_t sector, size_t entry_cnt, disk_sector_t p_dir) 
{
  return inode_create (sector, entry_cnt * sizeof (struct dir_entry), 1, p_dir);
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open (struct inode *inode) 
{
  struct dir *dir = calloc (1, sizeof *dir);
  if (inode != NULL && dir != NULL && inode_is_dir(inode))
    {
      dir->inode = inode;
      dir->pos = 0;
      return dir;
    }
  else
    {
      inode_close (inode);
      free (dir);
      return NULL; 
    }
}

struct dir *
dirsys_open(char *name){
	if (strcmp(name,"/")==0){
		return dir_open_root();
	}
	if (strcmp(name,".")==0){
		if (thread_current()->dir->inode->removed){
			return NULL;
		}
		return dir_reopen(thread_current()->dir);
	}
	if (strcmp(name,"..")==0){
		if (thread_current()->dir->inode->removed){
			return NULL;
		}
		return dir_open(open_parent_dir(thread_current()->dir->inode));
	}
	char *buf = name;
    struct dir *dir = get_dir(&buf);
	struct inode *inode = NULL;
	if (dir != NULL)
		dir_lookup (dir, buf, &inode);
	dir_close (dir);
	return dir_open (inode);
}

struct dir *get_dir(char **name){
	//printf("ggg : %s\n", *name);
	struct dir *d;
	char *buf = *name;
	if (buf[0] == '/'){
		d = dir_open_root();
		while (buf[0] == '/'){
			buf +=1 ;
		}
	}
	else{
		if (thread_current()->dir == NULL){
			d = dir_open_root();
		}
		else {
			d = dir_reopen(thread_current()->dir);
		}
	}
	if (d == NULL){
		return NULL;
	}
	while (true){
		//printf("get dir1 : %d\n",inode_get_inumber(d->inode));
		char *old_buf = buf;
		struct inode *t = go_dir(d,&buf);
		if (buf[0] == '\0'){
			inode_close(t);
			*name = old_buf;
			//printf("get dir2 : %d\n",inode_get_inumber(d->inode));
			if (d->inode->removed){
				dir_close(d);
				return NULL;
			}
			return d;
		}
		dir_close(d);
		d= dir_open(t);
		if (d==NULL){
			return NULL;
		}
		while (buf[0] == '/')
			buf++;
	}
}



bool make_dir(char *name){
	struct dir *d = get_dir(&name);
	disk_sector_t inode_sector = 0;
	bool success = (d != NULL
                  && free_map_allocate (1, &inode_sector)
                  && dir_create (inode_sector, 16, inode_get_inumber (d->inode))
                  && dir_add (d, name, inode_sector));
	if (!success && inode_sector != 0) {
		free_map_release (inode_sector, 1);
	}
	dir_close (d);
	return success;
}


/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir *
dir_open_root (void)
{
  return dir_open (inode_open (ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir *
dir_reopen (struct dir *dir) 
{
  return dir_open (inode_reopen (dir->inode));
}

/* Destroys DIR and frees associated resources. */
void
dir_close (struct dir *dir) 
{
  if (dir != NULL)
    {
      inode_close (dir->inode);
      free (dir);
    }
}

/* Returns the inode encapsulated by DIR. */
struct inode *
dir_get_inode (struct dir *dir) 
{
  return dir->inode;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool
lookup (const struct dir *dir, const char *name,
        struct dir_entry *ep, off_t *ofsp) 
{
  struct dir_entry e;
  size_t ofs;
  
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e) 
    if (e.in_use && !strcmp (name, e.name)) 
      {
        if (ep != NULL)
          *ep = e;
        if (ofsp != NULL)
          *ofsp = ofs;
        return true;
      }
  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool
dir_lookup (const struct dir *dir, const char *name,
            struct inode **inode) 
{
  struct dir_entry e;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  if (lookup (dir, name, &e, NULL))
    *inode = inode_open (e.inode_sector);
  else
    *inode = NULL;

  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool
dir_add (struct dir *dir, const char *name, disk_sector_t inode_sector) 
{
  struct dir_entry e;
  off_t ofs;
  bool success = false;
  
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  /* Check NAME for validity. */
  if (*name == '\0' || strlen (name) > NAME_MAX)
    return false;

  /* Check that NAME is not in use. */
  if (lookup (dir, name, NULL, NULL))
    goto done;

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.
     
     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */
  for (ofs = 0; inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e) 
    if (!e.in_use)
      break;

  /* Write slot. */
  e.in_use = true;
  strlcpy (e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  success = inode_write_at (dir->inode, &e, sizeof e, ofs) == sizeof e;

 done:
  return success;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool
dir_remove (struct dir *dir, const char *name) 
{
  struct dir_entry e;
  struct inode *inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  /* Find directory entry. */
  if (!lookup (dir, name, &e, &ofs))
    goto done;

  /* Open inode. */
  inode = inode_open (e.inode_sector);
  
  if (inode_is_dir(inode)){
  	int i = 0;
  	while (inode_read_at(inode, &e,sizeof e, i) == sizeof e){
  		if  (e.in_use){
  			goto done;
		  }
		  i += sizeof e;
	  }
  }
  
  if (inode == NULL)
    goto done;

  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at (dir->inode, &e, sizeof e, ofs) != sizeof e) 
    goto done;

  /* Remove inode. */
  inode_remove (inode);
  success = true;

 done:
  inode_close (inode);
  return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool
dir_readdir (struct dir *dir, char name[NAME_MAX + 1])
{
  struct dir_entry e;

  while (inode_read_at (dir->inode, &e, sizeof e, dir->pos) == sizeof e) 
    {
      dir->pos += sizeof e;
      if (e.in_use)
        {
          strlcpy (name, e.name, NAME_MAX + 1);
          return true;
        } 
    }
  return false;
}
