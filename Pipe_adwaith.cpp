#include <iostream>
#include <string>
#include <vector>
#include <fstream> // add this line
#include <sstream> // add this line
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

// part for child 1
// --------------------------------------------------------------------

void sendFileFromDir1ToDir2(int pipe1[], int pipe2[], string d1)
{

    vector<string> files1;
    files1.push_back("file1");
    files1.push_back("file2");

    string msg = "";
    for (const auto &file : files1)
    {
        string filename = d1 + "/" + file;
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
    write(pipe1[1], msg.c_str(), msg.size() + 1);
}

void readFileFromDir2ToDir1(int pipe1[], int pipe2[], string d1)
{
    string msg = "";
    char buf1[BUFSIZ];
    while (read(pipe2[0], buf1, BUFSIZ) > 0)
    {
        msg += buf1;
    }

    cout << "message recieved is : " << msg << "\n";

    // vector<string> files2;
    istringstream iss(msg);
    string filePath, file, contents;
    while (iss >> filePath >> file >> contents)
    {
        // files2.push_back(file);
        string filename = d1 + "/" + file;
        ofstream f(filename.c_str());
        if (!f.good())
        {
            msg = "Error: failed to create " + filename;
            write(pipe1[1], msg.c_str(), msg.size() + 1);
            exit(EXIT_FAILURE);
        }
        f << contents;
        f.close();
    }
}

//------------------------------------------------------------------------------

// part for child 2
void readFileFromDir1ToDir2(int pipe1[], int pipe2[], string d2)
{
    string msg = "";
    char buf[BUFSIZ];

    cout << "in child 2\n";

    while (read(pipe1[0], buf, BUFSIZ) > 0)
    {
        msg += buf;
    }

    cout << "message recieved in child2 : " << msg << "\n";

    // vector<string> files2;
    istringstream iss(msg);
    string filePath, file, contents;
    while (iss >> filePath >> file >> contents)
    {
        // files2.push_back(file);
        string filename = d2 + "/" + file;
        ofstream f(filename.c_str());
        if (!f.good())
        {
            msg = "Error: failed to create " + filename;
            write(pipe2[1], msg.c_str(), msg.size() + 1);
            exit(EXIT_FAILURE);
        }
        f << contents;
        f.close();
    }
}

void sendFileFromDir2ToDir1(int pipe1[], int pipe2[], string d2)
{

    vector<string> files2;
    files2.push_back("file3");
    files2.push_back("file4");

    string msg = "";

    for (const auto &file : files2)
    {
        string filename = d2 + "/" + file;
        // cout << "filename:" << filename << "\n";
        ifstream f(filename.c_str());
        if (!f.good())
        {
            msg = "Error: failed to read " + filename;
            write(pipe2[1], msg.c_str(), msg.size() + 1);
            exit(EXIT_FAILURE);
        }
        string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        msg += filename + " " + contents + "\n";
    }
    write(pipe2[1], msg.c_str(), msg.size() + 1);
}

// ----------------------------------------------------------------------------------------------------

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

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid1 == 0)
    {
        close(pipe1[0]); // Close read end of pipe1
        close(pipe2[1]); // Close write end of pipe2

        sendFileFromDir1ToDir2(pipe1, pipe2, d1);
        readFileFromDir2ToDir1(pipe1, pipe2, d1);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();

    if (pid2 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid2 == 0)
    {
        close(pipe1[1]); // Close write end of pipe1
        close(pipe2[0]); // Close read end of pipe2

        readFileFromDir1ToDir2(pipe1, pipe2, d2);
        sendFileFromDir2ToDir1(pipe1, pipe2, d2);
        exit(EXIT_SUCCESS);
    }

    return 0;
}