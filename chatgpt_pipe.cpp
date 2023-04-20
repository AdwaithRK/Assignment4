#include <iostream>
#include <string>
#include <vector>
#include <fstream> // add this line
#include <sstream> // add this line
#include <unistd.h>
#include <sys/wait.h>

using namespace std;
// using namespace std;

int main()
{
    string d1 = "dir1";
    string d2 = "dir2";

    // Create the directories and files
    system(("mkdir " + d1).c_str());
    system(("echo 'file1 contents' > " + d1 + "/file1").c_str());
    system(("echo 'file2 contents' > " + d1 + "/file2").c_str());

    system(("mkdir " + d2).c_str());
    system(("echo 'file3 contents' > " + d2 + "/file3").c_str());
    system(("echo 'file4 contents' > " + d2 + "/file4").c_str());

    // Create pipes for communication between the child processes
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the first child process
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    {
        // Child 1 reads the files in directory d1 and sends the list to child 2
        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2

        vector<string> files1;
        files1.push_back("file1");
        files1.push_back("file2");

        string msg = "";
        for (const auto &file : files1)
        {
            string filename = d1 + "/" + file;
            //  cout << "file name now is: " << filename << "\n";
            ifstream f(filename.c_str());
            if (!f.good())
            {
                msg = "Error: failed to read " + filename;
                write(pipe1[1], msg.c_str(), msg.size() + 1);
                exit(EXIT_FAILURE);
            }

            string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            msg += filename + " " + contents + "\n";
        }
        cout << msg << " haha \n";
        write(pipe1[1], msg.c_str(), msg.size() + 1);

        exit(EXIT_SUCCESS);
    }
    else
    {
        // Fork the second child process
        pid_t pid2 = fork();
        if (pid2 < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        {
            // cout << "here in child\n";
            // Child 2 receives the list from child 1 and creates the corresponding files in directory d2
            close(pipe1[1]); // Close write end of pipe1
            close(pipe2[0]); // Close read end of pipe2

            string msg = "";
            char buf[BUFSIZ];
            //  cout << "here before read\n";
            while (read(pipe1[0], buf, BUFSIZ) > 0)
            {
                msg += buf;
            }
            cout << "message = "
                 << "\n";
            cout << msg << "\n";

            vector<string> files1_n;
            vector<string> contents2;
            istringstream iss(msg);
            string file1Path, file1Name, file1Contents;
            string file2Path, file2Name, file2Contents;

            iss >> file1Path >> file1Name >> file1Contents;
            iss >> file2Path >> file2Name >> file2Contents;

            files1_n.push_back(file1Name);
            files1_n.push_back(file2Name);

            contents2.push_back(file1Contents);
            contents2.push_back(file1Contents);

            cout << "file name1 : " << file1Name << "\n";
            cout << "file name2 : " << file2Name << "\n";

            for (int i = 0; i < 2; i++)
            {
                string filename = d2 + "/" + files1_n[i];
                ofstream f(filename.c_str());
                if (!f.good())
                {
                    msg = "Error: failed to create " + filename;
                    write(pipe2[1], msg.c_str(), msg.size() + 1);
                    exit(EXIT_FAILURE);
                }
                f << contents2[i];
                f.close();
            }

            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process waits for both child processes to finish
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

            // Check that the directories are identical
            // system(("diff -r " + d1 + " " + d2).c_str());

            // Remove the directories and files
            // system(("rm -rf " + d1).c_str());
            // system(("rm -rf " + d2).c_str());

            exit(EXIT_SUCCESS);
        }
    }
}