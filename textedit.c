#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>
#include <string.h>

#define SIZE_BUF 4096
#define SIZE 16
#define F1 265
#define F2 266
#define F3 267
#define ENTER 10

#define DEBUG 0

void sig_winch(int signo)
{
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, &size);
    resizeterm(size.ws_row, size.ws_col);
}

void clear_buf(char *buf)
{
    int i = 0;
    for(i = 0; i < SIZE; i++)
    {
        buf[i] = 0;
    }
}

void clear_in()
{
    while(getchar() != '\n');
}

int main()
{
    WINDOW *wnd = NULL;
    WINDOW *command_wnd = NULL;
    WINDOW *working_wnd = NULL;
    int rows = 0, cols = 0, fd = 0, cycle = 1, i = 0, j = 0,
        write_size = 0, read_size = 0, x = 1, y = 1, num_buf = 0, 
        input_buf = 0, num_buf_tmp = 0;
    char buf[SIZE] = {0};
    char *work_buf = calloc(SIZE_BUF, sizeof(char));
    
    initscr();
    signal(SIGWINCH, sig_winch);
    cbreak();
    curs_set(0);
    refresh();
    getmaxyx(stdscr, rows, cols);
    wnd = newwin(rows, cols, 0, 0);
    box(wnd,'|', '-');
    command_wnd = newwin(3, cols, rows-3, 0);
    box(command_wnd,'|', '-');
    move(rows - 2, 2);
    printw("F1 - OPEN, F2 - SAVE, F3 - EXIT");
    working_wnd = newwin(rows - 7, cols - 2, 1, 1);
    wrefresh(wnd);
    wrefresh(command_wnd);
    wrefresh(working_wnd);
    refresh();
    keypad(stdscr, TRUE);
   // keypad(wnd, TRUE);
    //keypad(working_wnd, TRUE);
    noecho();
    
    while(cycle)
    {
        cbreak();
        input_buf = getch();
        switch(input_buf)
        {
            case F1:
            {
                if(fd != 0)
                    close(fd);
                memset(&work_buf[0], 0, sizeof(work_buf));
                nocbreak();
                clear_buf(buf);
                curs_set(1);
                echo();
                move(rows - 2, 2);
                printw("Name :                               ");
                refresh();
                mvgetnstr(rows - 2, 9, buf, 15); 
                fd = open(buf, O_RDWR|O_CREAT, 0666);
                if(fd == -1)
                {
                    delwin(wnd);
                    delwin(command_wnd);
                    delwin(working_wnd);
                    endwin();
                    perror("Create error ");
                    exit(1);
                }
                read_size = read(fd, work_buf, SIZE_BUF - 1);
                if(read_size == -1)
                {
                    delwin(wnd);
                    delwin(command_wnd);
                    delwin(working_wnd);
                    endwin();
                    perror("Read error ");
                    exit(1);
                }
                if(read_size == 0)
                    read_size = 1;
                cbreak();
                move(rows - 2, 2);
                printw("F1 - OPEN, F2 - SAVE, F3 - EXIT");
                noecho();
                break;
            }
            
            case F2:
            {
                if(fd != 0)
                {
                    work_buf[read_size + 1] = '\0';
                    if(lseek(fd, 0, SEEK_SET) == -1)
                    {
                        perror("Seek error ");
                        delwin(wnd);
                        delwin(command_wnd);
                        delwin(working_wnd);
                        endwin();
                        exit(1);
                    }
                    write_size = write(fd, work_buf, read_size * sizeof(char));
                    
                    if(write_size == -1)
                    {
                        delwin(wnd);
                        delwin(command_wnd);
                        delwin(working_wnd);
                        endwin();
                        perror("Write error ");
                        exit(1);
                    }
                }
                break;
            }
            
            case F3:
            {
                cycle = 0;
                if(fd != 0)
                    close(fd);
                break;
            }
            
            case KEY_ENTER:
            {
                if(work_buf[num_buf] == '\n')
                    break;
                read_size += 1;
                work_buf[num_buf] = '\n';
                break;
            }
            
            case KEY_RIGHT:
            {
                if(read_size != 0)
                    if((y < (cols - 2)) && num_buf < read_size && work_buf[num_buf] != '\n')
                    {
                        y += 1;
                        num_buf += 1;
                    }else{
                        if(work_buf[num_buf] == '\n')
                        {
                            y = 1;
                            x += 1;
                            num_buf += 1;
                        }
                    }
                break;
            }
            
            case KEY_UP:
            {
                if(x > 1 && read_size != 0)
                {
                    x -= 1;
                    for(i = num_buf; work_buf[i] != '\n'; i--);
                    num_buf = i - 1;
                    for(j = 0; work_buf[num_buf - j] != '\n'; j++)
                        if(num_buf - j + 1 == 0)
                            break;
                    if(y > j + 1)
                    {
                        y = j + 1;
                        num_buf += 1;
                    }else
                        num_buf = num_buf - j + y;
                }
                break;
            }
            
            case KEY_DOWN:
            {
                if(num_buf < read_size && x < (rows - 7) && read_size != 0)
                {
                    for(i = num_buf; work_buf[i] != '\n'; i++);
                    num_buf_tmp = i + 1;
                    for(j = 0; work_buf[num_buf_tmp + j] != '\n'; j++)
                        if(num_buf_tmp + j >= read_size)
                        {
                            break;
                        }
                    if(num_buf_tmp > read_size)
                        break;
                    else
                    {
                        x += 1;
                        if(y > j + 1)
                        {
                            y = j + 1;
                            num_buf = num_buf_tmp + j;
                        }else
                            num_buf = num_buf_tmp + (y - 1);
                    }
                }
                break;
            }
            
            case KEY_LEFT:
            {
                if(read_size != 0)
                    if(y > 1)
                    {
                        y -= 1;
                        num_buf -= 1;
                    }else
                    {
                        if(num_buf > 0)
                        {
                            num_buf -= 1;
                            x -= 1;
                            for(j = 0; work_buf[num_buf - 1 - j] != '\n'; j++)
                                if((num_buf - j) == 0)
                                    break;
                            y = j + 1;
                                
                        }
                    }
                break;
            }
            
            default:
            {
                if(read_size != 0)
                {
                    if(num_buf == read_size)
                        read_size ++;
                    work_buf[num_buf] = (char)input_buf;
                    wclear(working_wnd);
                }
                break;
            }
        }
#if DEBUG
        move(rows - 2, cols - 14);
        printw(" %d %d %d %d %c", num_buf, read_size, i, j, "%s", work_buf[num_buf]);
#endif

        move(x, y);
        mvwprintw(working_wnd, 0, 0, "%s", work_buf);
        wrefresh(working_wnd);
        refresh();
    }
    
    if(fd != 0)
        close(fd);
    free(work_buf);
    if(command_wnd != NULL)
        delwin(command_wnd);
    if(working_wnd != NULL)
        delwin(working_wnd);   
    if(wnd != NULL) 
        delwin(wnd);

    endwin();
    
    return 0;
}
