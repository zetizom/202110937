#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define REQ_FIFO "/tmp/request_fifo"
#define RES_FIFO_TEMPLATE "/tmp/response_%d"
#define MAX_BUF 512

// 사용법 출력
void usage(const char *progname) {
    fprintf(stderr, "Usage: %s <filename> <password>\n", progname);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    char *filename = argv[1];
    char *password = argv[2];
    int client_pid = getpid(); // 클라이언트 PID 획득
    
    char request_msg[MAX_BUF];
    char response_msg[MAX_BUF];
    char res_fifo_path[128];
    
    int fd_req, fd_res;
    
    // 요청 메시지 구성
    snprintf(request_msg, MAX_BUF, "DECRYPT %s %s %d", filename, password, client_pid);
    
    // 요청 FIFO 경로 생성
    snprintf(res_fifo_path, sizeof(res_fifo_path), RES_FIFO_TEMPLATE, client_pid);
    
    // 클라이언트가 먼저 응답 FIFO를 생성
    if (mkfifo(res_fifo_path, 0666) == -1 && errno != EEXIST) {
        perror("Client: mkfifo failed");
        return 1;
    }

    // 요청 FIFO에 명령 전송
    fd_req = open(REQ_FIFO, O_WRONLY);
    if (fd_req == -1) {
        perror("Client: Failed to open request FIFO");
        unlink(res_fifo_path); // 실패 시 생성한 FIFO 삭제
        return 1;
    }

    printf("Client %d: Sending request to Daemon...\n", client_pid);
    write(fd_req, request_msg, strlen(request_msg) + 1);
    close(fd_req);

    // 응답 대기
    printf("Client %d: Waiting for response via unique FIFO: %s\n", client_pid, res_fifo_path);
    

    fd_res = open(res_fifo_path, O_RDONLY);
    if (fd_res == -1) {
        perror("Client: Failed to open response FIFO");
        unlink(res_fifo_path);
        return 1;
    }
    
    // 응답 읽기
    ssize_t bytes_read = read(fd_res, response_msg, MAX_BUF - 1);
    if (bytes_read > 0) {
        response_msg[bytes_read] = '\0';
        printf("Client %d: Received result: %s\n", client_pid, response_msg);
    } else {
        printf("Client %d: Daemon/Worker closed channel before sending response.\n", client_pid);
    }
    
    // 정리 및 자원 반납
    close(fd_res); // 응답 FIFO 닫기
    
    // 자신의 고유 응답 FIFO를 파일 시스템에서 제거
    if (unlink(res_fifo_path) == 0) {
        printf("Client %d: Successfully unlinked response FIFO.\n", client_pid);
    } else {
        perror("Client: Failed to unlink response FIFO");
    }

    return 0;
}
