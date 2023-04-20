#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid1, pid2;

    if (pipe(fd) == -1)
    {
        std::cerr << "Error: Failed to create pipe.\n";
        return 1;
    }

    pid1 = fork();
    if (pid1 == -1)
    {
        std::cerr << "Error: Failed to create first child process.\n";
        return 1;
    }

    if (pid1 == 0)
    {
        // First child process
        close(fd[0]); // Close unused read end

        int num1 = 10, num2 = 20;
        write(fd[1], &num1, sizeof(num1));
        write(fd[1], &num2, sizeof(num2));
        std::cout << "First child process sent " << num1 << " and " << num2 << ".\n";

        close(fd[1]); // Close write end
        return 0;
    }

    pid2 = fork();
    if (pid2 == -1)
    {
        std::cerr << "Error: Failed to create second child process.\n";
        return 1;
    }

    if (pid2 == 0)
    {
        // Second child process
        close(fd[1]); // Close unused write end

        int num1, num2;
        read(fd[0], &num1, sizeof(num1));
        read(fd[0], &num2, sizeof(num2));
        std::cout << "Second child process received " << num1 << " and " << num2 << ".\n";
        std::cout << "Sum of " << num1 << " and " << num2 << " is " << (num1 + num2) << ".\n";

        close(fd[0]); // Close read end
        return 0;
    }

    // Parent process
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);



    return 0;
}
