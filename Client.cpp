#include <iostream>
#include <climits>
#include <thread>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 1024;

int main() {
    int client_fd;
    sockaddr_in server_addr{};

    // Create an SCTP socket
    client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (client_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(client_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_fd);
        return 1;
    }

    const char* message = "Hello, SCTP Server!";
    send(client_fd, message, strlen(message), 0);
    std::cout << "Message sent: " << message << std::endl;

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        std::cout << "Response from server: " << buffer << std::endl;
    } else {
        std::cerr << "Failed to receive response from server.\n";
    }

    close(client_fd);
    return 0;
}
