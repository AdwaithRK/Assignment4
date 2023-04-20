#include <iostream>
#include <string>
#include <vector>
    #include <fstream> // add this line
    #include <sstream> // add this line
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main()
{
    string msg = "file1 adwaith is testing contents\n hai this is adwaith\n";

    istringstream iss(msg);

    string result, result1;

    getline(iss, result, '\n');
    getline(iss, result1, '\n');

    cout
        << "message is : " << result;
    cout << "\nmessage 2 is : " << result1;
    return 0;
}
