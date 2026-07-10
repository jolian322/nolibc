#include "../libs/jlibc.h"
int _jfcp(char *src, char *dest, int options)
{
    /*
        copy from file src to file dest
        creates dest if it does not exist, otherwise overwrites it ( if options = 0)
    */
    char buffer[BUFFER_SIZE_OPTIMAL];

    int src_fd = _open(src, O_RDONLY, 0);
    if (src_fd < 0)
    {
        _write_str(2, "Error opening source file: ");
        _write_str(2, src);
        _write_str(2, "\n");
        return src_fd;
    }

    int flags;
    umode_t mode;
    // check if no file dest file exists and create it or if it exists overwrite it based on options
    if (options & 1)
    {                                                 // -f flag set?
        flags = O_WRONLY | O_CREAT | O_TRUNC;         // overwrite if exists
        mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // rw-r--r--
    }
    else
    {
        flags = O_WRONLY | O_CREAT | O_EXCL;          // fail if file exists
        mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // rw-r--r--
    }
    if (options & 2)
    { // -v flag set?
        _write_str(1, "Copying ");
        _write_str(1, src);
        _write_str(1, " to ");
        _write_str(1, dest);
        _write_str(1, "\n");
    }

    int dest_fd = _open(dest, flags, mode);
    if (dest_fd < 0)
    {
        _close(src_fd);
        _write_str(2, "dest file already exists (use -f flag to overwrite)");
        _write_str(2, "\n");
        return dest_fd;
    }

    while (1)
    {
        ssize_t bytes_read = _read(src_fd, buffer, BUFFER_SIZE_OPTIMAL);
        if (bytes_read > 0)
        {
            ssize_t bytes_written = _write(dest_fd, buffer, bytes_read);
            if (bytes_written < 0)
            {
                _close(src_fd);
                _close(dest_fd);
                return bytes_written;
            }
        }
        else if (bytes_read == 0)
        {
            break; // EOF
        }
        else
        {
            _close(src_fd);
            _close(dest_fd);
            return bytes_read; // error
        }
    }
    _close(src_fd);
    _close(dest_fd);
    return 0;
}
int _jfcp_parse(int argc, char **argv)
{
    char *src = NULL;
    char *dest = NULL;
    int options = 0; // -f flag  // -v flag
    // parse arguments
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            // flag
            if (argv[i][1] == 'f')
            {
                options |= 1; // -f flag
            }
            else if (argv[i][1] == 'v')
            {
                options |= 2; // -v flag
            }
            else
            {
                _write_str(2, "Usage: jfcp [-f] [-v] <source> <destination>\n");
                _write_str(2, "\n");
                return -1;
            }
        }
        else
        {
            // file path
            if (src == NULL)
            {
                src = argv[i];
            }
            else if (dest == NULL)
            {
                dest = argv[i];
            }
            else
            {
                _write_str(2, "Usage: jfcp [-f] [-v] <source> <destination>\n");
                _write_str(2, "\n");
                return -1;
            }
        }
    }
    if (src == NULL || dest == NULL)
    {
        _write_str(2, "Usage: jfcp [-f] [-v] <source> <destination>\n");
        _write_str(2, "\n");
        return -1;
    }
    return _jfcp(src, dest, options);
};
int _jflc(char *path)
{
    /* returns line count of a file */
    char buffer[BUFFER_SIZE_OPTIMAL];
    int fd = _open(path, O_RDONLY, 0);
    if (fd < 0)
    {
        _write_str(2, "Error opening file: ");
        _write_str(2, path);
        _write_str(2, "\n");
        return fd;
    }
    int line_count = 0;
    ssize_t bytes;
    while ((bytes = _read(fd, buffer, BUFFER_SIZE_OPTIMAL)) > 0)
    {
        // count lines in buffer
        for (ssize_t i = 0; i < bytes; i++)
        {
            if (buffer[i] == '\n')
            {
                line_count++;
            }
        }
    }

    _close(fd);
    return line_count;
}
int _jflc_parse(int argc, char **argv)
{
    int line_count = 0;
    char *path = NULL;
    char *paths[128] = {0}; // max 128 file paths
    int path_count = 0;
    // parse arguments
    if (argc < 2)
    {
        _write_str(2, "Usage: jflc <files>\n");
        _write_str(2, "\n");
        return -1;
    }
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            // flag
            _write_str(2, "Usage: jflc <files>\n");
            _write_str(2, "\n");
            _write_str(2, "No flags supported\n");
            return -1;
        }
        // file path
        if (path == NULL)
        {
            path = argv[i];
            paths[path_count] = argv[i];
            path_count += 1;
            path = NULL;
        }
    }
    for (int i = 0; i < path_count; i++)
    {
        int count = _jflc(paths[i]);
        if (count < 0)
        {
            _write_str(1, "error counting lines in file: ");
            _write_str(1, paths[i]);
            _write_str(1, "\n");
            return -1;
        }

        line_count += count;
    }

    _write_str(1, "line count: ");
    _write_int(1, line_count);
    _write_str(1, "\n");
    return 0;
};
void _jfhd(char *path, int options)
{
    // print hexdump of file to stdout

    int fd = _open(path, O_RDONLY, 0);
    if (fd < 0)
    {
        _write_str(2, "Error opening file: ");
        _write_str(2, path);
        _write_str(2, "\n");
        return;
    }
    ssize_t bytes;
    int bytesperline;
    if (options & 1)
    { // -s 16 bytes per line
        bytesperline = 16;
    }
    else
    {
        bytesperline = 16;
    }
    char buffer[bytesperline];
    int offset = 0;
    int amount;
    while ((bytes = _read(fd, buffer, bytesperline)) > 0)
    {
        // print offset

        _write_byte_hex(1, offset);
        _write_char(1, ' ');
        _write_char(1, ' ');
        _write_char(1, ' ');
        // print hex of buffer

        for (ssize_t i = 0; i < bytes; i++)
        {
            _write_byte_hex(1, buffer[i]);
            _write_char(1, ' ');
        }
        _write_char(1, ' ');
        _write_char(1, ' ');
        if (bytes < bytesperline)
        {
            amount = bytesperline - bytes;
            for (ssize_t i = 0; i < amount; i++)
            {
                _write_char(1, ' ');
                _write_char(1, ' ');
                _write_char(1, ' ');
            }
        }
        _write_char(1, '|');
        // print ASCII of buffer
        for (ssize_t i = 0; i < bytes; i++)
        {
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                _write_char(1, buffer[i]);
            }
            else
            {
                _write_char(1, '.');
            }
        }
        _write_char(1, '|');
        _write_char(1, '\n');
        offset += bytes;
    }
}
void _jfhd_parse(int argc, char **argv)
{
    char *path = NULL;
    int options = 0; // -s flag
    // parse arguments
    if (argc < 2)
    {
        _write_str(2, "Usage: jfhd [-s] <file>\n");
        _write_str(2, "\n");
        return;
    }
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            // flag
            if (argv[i][1] == 's')
            {
                options |= 1; // -s flag
            }
            else
            {
                _write_str(2, "Usage: jfhd [-s] <file>\n");
                _write_str(2, "\n");
                return;
            }
        }
        else
        {
            // file path
            if (path == NULL)
            {
                path = argv[i];
            }
            else
            {
                _write_str(2, "Usage: jfhd [-s] <file>\n");
                _write_str(2, "\n");
                return;
            }
        }
    }
    if (path == NULL)
    {
        _write_str(2, "Usage: jfhd [-s] <file>\n");
        _write_str(2, "\n");
        return;
    }
    _jfhd(path, options);
};

int main(int argc, char **argv, char **envp)
{
    _jfhd_parse(argc, argv);
    return 0;
}
