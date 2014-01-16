/**
 * @file
 *
 * @brief Header File for the In-Memory File System
 */

/*
 *  COPYRIGHT (c) 1989-2011.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _RTEMS_IMFS_H
#define _RTEMS_IMFS_H

#include <limits.h>

#include <rtems/libio_.h>
#include <rtems/pipe.h>

/**
 * @brief In-Memory File System Support.
 *
 * @defgroup IMFS In-Memory File System Support
 *
 * @ingroup FileSystemTypesAndMount
 */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Data types
 */

struct IMFS_jnode_tt;
typedef struct IMFS_jnode_tt IMFS_jnode_t;

typedef struct {
  rtems_chain_control                    Entries;
  rtems_filesystem_mount_table_entry_t  *mt_fs;
}  IMFS_directory_t;

typedef struct {
  rtems_device_major_number  major;
  rtems_device_minor_number  minor;
}  IMFS_device_t;

typedef struct {
  IMFS_jnode_t  *link_node;
} IMFS_link_t;

typedef struct {
  char *name;
} IMFS_sym_link_t;

typedef struct {
  pipe_control_t  *pipe;
} IMFS_fifo_t;

typedef struct {
  void *context;
} IMFS_generic_t;

/**
 *  IMFS "memfile" information
 *
 *  The data structure for the in-memory "memfiles" is based on classic UNIX.
 *
 *  block_ptr is a pointer to a block of IMFS_MEMFILE_BYTES_PER_BLOCK in
 *  length which could be data or a table of pointers to blocks.
 *
 *  Setting IMFS_MEMFILE_BYTES_PER_BLOCK to different values has a significant
 *  impact on the maximum file size supported as well as the amount of
 *  memory wasted due to internal file fragmentation.  The following
 *  is a list of maximum file sizes based on various settings
 *
 *  @code
 *    max_filesize with blocks of   16 is         1,328
 *    max_filesize with blocks of   32 is        18,656
 *    max_filesize with blocks of   64 is       279,488
 *    max_filesize with blocks of  128 is     4,329,344
 *    max_filesize with blocks of  256 is    68,173,568
 *    max_filesize with blocks of  512 is 1,082,195,456
 *  @endcode
 */
#define IMFS_MEMFILE_DEFAULT_BYTES_PER_BLOCK     128
  extern int imfs_rq_memfile_bytes_per_block;
  extern int imfs_memfile_bytes_per_block;

#define IMFS_MEMFILE_BYTES_PER_BLOCK imfs_memfile_bytes_per_block
#define IMFS_MEMFILE_BLOCK_SLOTS \
  (IMFS_MEMFILE_BYTES_PER_BLOCK / sizeof(void *))

typedef uint8_t *block_p;
typedef block_p *block_ptr;

typedef struct {
  off_t         size;             /* size of file in bytes */
  block_ptr     indirect;         /* array of 128 data blocks pointers */
  block_ptr     doubly_indirect;  /* 128 indirect blocks */
  block_ptr     triply_indirect;  /* 128 doubly indirect blocks */
} IMFS_memfile_t;

typedef struct {
  off_t         size;             /* size of file in bytes */
  block_p       direct;           /* pointer to file image */
} IMFS_linearfile_t;

/*
 *  Important block numbers for "memfiles"
 */
#define FIRST_INDIRECT           (0)
#define LAST_INDIRECT            (IMFS_MEMFILE_BLOCK_SLOTS - 1)

#define FIRST_DOUBLY_INDIRECT    (LAST_INDIRECT + 1)
#define LAST_DOUBLY_INDIRECT     \
   (LAST_INDIRECT + \
     (IMFS_MEMFILE_BLOCK_SLOTS * IMFS_MEMFILE_BLOCK_SLOTS))

#define FIRST_TRIPLY_INDIRECT    (LAST_DOUBLY_INDIRECT + 1)
#define LAST_TRIPLY_INDIRECT \
   (LAST_DOUBLY_INDIRECT +\
     (IMFS_MEMFILE_BLOCK_SLOTS * \
        IMFS_MEMFILE_BLOCK_SLOTS * IMFS_MEMFILE_BLOCK_SLOTS))

#define IMFS_MEMFILE_MAXIMUM_SIZE \
  (LAST_TRIPLY_INDIRECT * IMFS_MEMFILE_BYTES_PER_BLOCK)

/*
 *  What types of IMFS file systems entities there can be.
 */
typedef enum {
  IMFS_DIRECTORY = RTEMS_FILESYSTEM_DIRECTORY,
  IMFS_DEVICE = RTEMS_FILESYSTEM_DEVICE,
  IMFS_HARD_LINK = RTEMS_FILESYSTEM_HARD_LINK,
  IMFS_SYM_LINK =  RTEMS_FILESYSTEM_SYM_LINK,
  IMFS_MEMORY_FILE = RTEMS_FILESYSTEM_MEMORY_FILE,
  IMFS_LINEAR_FILE,
  IMFS_FIFO,
  IMFS_GENERIC,
  IMFS_INVALID_NODE
} IMFS_jnode_types_t;

/* The IMFS_GENERIC does not count */
#define IMFS_TYPE_COUNT (IMFS_FIFO + 1)

typedef union {
  IMFS_directory_t   directory;
  IMFS_device_t      device;
  IMFS_link_t        hard_link;
  IMFS_sym_link_t    sym_link;
  IMFS_memfile_t     file;
  IMFS_linearfile_t  linearfile;
  IMFS_fifo_t        fifo;
  IMFS_generic_t     generic;
} IMFS_types_union;

/** @} */

/**
 * @addtogroup IMFSGenericNodes
 */
/**@{*/

/**
 * @brief Initializes an IMFS node.
 *
 * @param[in,out] node The IMFS node.
 * @param[in] info The IMFS type information.
 *
 * @retval node Successful operation.
 * @retval NULL An error occurred.  The @c errno indicates the error.  This
 * will abort the make operation.
 *
 * @see IMFS_node_control, IMFS_node_initialize_default(), and
 * IMFS_node_initialize_generic().
 */
typedef IMFS_jnode_t *(*IMFS_node_control_initialize)(
  IMFS_jnode_t *node,
  const IMFS_types_union *info
);

/**
 * @brief Returns the node and does nothing else.
 *
 * @param[in,out] node The IMFS node.
 * @param[in] info The IMFS type information.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control.
 */
IMFS_jnode_t *IMFS_node_initialize_default(
  IMFS_jnode_t *node,
  const IMFS_types_union *info
);

/**
 * @brief Returns the node and sets the generic node context.
 *
 * @param[in,out] node The IMFS node.
 * @param[in] info The IMFS type information.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control.
 */
IMFS_jnode_t *IMFS_node_initialize_generic(
  IMFS_jnode_t *node,
  const IMFS_types_union *info
);

/**
 * @brief Prepares the removal of an IMFS node from its parent directory.
 *
 * @param[in,out] node The IMFS node.
 *
 * @retval node Successful operation.
 * @retval NULL An error occurred.  The @c errno indicates the error.  This
 * will abort the removal operation.
 *
 * @see IMFS_node_control and IMFS_node_remove_default().
 */
typedef IMFS_jnode_t *(*IMFS_node_control_remove)(
  IMFS_jnode_t *node
);

/**
 * @brief Returns the node and does nothing else.
 *
 * @param[in,out] node The IMFS node.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control.
 */
IMFS_jnode_t *IMFS_node_remove_default(
  IMFS_jnode_t *node
);

/**
 * @brief Destroys an IMFS node.
 *
 * @param[in,out] node The IMFS node.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control and IMFS_node_destroy_default().
 */
typedef IMFS_jnode_t *(*IMFS_node_control_destroy)( IMFS_jnode_t *node );

/**
 * @brief Returns the node and does nothing else.
 *
 * @param[in,out] node The IMFS node.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control.
 */
IMFS_jnode_t *IMFS_node_destroy_default( IMFS_jnode_t *node );

/**
 * @brief IMFS node control.
 */
typedef struct {
  IMFS_jnode_types_t imfs_type;
  const rtems_filesystem_file_handlers_r *handlers;
  IMFS_node_control_initialize node_initialize;
  IMFS_node_control_remove node_remove;
  IMFS_node_control_destroy node_destroy;
} IMFS_node_control;

/** @} */

/**
 * @addtogroup IMFS
 */
/**@{*/

/*
 * Major device number for the IMFS. This is not a real device number because
 * the IMFS is just a file system and does not have a driver.
 */
#define IMFS_DEVICE_MAJOR_NUMBER (0xfffe)

/**
 * @ingroup IMFSGenericNodes
 *
 * @brief Generic IMFS device major number.
 */
#define IMFS_GENERIC_DEVICE_MAJOR_NUMBER (0xfffd)

/*
 *  Maximum length of a "basename" of an IMFS file/node.
 */

#define IMFS_NAME_MAX  32

/*
 *  The control structure for an IMFS jnode.
 */

struct IMFS_jnode_tt {
  rtems_chain_node    Node;                  /* for chaining them together */
  IMFS_jnode_t       *Parent;                /* Parent node */
  char                name[IMFS_NAME_MAX+1]; /* "basename" */
  mode_t              st_mode;               /* File mode */
  unsigned short      reference_count;
  nlink_t             st_nlink;              /* Link count */
  ino_t               st_ino;                /* inode */

  uid_t               st_uid;                /* User ID of owner */
  gid_t               st_gid;                /* Group ID of owner */

  time_t              stat_atime;            /* Time of last access */
  time_t              stat_mtime;            /* Time of last modification */
  time_t              stat_ctime;            /* Time of last status change */
  const IMFS_node_control *control;
  IMFS_types_union    info;
};

static inline void IMFS_update_atime( IMFS_jnode_t *jnode )
{
  struct timeval now;

  gettimeofday( &now, 0 );

  jnode->stat_atime = now.tv_sec;
}

static inline void IMFS_update_mtime( IMFS_jnode_t *jnode )
{
  struct timeval now;

  gettimeofday( &now, 0 );

  jnode->stat_mtime = now.tv_sec;
}

static inline void IMFS_update_ctime( IMFS_jnode_t *jnode )
{
  struct timeval now;

  gettimeofday( &now, 0 );

  jnode->stat_ctime = now.tv_sec;
}

static inline void IMFS_mtime_ctime_update( IMFS_jnode_t *jnode )
{
  struct timeval now;

  gettimeofday( &now, 0 );

  jnode->stat_mtime = now.tv_sec;
  jnode->stat_ctime = now.tv_sec;
}

typedef struct {
  int instance;
  ino_t ino_count;
  const IMFS_node_control *node_controls [IMFS_TYPE_COUNT];
} IMFS_fs_info_t;

/*
 *  Shared Data
 */

extern const IMFS_node_control IMFS_node_control_directory;
extern const IMFS_node_control IMFS_node_control_device;
extern const IMFS_node_control IMFS_node_control_hard_link;
extern const IMFS_node_control IMFS_node_control_sym_link;
extern const IMFS_node_control IMFS_node_control_memfile;
extern const IMFS_node_control IMFS_node_control_linfile;
extern const IMFS_node_control IMFS_node_control_fifo;
extern const IMFS_node_control IMFS_node_control_enosys;

extern const rtems_filesystem_operations_table miniIMFS_ops;
extern const rtems_filesystem_operations_table IMFS_ops;
extern const rtems_filesystem_operations_table fifoIMFS_ops;

extern const rtems_filesystem_limits_and_options_t  IMFS_LIMITS_AND_OPTIONS;

/*
 *  Routines
 */

extern int IMFS_initialize(
   rtems_filesystem_mount_table_entry_t *mt_entry,
   const void                           *data
);

extern int fifoIMFS_initialize(
  rtems_filesystem_mount_table_entry_t  *mt_entry,
  const void                            *data
);

extern int miniIMFS_initialize(
   rtems_filesystem_mount_table_entry_t *mt_entry,
   const void                           *data
);

/**
 * @brief IMFS initialization support.
 */
extern int IMFS_initialize_support(
  rtems_filesystem_mount_table_entry_t *mt_entry,
  const rtems_filesystem_operations_table *op_table,
  const IMFS_node_control *const node_controls [IMFS_TYPE_COUNT]
);
/**
 * @brief Unmount this instance of IMFS.
 */
extern void IMFS_fsunmount(
   rtems_filesystem_mount_table_entry_t *mt_entry
);

/**
 * @brief RTEMS load tarfs.
 * 
 * This file implements the "mount" procedure for tar-based IMFS
 * extensions.  The TAR is not actually mounted under the IMFS.
 * Directories from the TAR file are created as usual in the IMFS.
 * File entries are created as IMFS_LINEAR_FILE nodes with their nods
 * pointing to addresses in the TAR image.
 *
 * Here we create the mountpoint directory and load the tarfs at
 * that node.  Once the IMFS has been mounted, we work through the
 * tar image and perform as follows:
 *  - For directories, simply call mkdir().  The IMFS creates nodes as
 *    needed.
 *  - For files, we make our own calls to IMFS eval_for_make and
 *    create_node.
 * 
 * TAR file format:
 *
 * @code
 *   Offset   Length                 Contents
 *     0    100  bytes  File name ('\0' terminated, 99 maxmum length)
 *   100      8  bytes  File mode (in octal ascii)
 *   108      8  bytes  User ID (in octal ascii)
 *   116      8  bytes  Group ID (in octal ascii)
 *   124     12  bytes  File size (s) (in octal ascii)
 *   136     12  bytes  Modify time (in octal ascii)
 *   148      8  bytes  Header checksum (in octal ascii)
 *   156      1  bytes  Link flag
 *   157    100  bytes  Linkname ('\0' terminated, 99 maxmum length)
 *   257      8  bytes  Magic PAX ("ustar\0" + 2 bytes padding)
 *   257      8  bytes  Magic GNU tar ("ustar  \0")
 *   265     32  bytes  User name ('\0' terminated, 31 maxmum length)
 *   297     32  bytes  Group name ('\0' terminated, 31 maxmum length)
 *   329      8  bytes  Major device ID (in octal ascii)
 *   337      8  bytes  Minor device ID (in octal ascii)
 *   345    167  bytes  Padding
 *   512   (s+p) bytes  File contents (s+p) := (((s) + 511) & ~511),
 *                      round up to 512 bytes
 * @endcode
 *
 *  Checksum:
 *  @code
 *    int   i, sum;
 *    char *header = tar_header_pointer;
 *
 *    sum = 0;
 *    for (i = 0; i < 512; i++)
 *        sum += 0xFF & header[i];
 * @endcode
 */
extern int rtems_tarfs_load(
   const char *mountpoint,
   uint8_t *tar_image,
   size_t tar_size
);

/**
 * @brief Dump the entire IMFS.
 * 
 * This routine dumps the entire IMFS that is mounted at the root
 * directory.
 *
 * NOTE: Assuming the "/" directory is bad.
 *       Not checking that the starting directory is in an IMFS is bad.
 */
extern void IMFS_dump( void );

/**
 * @brief Get the size of the largest file which can be created
 * using the IMFS memory file type.
 * 
 * Return the size of the largest file which can be created
 * using the IMFS memory file type.
 */
extern int IMFS_memfile_maximum_size( void );

/**
 * @brief Destroy an IMFS node.
 */
extern void IMFS_node_destroy( IMFS_jnode_t *node );

/**
 * @brief Clone an IMFS node.
 */
extern int IMFS_node_clone( rtems_filesystem_location_info_t *loc );

/**
 * @brief Free an IMFS node.
 */
extern void IMFS_node_free( const rtems_filesystem_location_info_t *loc );

/**
 * @brief IMFS Node Type Get the type of an IMFS node.
 * 
 * The following verifies that returns the type of node that the
 * loc refers to.
 */
extern rtems_filesystem_node_types_t IMFS_node_type(
  const rtems_filesystem_location_info_t *loc
);

/**
 * @brief Perform a status processing for the IMFS.
 * 
 * This routine provides a stat for the IMFS file system.
 */
extern int IMFS_stat(
  const rtems_filesystem_location_info_t *loc,
  struct stat *buf
);

/**
 * @brief IMFS evaluation node support.
 */
extern void IMFS_eval_path(
  rtems_filesystem_eval_path_context_t *ctx
);

/**
 * @brief Create a new IMFS link node.
 * 
 * The following rouine creates a new link node under parent with the
 * name given in name.  The link node is set to point to the node at
 * to_loc.
 */
extern int IMFS_link(
  const rtems_filesystem_location_info_t *parentloc,
  const rtems_filesystem_location_info_t *targetloc,
  const char *name,
  size_t namelen
);

/**
 * @brief Change the owner of IMFS.
 * 
 * This routine is the implementation of the chown() system
 * call for the IMFS.
 */
extern int IMFS_chown(
  const rtems_filesystem_location_info_t *loc,
  uid_t owner,
  gid_t group
);

/**
 * @brief Create an IMFS node.
 * 
 * Routine to create a node in the IMFS file system.
 */
extern int IMFS_mknod(
  const rtems_filesystem_location_info_t *parentloc,
  const char *name,
  size_t namelen,
  mode_t mode,
  dev_t dev
);

/**
 * @brief Create a new IMFS node.
 * 
 * Routine to create a new in memory file system node.
 */
extern IMFS_jnode_t *IMFS_allocate_node(
  IMFS_fs_info_t *fs_info,
  const IMFS_node_control *node_control,
  const char *name,
  size_t namelen,
  mode_t mode,
  const IMFS_types_union *info
);

/**
 * @brief Create an IMFS node.
 * 
 * Create an IMFS filesystem node of an arbitrary type that is NOT
 * the root directory node.
 */
extern IMFS_jnode_t *IMFS_create_node_with_control(
  const rtems_filesystem_location_info_t *parentloc,
  const IMFS_node_control *node_control,
  const char *name,
  size_t namelen,
  mode_t mode,
  const IMFS_types_union *info
);

extern bool IMFS_is_imfs_instance(
  const rtems_filesystem_location_info_t *loc
);

/** @} */

/**
 * @defgroup IMFSGenericNodes IMFS Generic Nodes
 *
 * @ingroup LibIO
 *
 * @brief Generic nodes are an alternative to standard drivers in RTEMS.
 *
 * The handlers of a generic node are called with less overhead compared to the
 * standard driver operations.  The usage of file system node handlers enable
 * more features like support for fsync() and fdatasync().  The generic nodes
 * use the reference counting of the IMFS.  This provides automatic node
 * destruction when the last reference vanishes.
 */
/**@{*/

/**
 * @brief Makes a generic IMFS node.
 *
 * @param[in] path The path to the new generic IMFS node.
 * @param[in] mode The node mode.
 * @param[in] node_control The node control.
 * @param[in] context The node control handler context.
 *
 * @retval 0 Successful operation.
 * @retval -1 An error occurred.  The @c errno indicates the error.
 *
 * @code
 * #include <sys/stat.h>
 * #include <assert.h>
 * #include <fcntl.h>
 *
 * #include <rtems/imfs.h>
 *
 * static const IMFS_node_control some_node_control = {
 *   .imfs_type = IMFS_GENERIC,
 *   .handlers = &some_node_handlers,
 *   .node_initialize = IMFS_node_initialize_generic,
 *   .node_remove = IMFS_node_remove_default,
 *   .node_destroy = some_node_destroy
 * };
 *
 * void example(void *some_node_context)
 * {
 *   int rv;
 *
 *   rv = IMFS_make_generic_node(
 *     "/path/to/some/generic/node",
 *     S_IFCHR | S_IRWXU | S_IRWXG | S_IRWXO,
 *     &some_node_control,
 *     some_node_context
 *   );
 *   assert(rv == 0);
 * }
 * @endcode
 */
extern int IMFS_make_generic_node(
  const char *path,
  mode_t mode,
  const IMFS_node_control *node_control,
  void *context
);

/** @} */

/**
 * @addtogroup IMFS
 */
/**@{*/

/**
 * @brief Mount an IMFS.
 */
extern int IMFS_mount(
  rtems_filesystem_mount_table_entry_t *mt_entry  /* IN */
);

/**
 * @brief Unmount an IMFS.
 */
extern int IMFS_unmount(
  rtems_filesystem_mount_table_entry_t *mt_entry  /* IN */
);

extern IMFS_jnode_t *IMFS_memfile_remove(
 IMFS_jnode_t  *the_jnode         /* IN/OUT */
);

/**
 * @brief Truncate a memory file.
 *
 * This routine processes the ftruncate() system call.
 */
extern int memfile_ftruncate(
  rtems_libio_t *iop,               /* IN  */
  off_t          length             /* IN  */
);

/**
 * @brief Read the next directory of the IMFS.
 * 
 * This routine will read the next directory entry based on the directory
 * offset. The offset should be equal to -n- time the size of an individual
 * dirent structure. If n is not an integer multiple of the sizeof a
 * dirent structure, an integer division will be performed to determine
 * directory entry that will be returned in the buffer. Count should reflect
 * -m- times the sizeof dirent bytes to be placed in the buffer.
 * If there are not -m- dirent elements from the current directory position
 * to the end of the exisiting file, the remaining entries will be placed in
 * the buffer and the returned value will be equal to -m actual- times the
 * size of a directory entry.
 */
extern ssize_t imfs_dir_read(
  rtems_libio_t *iop,              /* IN  */
  void          *buffer,           /* IN  */
  size_t         count             /* IN  */
);

/**
 * @name IMFS Memory File Handlers
 *
 * This section contains the set of handlers used to process operations on
 * IMFS memory file nodes.  The memory files are created in memory using
 * malloc'ed memory.  Thus any data stored in one of these files is lost
 * at system shutdown unless special arrangements to copy the data to
 * some type of non-volailte storage are made by the application.
 */
/**@{*/

/**
 * @brief Open a memory file.
 *
 * This routine processes the open() system call.  Note that there is
 * nothing special to be done at open() time.
 */
extern int memfile_open(
  rtems_libio_t *iop,             /* IN  */
  const char    *pathname,        /* IN  */
  int            oflag,           /* IN  */
  mode_t         mode             /* IN  */
);

/**
 * @brief Read a memory file.
 *
 * This routine processes the read() system call.
 */
extern ssize_t memfile_read(
  rtems_libio_t *iop,             /* IN  */
  void          *buffer,          /* IN  */
  size_t         count            /* IN  */
);

/**
 * @brief Write a memory file.
 *
 * This routine processes the write() system call.
 */
extern ssize_t memfile_write(
  rtems_libio_t *iop,             /* IN  */
  const void    *buffer,          /* IN  */
  size_t         count            /* IN  */
);

/** @} */

/**
 * @name IMFS Device Node Handlers
 *
 * This section contains the set of handlers used to map operations on
 * IMFS device nodes onto calls to the RTEMS Classic API IO Manager.
 */
/**@{*/

extern int device_open(
  rtems_libio_t *iop,            /* IN  */
  const char    *pathname,       /* IN  */
  int            oflag,          /* IN  */
  mode_t         mode            /* IN  */
);

extern int device_close(
  rtems_libio_t *iop             /* IN  */
);

extern ssize_t device_read(
  rtems_libio_t *iop,            /* IN  */
  void          *buffer,         /* IN  */
  size_t         count           /* IN  */
);

extern ssize_t device_write(
  rtems_libio_t *iop,               /* IN  */
  const void    *buffer,            /* IN  */
  size_t         count              /* IN  */
);

extern int device_ioctl(
  rtems_libio_t   *iop,
  ioctl_command_t  command,
  void            *buffer
);

extern int device_ftruncate(
  rtems_libio_t *iop,               /* IN  */
  off_t          length             /* IN  */
);

/** @} */

/**
 * @brief Set IMFS file access and modification times.
 * 
 * 
 * This routine is the implementation of the utime() system
 * call for the IMFS.
 */
extern int IMFS_utime(
  const rtems_filesystem_location_info_t *loc,
  time_t actime,
  time_t modtime
);

/**
 * @brief Change the IMFS file mode.
 */
extern int IMFS_fchmod(
  const rtems_filesystem_location_info_t *loc,
  mode_t mode
);

/**
 * @brief Create a new IMFS symbolic link node.
 * 
 * The following rouine creates a new symbolic link node under parent
 * with the name given in name.  The node is set to point to the node at
 * to_loc.
 */
extern int IMFS_symlink(
  const rtems_filesystem_location_info_t *parentloc,
  const char *name,
  size_t namelen,
  const char *target
);

/**
 * @brief Put IMFS symbolic link into buffer.
 * 
 * The following rouine puts the symbolic links destination name into
 * buff.
 * 
 */
extern ssize_t IMFS_readlink(
  const rtems_filesystem_location_info_t *loc,
  char *buf,
  size_t bufsize
);

/**
 * @brief Rename the IMFS.
 * 
 * The following rouine creates a new link node under parent with the
 * name given in name and removes the old.
 */
extern int IMFS_rename(
  const rtems_filesystem_location_info_t *oldparentloc,
  const rtems_filesystem_location_info_t *oldloc,
  const rtems_filesystem_location_info_t *newparentloc,
  const char *name,
  size_t namelen
);
/**
 * @brief IMFS node removal handler.
 * 
 * This file contains the handler used to remove a node when a file type
 * does not require special actions.
 */
extern int IMFS_rmnod(
  const rtems_filesystem_location_info_t *parentloc,
  const rtems_filesystem_location_info_t *loc
);

/*
 *  Turn on IMFS assertions when RTEMS_DEBUG is defined.
 */
#ifdef RTEMS_DEBUG
  #include <assert.h>

  #define IMFS_assert(_x) assert(_x)
#else
  #define IMFS_assert(_x)
#endif

static inline void IMFS_Set_handlers( rtems_filesystem_location_info_t *loc )
{
  IMFS_jnode_t *node = (IMFS_jnode_t *) loc->node_access;

  loc->handlers = node->control->handlers;
}

static inline void IMFS_add_to_directory(
  IMFS_jnode_t *dir,
  IMFS_jnode_t *node
)
{
  node->Parent = dir;
  rtems_chain_append_unprotected( &dir->info.directory.Entries, &node->Node );
}

static inline void IMFS_remove_from_directory( IMFS_jnode_t *node )
{
  IMFS_assert( node->Parent != NULL );
  node->Parent = NULL;
  rtems_chain_extract_unprotected( &node->Node );
}

static inline IMFS_jnode_types_t IMFS_type( const IMFS_jnode_t *node )
{
  return node->control->imfs_type;
}

static inline bool IMFS_is_directory( const IMFS_jnode_t *node )
{
  return node->control->imfs_type == IMFS_DIRECTORY;
}

static inline IMFS_jnode_t *IMFS_create_node(
  const rtems_filesystem_location_info_t *parentloc,
  IMFS_jnode_types_t type,
  const char *name,
  size_t namelen,
  mode_t mode,
  const IMFS_types_union *info
)
{
  const IMFS_fs_info_t *fs_info =
    (const IMFS_fs_info_t *) parentloc->mt_entry->fs_info;

  return IMFS_create_node_with_control(
    parentloc,
    fs_info->node_controls [type],
    name,
    namelen,
    mode,
    info
  );
}

/** @} */

/**
 * @addtogroup IMFSGenericNodes
 */
/**@{*/

static inline void *IMFS_generic_get_context_by_node(
  const IMFS_jnode_t *node
)
{
  return node->info.generic.context;
}

static inline void *IMFS_generic_get_context_by_location(
  const rtems_filesystem_location_info_t *loc
)
{
  return loc->node_access_2;
}

static inline void *IMFS_generic_get_context_by_iop(
  const rtems_libio_t *iop
)
{
  return IMFS_generic_get_context_by_location( &iop->pathinfo );
}

static inline dev_t IMFS_generic_get_device_identifier_by_node(
  const IMFS_jnode_t *node
)
{
  return rtems_filesystem_make_dev_t(
    IMFS_GENERIC_DEVICE_MAJOR_NUMBER,
    node->st_ino
  );
}

#ifdef __cplusplus
}
#endif
/** @} */
#endif
/* end of include file */
