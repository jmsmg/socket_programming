#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <unistd.h>
#include <cstring>

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0); // 리스닝 소켓 생성
    if (listen_fd == -1) {
        std::cerr << "소켓 생성 실패\n";
        return 1;
    }

    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345); // 포트 설정

    if (bind(listen_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        std::cerr << "바인딩 실패\n";
        return 1;
    }

    if (listen(listen_fd, SOMAXCONN) == -1) {
        std::cerr << "리스닝 실패\n";
        return 1;
    }

    int kq = kqueue(); // kqueue 생성
    if (kq == -1) {
        std::cerr << "kqueue 생성 실패\n";
        return 1;
    }

    struct kevent change;
    EV_SET(&change, listen_fd, EVFILT_READ, EV_ADD, 0, 0, NULL); // 리스닝 소켓에 대한 이벤트 설정

    struct kevent events[64];
    int num_events = 1; // 현재 이벤트 개수 (처음에는 리스닝 소켓 하나만 모니터링)

    while (true) {
        int ret = kevent(kq, &change, 1, events, num_events, NULL); // 이벤트 대기
        if (ret == -1) {
            std::cerr << "이벤트 대기 실패\n";
            break;
        }

        for (int i = 0; i < ret; ++i) {
            if (events[i].ident == listen_fd) { // 클라이언트 연결 요청이 있는 경우
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len); // 클라이언트 연결 수락
                if (client_fd == -1) {
                    std::cerr << "클라이언트 연결 수락 실패\n";
                    continue;
                }
                std::cout << "클라이언트가 연결되었습니다.\n";

                EV_SET(&change, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL); // 클라이언트 소켓에 대한 이벤트 설정
                ++num_events;
            } else { // 클라이언트로부터 메시지가 도착한 경우
                char buffer[1024];
                ssize_t bytes_read = recv(events[i].ident, buffer, sizeof(buffer), 0); // 데이터 수신
                if (bytes_read <= 0) {
                    std::cerr << "클라이언트 연결 종료\n";
                    close(events[i].ident);
                    EV_SET(&change, events[i].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL); // 이벤트 삭제
                    --num_events;
                } else {
                    send(events[i].ident, buffer, bytes_read, 0); // 클라이언트에게 메시지 전송
                }
            }
        }
    }

    close(listen_fd); // 리스닝 소켓 닫기
    close(kq); // kqueue 닫기

    return 0;
}