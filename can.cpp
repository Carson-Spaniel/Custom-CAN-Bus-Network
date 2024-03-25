#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include <thread>
#include <mutex>
#include <deque>
#include <unistd.h>

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

// Print out CAN message
void printCANMessage(int arbID, int dlc, int* data) {
    std::cout << "ArbID: " << std::hex << arbID;
    std::cout << "\tDLC: " << std::hex << dlc;
    std::cout << "\tData: ";
    for (int i = 0; i < dlc; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << data[i] << " ";
    }
    std::cout << std::endl;
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
void receiveCANMessage(int arbID, int expectedID, int dlc, std::deque<CANMessage>& channel) {
    while (true) {
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
                break;
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
    std::map<int, std::string> component;

    // Define CAN components
    component[0x001] = "Engine Information";
    component[0x002] = "Transmission Information";
    component[0x003] = "ABS Information";
    component[0x004] = "Airbag Information";
    component[0x005] = "Climate Control Information";

    std::deque<CANMessage> channel; // Channel to store CAN messages

    // Simulated CAN messages
    int message1[] = {0x001, 0x08, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x2a, 0x00};
    int message2[] = {0x002, 0x04, 0x11, 0x22, 0x33, 0x44};
    int message3[] = {0x003, 0x03, 0xaa, 0xbb, 0xcc};
    int message4[] = {0x004, 0x06, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    int message5[] = {0x005, 0x02, 0x01, 0x02};
    int message6[] = {0x006, 0x05, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e};
    int message7[] = {0x007, 0x03, 0x55, 0x66, 0x77};
    int message8[] = {0x008, 0x04, 0xaa, 0xbb, 0xcc, 0xdd};

    // Create threads for sending and receiving
    std::thread senderThread1(sendCANMessage, message1[0], message1[1], message1 + 2, std::ref(channel));
    std::thread senderThread2(sendCANMessage, message2[0], message2[1], message2 + 2, std::ref(channel));
    std::thread senderThread3(sendCANMessage, message3[0], message3[1], message3 + 2, std::ref(channel));
    std::thread senderThread4(sendCANMessage, message4[0], message4[1], message4 + 2, std::ref(channel));
    std::thread senderThread5(sendCANMessage, message5[0], message5[1], message5 + 2, std::ref(channel));
    std::thread senderThread6(sendCANMessage, message6[0], message6[1], message6 + 2, std::ref(channel));
    std::thread senderThread7(sendCANMessage, message7[0], message7[1], message7 + 2, std::ref(channel));
    std::thread senderThread8(sendCANMessage, message8[0], message8[1], message8 + 2, std::ref(channel));

    // Increment the number of messages sent
    sent = 8;

    sleep(1); // Wait for some time to allow messages to be sent
    std::cout << "\n";

    std::thread receiverThread1(receiveCANMessage, message1[0], 0x001, message1[1], std::ref(channel));
    std::thread receiverThread2(receiveCANMessage, message2[0], 0x002, message2[1], std::ref(channel));
    std::thread receiverThread3(receiveCANMessage, message3[0], 0x003, message3[1], std::ref(channel));
    std::thread receiverThread4(receiveCANMessage, message4[0], 0x004, message4[1], std::ref(channel));
    std::thread receiverThread5(receiveCANMessage, message5[0], 0x005, message5[1], std::ref(channel));
    std::thread receiverThread6(receiveCANMessage, message6[0], 0x006, message6[1], std::ref(channel));
    std::thread receiverThread7(receiveCANMessage, message7[0], 0x007, message7[1], std::ref(channel));
    std::thread receiverThread8(receiveCANMessage, message8[0], 0x008, message8[1], std::ref(channel));
    
    // Join threads to wait for them to finish
    senderThread1.join();
    senderThread2.join();
    senderThread3.join();
    senderThread4.join();
    senderThread5.join();
    senderThread6.join();
    senderThread7.join();
    senderThread8.join();

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
