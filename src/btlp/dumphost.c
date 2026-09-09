/* Persona 1 (JP) - writing a buffer out to the development machine.
 *   BTLP @ 0x80066864 BtlWriteHostFile
 *
 * The Psy-Q host-file calls go through BIOS breakpoints the debugging station
 * answers, so this only does anything on development hardware: it creates the
 * named file on the PC, pushes the buffer at it and closes it again. Nothing
 * calls it, and no error is checked - a failed create leaves a negative handle
 * that the write and the close both take without complaint.
 *
 * The length is a short, which is why the call narrows it.
 */
#include <decomp/types.h>
#include <libsn.h>

void BtlWriteHostFile(char *name, u_short len, char *buf)
{
    int fd;

    fd = PCcreat(name, 0);
    PCwrite(fd, buf, len);
    PCclose(fd);
}
