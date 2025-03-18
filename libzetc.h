void start_up (void);
char *translate (const char *filename);

#define __S_ISTYPE(mode, mask)  (((mode) & __S_IFMT) == (mask))
#define __S_IFMT        0170000 /* These bits determine file type.  */
#define __S_IFDIR       0040000 /* Directory.  */
#define S_ISDIR(mode)    __S_ISTYPE((mode), __S_IFDIR)