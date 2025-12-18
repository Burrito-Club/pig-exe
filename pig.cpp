#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <string>
using namespace std;

struct Pig {
    int hunger = 50;
    int thirst = 50;
    int happiness = 50;
    int health = 100;
    int age = 0;
    bool sleeping = false;
    bool alive = true;
};

string playerInput = "";
mutex inputMutex;

// Input thread
void getInput() {
    string input;
    while (true) {
        getline(cin, input);
        lock_guard<mutex> lock(inputMutex);
        playerInput = input;
    }
}

// ASCII pig moods
void printPig(Pig &p) {
    if (p.sleeping) {
        cout <<
R"(   _
       <`--'\>______
       /z z  `'     \
      (`')  ,        @
       `-._,        /
          )-)_/--( >  jv
         ''''  ''''
)";
    } else if (p.health < 30) {
        cout <<
R"(   _
       <`--'\>______
       /x X  `'     \
      (`')  ,        @
       `-._,        /
          )-)_/--( >  jv
         ''''  ''''
)";
    } else if (p.hunger > 70 || p.thirst > 70) {
        cout <<
R"(   _
       <`--'\>______
       /O O  `'     \
      (`')  ,        @
       `-._,        /
          )-)_/--( >  jv
         ''''  ''''
)";
    } else {
        cout <<
R"(   _
       <`--'\>______
       /^ ^  `'     \
      (`')  ,        @
       `-._,        /
          )-)_/--( >  jv
         ''''  ''''
)";
    }
}

void printStats(Pig &p) {
    cout << "Hunger: " << p.hunger << "\n";
    cout << "Thirst: " << p.thirst << "\n";
    cout << "Happiness: " << p.happiness << "\n";
    cout << "Health: " << p.health << "\n";
    cout << "Age: " << p.age << " game minutes\n";
}

void handleAction(Pig &p, const string &input, int minutes) {
    if (p.sleeping) {
        if (input != "stats") {
            int remainingGameMinutes = 15 - (minutes % 30); // 15 game minutes sleep
            int realMinutes = remainingGameMinutes * 15 / 60; // 15s per game minute
            if(realMinutes == 0) realMinutes = 1;
            cout << "Pig is sleeping. Will be awake in " << realMinutes << " real minutes.\n";
            return;
        } else {
            printStats(p);
            return;
        }
    }

    if (input == "1") { p.hunger -= 30; cout << "Fed the pig!\n"; }
    else if (input == "2") { p.thirst -= 30; cout << "Gave water!\n"; }
    else if (input == "3") { p.happiness += 20; cout << "Played with pig!\n"; }
    else if (input == "4") { cout << "Did nothing.\n"; }
    else if (input == "stats") { printStats(p); }
    else cout << "Invalid action! Try again.\n";

    if (p.hunger < 0) p.hunger = 0;
    if (p.thirst < 0) p.thirst = 0;
    if (p.happiness > 100) p.happiness = 100;
}

int main() {
    Pig p;
    thread inputThread(getInput);
    inputThread.detach();

    int minutes = 0;
    int minutesSincePrompt = 0;
    const int promptInterval = 2; // every 2 game minutes
    bool promptActive = false;

    cout << "Welcome to your ASCII pig!\n";

    while (p.alive) {
        this_thread::sleep_for(chrono::seconds(15)); // 15s per game minute
        minutes++;
        minutesSincePrompt++;
        p.age++;

        // Sleep schedule: 15 game minutes asleep every 30
        if (minutes % 30 < 15) p.sleeping = false;
        else p.sleeping = true;

        // Update stats
        if (!p.sleeping) {
            p.hunger += 2;
            p.thirst += 2;
            p.happiness -= 1;
        } else {
            p.happiness += 1; // pig gains happiness while resting
        }

        if (p.hunger > 100) p.hunger = 100;
        if (p.thirst > 100) p.thirst = 100;
        if (p.happiness < 0) p.happiness = 0;
        if (p.happiness > 100) p.happiness = 100;

        // Health decreases if starving or dehydrated
        if (p.hunger > 80 || p.thirst > 80) {
            p.health -= 3;
            if (p.health < 0) p.health = 0;
        }

        // Death check
        if (p.hunger >= 100 || p.thirst >= 100 || p.health <= 0) {
            p.alive = false;
            break;
        }

        system("cls"); // clear console
        printPig(p);

        // Action prompt every 2 game minutes if awake
        if (!p.sleeping && minutesSincePrompt >= promptInterval && !promptActive) {
            minutesSincePrompt = 0;
            promptActive = true;
            cout << "\nChoose action: 1-Feed 2-Water 3-Play 4-Do nothing OR type 'stats'\n";
        }

        // Handle input
        if (promptActive) {
            string inputCopy;
            {
                lock_guard<mutex> lock(inputMutex);
                if (!playerInput.empty()) {
                    inputCopy = playerInput;
                    playerInput = ""; // clear immediately
                }
            }

            if (!inputCopy.empty()) {
                handleAction(p, inputCopy, minutes);
                promptActive = false; // one action per prompt
            }
        }
    }

    cout << "\nYour pig has died at age " << p.age << " game minutes.\n";
    return 0;
}
