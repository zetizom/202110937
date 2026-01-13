# ?? Linux System Programming: Daemon-based File Encryption System

## 1. 프로젝트 개요 (Overview)

리눅스 환경에서 동작하는 **데몬(Daemon) 기반의 파일 자동 암호화/복호화 시스템**입니다.
특정 디렉토리를 백그라운드에서 실시간으로 감시하다가 파일이 생성되면 자동으로 암호화하고, 클라이언트의 요청이 있을 때 복호화를 수행합니다.

* **목표:** 리눅스 시스템 프로그래밍의 핵심 개념(IPC, Signal, Process Management, File System)과 OpenSSL 라이브러리 활용 능력 습득
* **개발 언어:** C언어
* **환경:** Linux (Ubuntu 20.04+), GCC, Make

## 2. 주요 구현 기술 (Technical Specs)

이 프로젝트는 다음과 같은 시스템 프로그래밍 기법을 사용하여 구현되었습니다.

* **Process Management:**
* `fork()`, `setsid()`를 이용한 **데몬 프로세스(Daemonize)** 구현.
* `fork()`, `exec()`를 통해 작업을 수행할 **Worker 프로세스** 생성.
* `waitpid()`와 Signal Handling을 통한 좀비 프로세스(Zombie Process) 방지.


* **I/O Multiplexing:**
* `select()` 시스템 콜을 사용하여 **클라이언트 요청(FIFO)**과 **파일 생성 이벤트(inotify)**를 단일 스레드에서 동시에 모니터링.


* **Inter-Process Communication (IPC):**
* **Named Pipe (FIFO):** 클라이언트와 데몬, 데몬과 워커 간의 통신 채널로 사용.
* 클라이언트별 고유 응답 채널(`/tmp/response_[PID]`) 생성으로 다중 클라이언트 요청 처리 구조 마련.


* **File System Monitoring:**
* `inotify` API를 사용하여 디렉토리 내 파일 생성(`IN_CLOSE_WRITE`) 이벤트 실시간 감지.


* **Cryptography:**
* **OpenSSL (EVP API):** AES-256-CBC 알고리즘 적용.
* **Security:** PBKDF2를 이용한 Key Derivation, Random Salt & IV 생성으로 보안성 강화.



## 3. 소스 파일 구성 (File Structure)

* `daemon.c`: 메인 데몬 프로그램. `select()`를 통해 이벤트를 감지하고 `worker`를 fork하여 작업을 분배합니다.
* `worker.c`: 실제 암호화/복호화 로직을 수행하는 실행 파일입니다. OpenSSL 라이브러리가 사용됩니다.
* `client.c`: 데몬에게 파일 복호화를 요청하고 결과를 수신하는 사용자 프로그램입니다.
* `Makefile`: 빌드 자동화 스크립트 (`-lssl -lcrypto` 링크 포함).

## 4. 빌드 및 실행 방법 (Build & Usage)

### 의존성 설치 (Prerequisites)

OpenSSL 개발 라이브러리가 필요합니다.

```bash
sudo apt-get install libssl-dev

```

### 컴파일 (Compile)

제공된 Makefile을 사용하여 전체 프로젝트를 빌드합니다.

```bash
make all

```

### 실행 시나리오 (Execution)

**1. 데몬 실행**
데몬을 실행하면 백그라운드 프로세스로 전환되며, `./monitor` 디렉토리를 감시하기 시작합니다.

```bash
./daemon
# "Daemon started. PID: [PID]" 메시지 출력 확인

```

**2. 자동 암호화 테스트 (Auto Encryption)**
`./monitor` 디렉토리에 파일을 생성하거나 옮기면 자동으로 암호화됩니다.

```bash
echo "System Programming Project" > test.txt
mv test.txt ./monitor/
# ls -l ./monitor/ 확인 시 test.txt.enc 파일 생성됨

```

**3. 복호화 요청 (Decryption Client)**
클라이언트 프로그램을 이용해 암호화된 파일을 복구합니다. (기본 패스워드: `password`)

```bash
# Usage: ./client <파일명> <비밀번호>
./client ./monitor/test.txt.enc password

# 결과: ./restored/test.txt 파일 생성 및 성공 메시지 출력

```

## 5. 구현 시 고려사항 및 이슈 (Implementation Notes)

* **동시성 처리:** `select()`를 사용하여 FIFO와 inotify 디스크립터를 동시에 감시함으로써, 파일이 복사되는 도중에 클라이언트 요청이 들어와도 블로킹 없이 처리되도록 구현했습니다.
* **리소스 정리:** 데몬 종료 시 열려있는 FIFO 파일과 파일 디스크립터가 적절히 정리(`unlink`, `close`)되도록 처리했습니다.
* **비밀번호 정책:** 현재 자동 암호화 시 사용되는 비밀번호는 `worker.c` 내부에 `SYSTEM_PASS`로 정의되어 있습니다. (실제 배포 시에는 키 관리 시스템(KMS) 연동 필요)
