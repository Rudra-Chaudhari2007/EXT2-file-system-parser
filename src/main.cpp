#include<iostream>
#include<cstring>
#include<fstream>
using namespace std;

int main()
{
    ifstream file ("/home/rudra/projects/disk-backpup.img" , ios :: binary);

    if(!file)
    {
        cout << "Couldn't open file!!" << endl;
        return 1;
    }

    



    return 0;
}