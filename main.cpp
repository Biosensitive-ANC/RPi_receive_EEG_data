#include "EEG.h"
#include <thread>
#include <chrono>

int main() {
    EEGSerial eeg;
    eeg.startListening();  // Start the data reception thread

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Use getDataMutex() to access the mutex
        {
            std::lock_guard<std::mutex> lock(eeg.getDataMutex());
            std::cout << "[Main Thread] "
                << "Attention: " << (int)EEGSerial::attention
                << ", Meditation: " << (int)EEGSerial::meditation
                << std::endl;
        }
    }

    return 0;
}
