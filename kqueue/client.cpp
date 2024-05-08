#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성
    if (sockfd == -1) {
        std::cerr << "소켓 생성 실패\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // 서버 포트
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 서버 IP

    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        std::cerr << "서버 연결 실패\n";
        return 1;
    }

    std::cout << "서버에 연결되었습니다.\n";

    while (true) {
        std::string message;
        std::cout << "전송할 메시지 입력: ";
        std::getline(std::cin, message);

        ssize_t bytes_sent = send(sockfd, message.c_str(), message.length(), 0); // 메시지 전송
        if (bytes_sent == -1) {
            std::cerr << "메시지 전송 실패\n";
            break;
        }

        char buffer[1024];
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer), 0); // 서버로부터 응답 받기
        if (bytes_received == -1) {
            std::cerr << "응답 받기 실패\n";
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << "서버 응답: " << buffer << std::endl;

        if (message == "exit") {
            break;
        }
    }

    close(sockfd); // 소켓 닫기

    return 0;
}
