#include "ll/sys/cdefs.h"

__BEGIN_DECLS
extern int (*pcibios_read_config_byte)
		(BYTE bus, BYTE dev, BYTE where, BYTE *val);
extern int (*pcibios_read_config_word)
		(BYTE bus, BYTE dev, BYTE where, WORD *val);
extern int (*pcibios_read_config_dword)
		(BYTE bus, BYTE dev, BYTE where, DWORD *val);
extern int (*pcibios_write_config_byte)
		(BYTE bus, BYTE dev, BYTE where, BYTE val);
extern int (*pcibios_write_config_word)
		(BYTE bus, BYTE dev, BYTE where, WORD val);
extern int (*pcibios_write_config_dword)
		(BYTE bus, BYTE dev, BYTE where, DWORD val);

int pcibios_find_class(DWORD class_code, WORD index, BYTE *bus, BYTE *dev);
int pcibios_find_device(WORD vendor, WORD device, WORD index, BYTE *bus, BYTE *dev);
int pcibios_init(void);
int pcibios_present(void);
__END_DECLS
