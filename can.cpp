#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include <thread>
#include <mutex>
#include <deque>
#include <unistd.h>
#include <random>

bool stringInformation = false;

std::mutex cout_mutex; // Mutex for protecting std::cout
std::mutex queue_mutex; // Mutex for protecting the channel

int received = 0;
int sent = 0;

// Define a CAN message struct to hold ID, DLC, and data
struct CANMessage {
    int arbID;
    int dlc;
    int data[8]; // Max 8 bytes of data
};

std::map<int, std::string> component;

// Print out CAN message
void printCANMessage(int arbID, int dlc, int* data) {
    if (stringInformation){
        std::cout << component[arbID];
    }
    else{
        std::cout << "ArbID: " << std::hex << arbID;
        std::cout << "\tDLC: " << std::hex << dlc;
        std::cout << "\tData: ";
        for (int i = 0; i < dlc; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << data[i] << " ";
        }
    }
    std::cout << std::endl;
}

CANMessage generateCANMessage(int arbID, int dlc) {
    CANMessage message;
    message.arbID = arbID;
    message.dlc = dlc;

    std::random_device rd;  // Obtain a random number from hardware
    std::mt19937 eng(rd()); // Seed the generator
    std::uniform_int_distribution<> distr(0, 255); // Define the range

    for (int i = 0; i < dlc; ++i) {
        message.data[i] = distr(eng); // Generate random integer in the range [0, 255]
    }
    return message;
}

// Function to print the channel contents
void printChannel(std::deque<CANMessage>& channel) {
    std::unique_lock<std::mutex> lock(queue_mutex); // Lock the channel mutex
    std::deque<CANMessage> temp = channel; // Create a temporary copy
    lock.unlock();

    std::unique_lock<std::mutex> lock2(cout_mutex); // Lock cout to avoid mixed output
    std::cout << "\nChannel Contents:" << std::endl;
    while (!temp.empty()) {
        CANMessage message = temp.front();
        temp.pop_front();
        printCANMessage(message.arbID, message.dlc, message.data);
    }
    lock2.unlock();
}

// Simulated CAN sending function
void sendCANMessage(int arbID, int dlc, int* data, std::deque<CANMessage>& channel) {
    std::unique_lock<std::mutex> lock(queue_mutex); // Lock the channel mutex
    channel.push_back({arbID, dlc, {data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]}});
    lock.unlock();

    std::unique_lock<std::mutex> lock2(cout_mutex); // Lock cout to avoid mixed output
    std::cout << "Sending CAN Message: ";
    printCANMessage(arbID, dlc, data);
    lock2.unlock();
}

// Simulated CAN receiving function
void receiveCANMessage(int arbID, int dlc, std::deque<CANMessage>& channel) {
    int expectedID = arbID;
    while (true) {
        // if (received >= sent){
        //     break;
        // }
        // printChannel(channel);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for a short time to avoid busy-waiting
        std::unique_lock<std::mutex> lock(queue_mutex); // Lock the channel mutex

        if (!channel.empty()) {
            CANMessage message = channel.front();
            channel.pop_front();
            lock.unlock(); // Unlock before processing the message

            if (message.arbID == expectedID) {
                received++;
                std::unique_lock<std::mutex> lock2(cout_mutex); // Lock cout to avoid mixed output
                std::cout << "Received CAN Message: ";
                printCANMessage(message.arbID, message.dlc, message.data);
                lock2.unlock();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distrib(1, 500); // Generate a random number between 1 and 49

                // Generate random duration
                int randomDuration = distrib(gen);

                std::this_thread::sleep_for(std::chrono::milliseconds(randomDuration)); // Sleep for a short time to avoid busy-waiting

                CANMessage message1 = generateCANMessage(arbID, dlc);
                std::thread senderThread1(sendCANMessage, message1.arbID, message1.dlc, message1.data, std::ref(channel));
                senderThread1.join();
            } else {
                channel.push_front(message);
            } 
        } 
        else if (channel.empty() && received == sent){
            break;
        }
        else {
            lock.unlock(); // Unlock if the queue is empty
        }
    }
}

int main() {
    // Define CAN components
    component[0x001] = "Engine Information";
    component[0x002] = "Transmission Information";
    component[0x003] = "ABS Information";
    component[0x004] = "Airbag Information";
    component[0x005] = "Climate Control Information";
    component[0x006] = "Entertainment System Information";
    component[0x007] = "Steering System Information";
    component[0x008] = "Suspension System Information";

    std::deque<CANMessage> channel; // Channel to store CAN messages

    for(int i=0;i<1;i++){
        sent += 8;

        // Generate CAN messages dynamically with random data
        CANMessage message1 = generateCANMessage(0x001, 0x08);
        CANMessage message2 = generateCANMessage(0x002, 0x04);
        CANMessage message3 = generateCANMessage(0x003, 0x08);
        CANMessage message4 = generateCANMessage(0x004, 0x04);
        CANMessage message5 = generateCANMessage(0x005, 0x02);
        CANMessage message6 = generateCANMessage(0x006, 0x08);
        CANMessage message7 = generateCANMessage(0x007, 0x08);
        CANMessage message8 = generateCANMessage(0x008, 0x08);

        // Create threads for sending and receiving
        std::thread senderThread1(sendCANMessage, message1.arbID, message1.dlc, message1.data, std::ref(channel));
        std::thread senderThread2(sendCANMessage, message2.arbID, message2.dlc, message2.data, std::ref(channel));
        std::thread senderThread3(sendCANMessage, message3.arbID, message3.dlc, message3.data, std::ref(channel));
        std::thread senderThread4(sendCANMessage, message4.arbID, message4.dlc, message4.data, std::ref(channel));
        std::thread senderThread5(sendCANMessage, message5.arbID, message5.dlc, message5.data, std::ref(channel));
        std::thread senderThread6(sendCANMessage, message6.arbID, message6.dlc, message6.data, std::ref(channel));
        std::thread senderThread7(sendCANMessage, message7.arbID, message7.dlc, message7.data, std::ref(channel));
        std::thread senderThread8(sendCANMessage, message8.arbID, message8.dlc, message8.data, std::ref(channel));

        // Join threads to wait for them to finish
        senderThread1.join();
        senderThread2.join();
        senderThread3.join();
        senderThread4.join();
        senderThread5.join();
        senderThread6.join();
        senderThread7.join();
        senderThread8.join();
    }

    sent = sent/2;

    // // Wait for some time to allow messages to be sent
    // sleep(1);
    // std::cout << "\n";

    std::thread receiverThread1(receiveCANMessage, 0x001, 0x08, std::ref(channel));
    std::thread receiverThread2(receiveCANMessage, 0x002, 0x04, std::ref(channel));
    std::thread receiverThread3(receiveCANMessage, 0x003, 0x08, std::ref(channel));
    std::thread receiverThread4(receiveCANMessage, 0x004, 0x04, std::ref(channel));
    std::thread receiverThread5(receiveCANMessage, 0x005, 0x02, std::ref(channel));
    std::thread receiverThread6(receiveCANMessage, 0x006, 0x08, std::ref(channel));
    std::thread receiverThread7(receiveCANMessage, 0x007, 0x08, std::ref(channel));
    std::thread receiverThread8(receiveCANMessage, 0x008, 0x08, std::ref(channel));
    
    receiverThread1.join();
    receiverThread2.join();    
    receiverThread3.join();
    receiverThread4.join();
    receiverThread5.join();
    receiverThread6.join();    
    receiverThread7.join();
    receiverThread8.join();

    return 0;
}