# 실시간 암호화 디렉토리 프로그램 (Real-time Encryption Directory Program)

## 1. 프로젝트 정보 (Project Info)

* **과목명:** UNIX Programming
 
* **프로젝트 주제:** 실시간 디렉토리 감시 및 자동 암호화/복호화 시스템 



* **개발자:** 202110937 서형완 



## 2. 프로젝트 개요 (Overview)

이 프로젝트는 리눅스 환경에서 특정 디렉토리(`monitor`)를 실시간으로 감시하는 **데몬(Daemon)** 프로세스를 기반으로 동작합니다. 일반 파일이 디렉토리에 생성되면 이를 감지하여 자동으로 암호화한 후 원본 파일을 삭제하며, 사용자가 **클라이언트(Client)** 프로세스를 통해 요청할 경우 비밀번호 인증을 거쳐 파일을 복호화합니다.

전체 시스템은 **Daemon**, **Worker**, **Client**의 세 가지 프로세스로 구성되어 유기적으로 동작합니다.

## 3. 시스템 구조 (System Architecture)

### 3.1. 프로세스 구성


* **Daemon Process:** `monitor` 디렉토리 감시 및 클라이언트의 요청을 대기하는 백그라운드 프로세스입니다. 



* **Worker Process:** 데몬에 의해 `fork`되어 실제 암호화 및 복호화 작업을 수행하는 프로세스입니다. 


 
* **Client Process:** 유저로부터 복호화 요청과 비밀번호를 입력받아 데몬에게 전달하는 인터페이스입니다. 



### 3.2. 동작 흐름

 
* **암호화:** `monitor` 폴더 생성 → 폴더 내 파일 발생 감지 → 데몬이 워커 `fork` → 워커가 암호화 수행 후 원본 삭제 



* **복호화:** 유저 복호화 요청 → 클라이언트가 메시지 전송(FIFO) → 데몬이 이벤트 수신 후 워커 `fork` → 워커가 복호화 수행 및 완료 신호 전송 



## 4. 사용 기술 및 라이브러리 (Tech Stack)

이 프로젝트는 **UNIX System Call**과 **OpenSSL** 라이브러리를 적극적으로 활용하여 구현되었습니다.

### UNIX System Programming

* **Process Management:**

	- `fork()`: Worker 프로세스 생성 



	- `setsid()`: Daemon 프로세스를 터미널과 분리 



	- `execl()`: Daemon에서 Worker 프로세스로 교체 


	- `waitpid()`: Worker 프로세스 종료 대기 (좀비 프로세스 방지) 




* **File System & Monitoring:**

	- `inotify_init()`, `inotify_add_watch()`: 디렉토리 내 파일 생성 이벤트 감지 



	- `select()`: Client 요청 대기와 디렉토리 감시 동시 수행 (I/O Multiplexing) 



	- `unlink()`, `rename()`: 원본 삭제 및 파일 확장자(.enc) 변경 




* **IPC (Inter-Process Communication):**

	- `mkfifo()`: 프로세스 간 통신을 위한 FIFO 생성 



	- `open()`, `write()`, `read()`: FIFO 및 파일 입출력 





### Cryptography (OpenSSL)


* **Algorithm:** AES-256-CBC 모드 사용 



* **Key Derivation:** `PKCS5_PBKDF2_HMAC()`을 사용하여 비밀번호와 Salt를 강력한 키로 변환 



* **EVP API:** `EVP_EncryptInit_ex`, `EVP_EncryptUpdate`, `EVP_EncryptFinal_ex` 등을 통한 암호화/복호화 수행 및 패딩 처리 



## 5. 빌드 및 실행 방법 (How to Run)

### 5.1. 컴파일

제공된 `Makefile`을 사용하여 프로젝트를 빌드합니다.

```bash
make

```

* 빌드 후 `daemon`, `worker`, `client` 실행 파일이 생성됩니다. 



### 5.2. 실행 및 테스트

**Step 1: 데몬 실행**
데몬을 실행하면 `./monitor` 디렉토리가 자동으로 생성됩니다.

```bash
./daemon
# 출력: Daemon started. PID: [PID]

```



**Step 2: 파일 암호화 테스트**
`monitor` 디렉토리에 파일을 복사하거나 생성하면, 즉시 암호화되어 `.enc` 파일로 변환되고 원본은 삭제됩니다.

```bash
cp git.md monitor/
# 결과: monitor/git.md -> monitor/git.md.enc 변환됨

```



**Step 3: 파일 복호화 테스트**
클라이언트 프로그램을 사용하여 암호화된 파일을 복구합니다. 복구된 파일은 `restored` 디렉토리에 저장됩니다.

```bash
# 사용법: ./client monitor/[암호화파일명] [비밀번호]
./client monitor/git.md.enc password

```



**Step 4: 종료**
데몬 프로세스를 종료하려면 `kill` 명령어를 사용합니다.

```bash
kill [Daemon PID]

```



## 6. 디렉토리 구조

* `monitor/`: 감시 대상 디렉토리. 이곳에 파일이 들어오면 암호화됩니다. 



* `restored/`: 복호화 요청 시 원본 파일이 복구되어 저장되는 디렉토리입니다.