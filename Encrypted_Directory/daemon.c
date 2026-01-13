#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/inotify.h> 
#include <sys/select.h>  

#define REQ_FIFO "/tmp/request_fifo"
#define MONITOR "./monitor"
#define MAX_BUF 1024
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))


// 프로세스 데몬화
void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);
    if (setsid() < 0) exit(1);
    printf("Daemon started. PID: %d\n", getpid());
    umask(0);
}

// 좀비 프로세스 회수 
void clean_zombies() {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// 워커 프로세스 실행 
void launch_worker(const char *mode, const char *arg1, const char *arg2, const char *arg3) {
    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스 (Worker)
        char msg[MAX_BUF];
        
        // 암호화 요청
        if (strcmp(mode, "ENCRYPT") == 0) {
            
            snprintf(msg, MAX_BUF, "%s %s", mode, arg1);
        } else {
            // 암호화 요청 
            snprintf(msg, MAX_BUF, "%s %s %s %s", mode, arg1, arg2, arg3);
        }
        
        // 워커 프로세스로 교체
        char *worker_path = "./worker";
        execl(worker_path, worker_path, msg, (char *)NULL);
        perror("execl failed");
        exit(1);
    }
}

int main() {
    int fd_fifo, fd_inotify, wd;
    fd_set readfds;
    char buffer[MAX_BUF];

    // 감시 디렉토리 생성
    mkdir(MONITOR, 0777);

    daemonize();

    // FIFO 생성 및 열기
    if (mkfifo(REQ_FIFO, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo failed"); exit(1);
    }  
    fd_fifo = open(REQ_FIFO, O_RDWR); 

    // Inotify 초기화 및 감시 설정
    fd_inotify = inotify_init();
    if (fd_inotify < 0) { perror("inotify_init"); exit(1); }

    // 파일 쓰기가 끝나고 닫힐 때 이벤트 발생 
    wd = inotify_add_watch(fd_inotify, MONITOR, IN_CLOSE_WRITE);
    if (wd < 0) { perror("inotify_add_watch"); exit(1); }

    printf("Daemon: Monitoring FIFO and Directory '%s'...\n", MONITOR);

    while (1) {
        // 좀비 프로세스 정리
        clean_zombies();

        // select를 이용해서 클라이언트 요청, 디렉토리 감시 동시 수행
        FD_ZERO(&readfds);
        FD_SET(fd_fifo, &readfds);
        FD_SET(fd_inotify, &readfds);
        int max_fd = (fd_fifo > fd_inotify) ? fd_fifo : fd_inotify;

        // 이벤트 대기
        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue; // 시그널 인터럽트 무시
            perror("select error"); break;
        }

        // 클라이언트 요청 (FIFO) 발생
        if (FD_ISSET(fd_fifo, &readfds)) {
            int n = read(fd_fifo, buffer, MAX_BUF - 1);
            if (n > 0) {
                buffer[n] = '\0';                
                char cmd[16], fname[256], pass[256], pid_str[16];
                sscanf(buffer, "%s %s %s %s", cmd, fname, pass, pid_str);
                
                printf("Daemon: Client Request -> %s\n", buffer);
                launch_worker(cmd, fname, pass, pid_str);
            }
        }

        // 파일 생성 이벤트 발생
        if (FD_ISSET(fd_inotify, &readfds)) {
            int length = read(fd_inotify, buffer, BUF_LEN);
            int i = 0;
            while (i < length) {
                struct inotify_event *event = (struct inotify_event *)&buffer[i];
                if (event->len) {
                    if (event->mask & IN_CLOSE_WRITE) {
                        // 암호화된 파일(.enc)이 생성된 것은 무시
                        if (strstr(event->name, ".enc") == NULL) {
                            char full_path[512];
                            snprintf(full_path, sizeof(full_path), "%s/%s", MONITOR, event->name);
                            
                            printf("Daemon: New File Detected -> %s\n", full_path);
                            // 암호화 워커 실행 
                            launch_worker("ENCRYPT", full_path, NULL, NULL);
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

    close(fd_fifo);
    close(fd_inotify);  
    unlink(REQ_FIFO); // FIFO 및, inotify 정리
    return 0;
}