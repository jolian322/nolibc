// set tty to raw mode and do some basic terminal control
#include "../libs/jlibc.h"

#define raw_iflag ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON) // input modes: disable break processing, carriage return to newline translation, parity checking, stripping of eighth bit, and software flow control
// not BRKINT -> do not generate SIGINT when a break condition is detected on the input
// not ICRNL -> do not translate carriage return to newline on input
// not INPCK -> do not enable input parity checking
// not ISTRIP -> do not strip the eighth bit from input characters
// not IXON -> do not enable software flow control (e.g., Ctrl-S/Ctrl-Q for XON/XOFF)
#define raw_oflag ~(OPOST) // output modes: disable implementation-defined output processing
// not OPOST -> output is transmitted exactly as specified, without any additional processing (e.g., no newline translation)
#define raw_lflag ~(ECHO | ICANON | IEXTEN | ISIG) // local modes: disable echoing, canonical mode, extended input processing, and signal generation
// not ICANON -> return _read immediately with whatever input is available, without waiting for a newline
// not ECHO -> do not echo input characters back to the terminal
// not ISIG -> do not generate signals for special characters (e.g., Ctrl-C will
// not IEXTEN -> disable implementation-defined input processing (e.g., Ctrl-V for literal next)

// ANSI escape codes for terminal control
#define ESC_MOVE_HOME "\x1b[H"   // Jump cursor to top-left (Row 1, Col 1)
#define ESC_HIDE_CUR "\x1b[?25l" // Hide the blinking cursor
#define ESC_SHOW_CUR "\x1b[?25h" // Show the blinking cursor
#define ESC_SAVE_CUR "\x1b[s"    // Remember where cursor is right now
#define ESC_RESTORE_CUR "\x1b[u" // Jump back to saved position
// ERASING (Crucial for Tab-Completion)
#define ESC_CLR_SCREEN "\x1b[2J" // Wipe entire screen
#define ESC_CLR_LINE "\x1b[2K"   // Wipe the whole current line
#define ESC_CLR_TO_END "\x1b[K"  // Wipe from cursor to the right edge of screen
// COLORS & STYLES (For your prompt)
#define ESC_RESET "\x1b[0m"   // Reset to default white/black
#define ESC_BOLD "\x1b[1m"    // Bold text
#define ESC_RED "\x1b[31m"    // Red text
#define ESC_GREEN "\x1b[32m"  // Green text
#define ESC_YELLOW "\x1b[33m" // Yellow text
#define ESC_BLUE "\x1b[34m"   // Blue text
#define ESC_CYAN "\x1b[36m"   // Cyan text
// CURSOR MOVEMENT
#define ESC_CUR_RIGHT_1 "\x1b[C" // Move cursor right 1 space
#define ESC_CUR_LEFT_1 "\x1b[D"  // Move cursor left 1 space
// ALTERNATE SCREEN BUFFER (Useful for full-screen applications like text editors)
#define ESC_ALT_SCREEN_ON "\x1b[?1049h"  // Switch to the blank alternate screen
#define ESC_ALT_SCREEN_OFF "\x1b[?1049l" // Switch back to normal screen

/*
| Key Pressed | What the emulator sends to your shell | Number of Bytes |
| :--- | :--- | :--- |
| **Up Arrow** | `\x1b[A` | 3 bytes |
| **Down Arrow** | `\x1b[B` | 3 bytes |
| **Right Arrow** | `\x1b[C` | 3 bytes |
| **Left Arrow** | `\x1b[D` | 3 bytes |
| **Home** | `\x1b[H` | 3 bytes |
| **End** | `\x1b[F` | 3 bytes |
| **Page Up** | `\x1b[5~` | 4 bytes |
| **F1** | `\x1bOP` | 3 bytes |
| **F2** | `\x1bOQ` | 3 bytes |
| **Ctrl + Up Arrow** | `\x1b[1;5A` | 6 bytes! |
| Keypress | Byte Value | C Escape Code | Historic Name | What it originally did on a teletype |
| :--- | :--- | :--- | :--- | :--- |
| **Ctrl+@ / Ctrl+Space** | `0` | `\0` | NUL | Null / Empty |
| **Ctrl+A** | `1` | `\x01` | SOH | Start of Heading |
| **Ctrl+B** | `2` | `\x02` | STX | Start of Text |
| **Ctrl+C** | `3` | `\x03` | ETX | End of Text (Now: **SIGINT**) |
| **Ctrl+D** | `4` | `\x04` | EOT | End of Transmission (Now: **EOF**) |
| **Ctrl+E** | `5` | `\x05` | ENQ | Enquiry |
| **Ctrl+F** | `6` | `\x06` | ACK | Acknowledge |
| **Ctrl+G** | `7` | `\x07` | BEL | **Ring the physical bell!** |
| **Ctrl+H** | `8` | `\x08` | BS | Backspace |
| **Ctrl+I** | `9` | `\x09` | HT | Horizontal Tab (The **Tab** key) |
| **Ctrl+J** | `10` | `\x0a` | LF | Line Feed (The **Enter** key on Linux) |
| **Ctrl+K** | `11` | `\x0b` | VT | Vertical Tab |
| **Ctrl+L** | `12` | `\x0c` | FF | Form Feed (Clear screen in some contexts) |
| **Ctrl+M** | `13` | `\x0d` | CR | Carriage Return (The **Enter** key on Windows/older Macs) |
| **Ctrl+[** | `27` | `\x1b` | ESC | **Escape!** (This is where `\x1b` comes from!) |
| **Ctrl+\** | `28` | `\x1c` | FS | File Separator (Now: **SIGQUIT**) |
| **Ctrl+]** | `29` | `\x1d` | GS | Group Separator |
| **Ctrl+^** | `30` | `\x1e` | RS | Record Separator |
| **Ctrl+_** | `31` | `\x1f` | US | Unit Separator |
| **Ctrl+?** | `127` | `\x7f` | DEL | **Delete/Backspace** (This one wraps around to 127) |

tab = 9
enter = 13


*/
#define CTRLD 4
// restore the terminal states (colors, cursor visibility, etc.) in case we left it in a weird state
void restore_terminal_visuals()
{
    _write_str(1, ESC_RESET);          // Force colors/styles back to normal
    _write_str(1, ESC_SHOW_CUR);       // Force cursor to be visible
    _write_str(1, ESC_ALT_SCREEN_OFF); // Force switch back to main screen
    _write_str(1, ESC_RESTORE_CUR);    // Force cursor back to where it was before (in case we left it somewhere weird)
}
enum state
{
    RUNNING,
    EXITED
} state = RUNNING;
int main(int argc, char *argv[], char *envp[])
{
    char a[16];
    // turn on raw mode for the terminal
    struct termios t;
    _ioctl(0, TCGETS, (unsigned long)&t); // get
    // save original terminal settings in t so we can restore them later
    struct termios original_t = t;
    t.c_iflag &= raw_iflag; // set input modes
    t.c_oflag &= raw_oflag; // set output modes
    t.c_lflag &= raw_lflag; // set local modes

    _ioctl(0, TCSETS, (unsigned long)&t); // set modified terminal settings to put terminal in raw mode (immediate input, no echo, etc.)

    _write_str(1, ESC_ALT_SCREEN_ON);
    _write_str(1, ESC_CLR_SCREEN);
    _write_str(1, ESC_HIDE_CUR);
    _write_str(1, ESC_MOVE_HOME);

    while (state == RUNNING)
    {
        int amount = _read(0, a, 1);
        if (a[0] >= 32 && a[0] <= 126)
            _write(1, &a, 1); // echo the character back to the terminal
    }
    // wait for input to see the effect of raw mode (e.g., characters will not be echoed and will be available immediately without waiting for a newline)
    _write(1, &a, 1); // echo the character back to the terminal
    // restore terminal to original state before exiting
    _ioctl(0, TCSETS, (unsigned long)&original_t); // restore original terminal settings
    restore_terminal_visuals();                    // restore terminal visuals in case they were left in a weird state
    return 0;
}