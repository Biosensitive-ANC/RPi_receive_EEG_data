#include "EEG.h"

// Initialize static global variables
uint8_t EEGSerial::attention = 0;
uint8_t EEGSerial::meditation = 0;

EEGSerial::EEGSerial() : receive_ok(false) {
    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        std::cerr << "Error: Cannot open " << SERIAL_PORT << std::endl;
        exit(1);
    }
    configureSerialPort();
}

EEGSerial::~EEGSerial() {
    stopListening();  // Stop listening thread on exit
    close(fd);
}

void EEGSerial::configureSerialPort() {
    struct termios options;
    tcgetattr(fd, &options);

    // Set baud rate to 115200
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    // Configure serial port parameters (8N1: 8 data bits, no parity, 1 stop bit)
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 data bits

    options.c_cflag |= CREAD | CLOCAL;  // Enable receiver, ignore modem control lines
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Disable canonical mode
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    options.c_oflag &= ~OPOST; // Disable output processing

    tcsetattr(fd, TCSANOW, &options);
}

void EEGSerial::receiveData() {
    uint8_t buffer;
    int count = 0;

    while (true) {
        int bytesRead = read(fd, &buffer, 1);
        if (bytesRead > 0) {
            receive[count] = buffer;

            // Serial data packet parsing logic
            switch (count) {
            case 0: if (buffer == 0xAA) count++; else count = 0; break;
            case 1: if (buffer == 0xAA) count++; else count = 0; break;
            case 2: if (buffer == 0x20) count++; else count = 0; break;
            case 3: if (buffer == 0x02) count++; else count = 0; break;
            case 5: if (buffer == 0x83) count++; else count = 0; break;
            case 6: if (buffer == 0x18) count++; else count = 0; break;
            case 35:
                count = 0;
                receive_ok = true;
                processEEGData();
                break;
            default:
                count++;
                break;
            }
        }
    }
}

void EEGSerial::processEEGData() {
    if (receive_ok) {
        receive_ok = false;
        uint8_t new_attention = receive[32];
        uint8_t new_meditation = receive[34];

        // Compute checksum
        uint16_t checksum = 0;
        for (int i = 3; i < 35; i++) {
            checksum += receive[i];
        }
        checksum = (~checksum) & 0xFF;

        if (checksum == receive[35]) {
            std::lock_guard<std::mutex> lock(dataMutex);
            attention = new_attention;
            meditation = new_meditation;

            std::cout << "[EEG Update] Attention: " << (int)attention
                << ", Meditation: " << (int)meditation << std::endl;
        }
        else {
            std::cout << "[EEG] Checksum Error!" << std::endl;
        }
    }
}

// Start the listening thread
void EEGSerial::startListening() {
    receiveThread = std::thread(&EEGSerial::receiveData, this);
}

// Stop the listening thread
void EEGSerial::stopListening() {
    if (receiveThread.joinable()) {
        receiveThread.detach();  // Detach the thread to run independently
    }
}

std::mutex& EEGSerial::getDataMutex()
{
    return dataMutex;
}
