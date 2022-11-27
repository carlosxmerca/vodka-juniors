#include <iostream>
#include <stdio.h>

using namespace std;

int main(void)
{
    // Temp timestamp, testing
    int seconds = stoi(string(1, __TIME__[6]) + string(1, __TIME__[7]));
    // Water times (seconds)
    int water_times[2] = {15, 30};
    // Week water times
    int week[7] = {2, 2, 2, 2, 2, 2, 2};

    // Imaginary day, to change with request day
    int i = 0;

    while (true)
    {
        // Working idea, with i being the requested day
        if (i == 0)
            for (int j = 0; j < 7; i++)
                week[j] = 2;

        while (week[i] > 0)
        {
            // Working algorithm
            if ((seconds == water_times[0] || seconds == water_times[1]))
                week[i]--;
        }

        i++;
        if (i > 7)
            i = 0;
    }

    return 0;
}