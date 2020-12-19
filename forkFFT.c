#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
// convert

float convertToFloat(char *value)
{
    char *pend;

    float f = strtof(value, &pend);

    // printf("converted to: %d\n", errno);

    if (f == 0 && value == pend)
    {
        fprintf(stderr, "convert error");
        exit(EXIT_FAILURE);
    }

    // printf("converted to: %f\n", f);

    return f;
}

void addTo(char *p, char *value)
{
    strcat(p, value);

    // printf("concat : %s new string %s\n", value, p);
    // fflush(stdout);
}

void writeToPipe(int fd, char *value)
{
    FILE *f = fdopen(fd, "w");

    if (fputs(value, f) == -1)
    {
        fprintf(stderr, "writeToPipe failed\n");
        exit(EXIT_FAILURE);
    }

    // fflush(f);
    fclose(f);
}

static void wait_handler(int err)
{
    int status,
        pid, error = err;
    while ((pid = wait(&status)) != -1)
    {
        if (WEXITSTATUS(status) != EXIT_SUCCESS)
            error = 1;
    }

    if (error)
        exit(EXIT_FAILURE);

    if (errno != ECHILD)
    {
        fprintf(stderr, "cannot wait");
        wait_handler(1);
    }
}

void clientSetup(int *in, int *out, int *other_in, int *other_out)
{
    close(in[1]);
    close(out[0]);

    if (dup2(in[0], STDIN_FILENO) == -1 || dup2(out[1], STDOUT_FILENO) == -1)
    {
        fprintf(stderr, "could not redirect pipe");
    }

    close(in[0]);
    close(out[1]);

    close(other_in[0]);
    close(other_out[1]);

    close(other_in[0]);
    close(other_out[1]);

    execlp("./forkFFT", "./forkFFT", NULL);

    // dup2(fd,STDOUT_FILENO); // new descriptor
    // close(fd);

    fprintf(stderr, "Cannot exec!\n");
    exit(EXIT_FAILURE);
}

void readPipes(int *first, int *sec)
{
    FILE *first_out = fdopen(first[0], "r");

    char *first_line = NULL;
    size_t first_lencap = 0;
    ssize_t first_len = 0;

    FILE *sec_out = fdopen(sec[0], "r");

    char *sec_line = NULL;
    size_t sec_lencap = 0;
    ssize_t sec_len = 0;

    if ((first_len = getline(&first_line, &first_lencap, first_out)) != -1)
    {
        fprintf(stdout, "%s", first_line);
    }

    if ((sec_len = getline(&sec_line, &sec_lencap, sec_out)) != -1)
    {
        fprintf(stdout, "%s", sec_line);
    }


    fclose(first_out);
    fclose(sec_out);
}

void forkAndWrite(char *even, char *odd)
{
    int pipe_even_in[2];
    int pipe_even_out[2];

    pipe(pipe_even_in);
    pipe(pipe_even_out);

    int pipe_odd_in[2];
    int pipe_odd_out[2];

    pipe(pipe_odd_in);
    pipe(pipe_odd_out);

    pid_t even_pid, odd_pid;

    even_pid = fork();

    switch (even_pid)
    {
    case -1:
        fprintf(stderr, "Cannot fork!\n");
        exit(EXIT_FAILURE);
    case 0:
        fprintf(stderr, "even!\n");
        clientSetup(pipe_even_in, pipe_even_out, pipe_odd_in, pipe_odd_out);
        break;
    default:
        // parent tasks ...

        close(pipe_even_in[0]);
        close(pipe_even_out[1]);

        int pfd_even = pipe_even_in[1];
        writeToPipe(pfd_even, even);

        odd_pid = fork();

        switch (odd_pid)
        {
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            fprintf(stderr, "odd!\n");

            clientSetup(pipe_odd_in, pipe_odd_out, pipe_even_in, pipe_even_out);
            break;

        default:
            // parent tasks ...

            close(pipe_odd_in[0]);
            close(pipe_odd_out[1]);

            int pfd_odd = pipe_odd_in[1];
            writeToPipe(pfd_odd, odd);

            readPipes(pipe_even_out, pipe_odd_out);

            // wait_handler(even_pid, odd_pid);

            break;
        }

        break;
    }
}

int main(int argc, char *const argv[])
{

    // stdin = fopen("constant.txt", "r");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelength;

    // char **lines_p = malloc(0);
    int count = 0;
    int index = 0;

    char *even = malloc(0);
    char *odd = malloc(0);

    while ((linelength = getline(&line, &linecap, stdin)) > 0)
    {
        printf("current line is: %s", line);

        if (count % 2 == 0)
        {
            addTo(even, line);
        }
        else
        {
            addTo(odd, line);
            index++;
        }
        count++;
        // printf("inserted count now %d\n", count);
        // fflush(stdout);
    }

    printf("even is: \n%s", even);
    printf("odd is: \n%s", odd);

    if (count == 0)
    {
        fprintf(stderr, "only 0");
        exit(EXIT_FAILURE);
    }

    if (count == 1)
    {
        float f = convertToFloat(even);
        fprintf(stdout, "out: %f\n", f);
        exit(EXIT_SUCCESS);
    }

    if (count % 2 != 0)
    {
        fprintf(stderr, "not even array");
        exit(EXIT_FAILURE);
    }

    forkAndWrite(even, odd);

    pid_t wpid;
    int status = 0;

    while ((wpid = wait(&status)) > 0)
    {
        printf("Exit status of %d was %d\n", (int)wpid, status);
    }

    fprintf(stderr, "- end of programm code \n");
    /* code */
    return 0;
}
