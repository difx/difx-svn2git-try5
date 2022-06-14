
#include "Mark6.h"
#include "Mark6Meta.h"
#include <iostream>
#include <unistd.h>


int main() {
    Mark6 *mark6 = new Mark6();

    while (true)
    {
        mark6->pollDevices();
        usleep(5000000);
    }
}

