#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <chrono>

const int CHUNK_SIZE = 1024;  // Set chunk size to 1K bytes

// Function to send file contents
void send_file(const std::string& filename, int socket) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        std::string errorMsg = "Error: Failed to open file: " + filename;
        send(socket, errorMsg.c_str(), errorMsg.size(), 0); // Send error message to server
        return;
    }

    char buffer[CHUNK_SIZE];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int bytes_read = file.gcount();
        if (send(socket, buffer, bytes_read, 0) < 0) {
            std::cerr << "Failed to send file chunk." << std::endl;
            return;
        }
    }

    file.close();
    std::cout << "File sent successfully: " << filename << std::endl;

    // Send EOF in a separate transmission
    std::string eof_msg = "EOF";
    send(socket, eof_msg.c_str(), eof_msg.size(), 0);
}


// Function to receive file contents and save locally
void receive_file(const std::string& filename, int socket) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for receiving: " << filename << std::endl;
        return;
    }

    char buffer[CHUNK_SIZE];
    std::string data;
    while (true) {
        int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Error or connection closed while receiving file data." << std::endl;
            break;
        }
        
        data.append(buffer, bytes_received);
        // Check for EOF message in the buffer
        if (data.size() >= 3 && data.substr(data.size() - 3) == "EOF") {
            file.write(data.c_str(), data.size() - 3); // Write data except EOF
            break;
        } else {
            file.write(data.c_str(), data.size());
            data.clear();
        }
    }

    file.close();
    std::cout << "File received and saved as: " << filename << std::endl;

    // Send confirmation back to the client
    std::string confirmation = "File received successfully.\n";
    send(socket, confirmation.c_str(), confirmation.size(), 0);
}



void handle_client(int client_sock, std::string username) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << "The other user has disconnected or an error occurred." << std::endl;
            break; // Connection closed or error
        }
        std::string message(buffer, bytes_received);

        // Check if the message is an error message regarding file transfer
        if (message.find("Error: Failed to open file:") == 0) {
            std::cerr << message << std::endl; // Display error message
            continue; // Continue listening for other messages or commands
        }
        
        if (message.substr(0, 9) == "transfer ") {
            std::string filename = message.substr(9);
            receive_file("received_" + filename, client_sock);
            std::string confirmation = "File '" + filename + "' received successfully.\n";
            send(client_sock, confirmation.c_str(), confirmation.size(), 0);
        } else if (message == "EOF") {
            continue; // Ignore EOF messages that come without file data
        } else {
            std::cout << message << std::endl; // Normal message
        }
    }
    close(client_sock);
}




void server_thread(int listen_port, std::string username) {
    int server_fd, client_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << username << ": Could not create socket" << std::endl;
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(listen_port);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror((username + ": Bind failed. Error").c_str());
        return;
    }

    listen(server_fd, 3);
    std::cout << username << " is running.\nthe server port number is: " << listen_port << std::endl;

    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, &c))) {
        std::thread client_thread(handle_client, client_sock, username);
        client_thread.detach();
    }

    if (client_sock < 0) {
        perror((username + ": Accept failed").c_str());
        return;
    }
}

void client_thread(std::string host, int connect_port, std::string username) {
    int sock;
    struct sockaddr_in server;
    char message[1024];
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << username << ": Could not create socket" << std::endl;
        return;
    }

    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(connect_port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        std::cerr << username << ": Failed to connect to port " << connect_port << std::endl;
        return;
    }

    std::cout << "Connected to " << (username == "Alice" ? "Bob" : "Alice") << " in port " << connect_port << std::endl;

    while (true) {
        std::cin.getline(message, sizeof(message));
        if (strncmp(message, "transfer ", 9) == 0) {
            std::string filename = message + 9;
            std::ifstream testFile(filename, std::ios::binary);
            if (!testFile.is_open()) {
                std::cerr << "Failed to open file: " << filename << std::endl;
                continue;  // Skip attempting to send the file and proceed to get next input
            }
            send(sock, message, strlen(message), 0);  // Send transfer command
            send_file(filename, sock);  // Send the file
            testFile.close();

            // Wait for server confirmation
            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer) - 1, 0);
            std::cout << "Server confirmation: " << buffer << std::endl;
        } else if (strcmp(message, "quit") == 0) {
            send(sock, message, strlen(message), 0);
            break;
        } else {
            char fullMessage[1024];
            snprintf(fullMessage, sizeof(fullMessage), "%s: %s", username.c_str(), message);
            send(sock, fullMessage, strlen(fullMessage), 0);
        }
    }

    close(sock);
}

int main() {
    srand(time(nullptr));
    int listen_port = 1025 + rand() % (65535 - 1025);
    int connect_port;
    std::string host = "127.0.0.1";
    std::string username;

    std::cout << "Enter your username: ";
    std::cin >> username;
    std::cin.ignore();

    std::thread server(server_thread, listen_port, username);
    server.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Hello, please input a target port number for " << (username == "Alice" ? "Bob" : "Alice") << ": ";
    std::cin >> connect_port;
    std::cin.ignore();

    std::thread client(client_thread, host, connect_port, username);
    client.join();

    return 0;
}
