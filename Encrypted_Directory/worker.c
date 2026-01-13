#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h> 
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define REQ_FIFO "/tmp/request_fifo"
#define RES_FIFO_TEMPLATE "/tmp/response_%d"
#define MAX_BUF 512
#define AES_KEYLEN 32  // AES-256 key size
#define AES_IVLEN 16   // AES block size (128-bit)
#define SALT_LEN 16    // Salt size
#define ITERATIONS 10000 // PBKDF2 iterations
#define SYSTEM_PASS "password" // 자동 암호화 시 사용할 기본 비밀번호

// OpenSSL 에러 출력
void handle_openssl_error() {
    ERR_print_errors_fp(stderr);
}

// 키 유도 (기본 비밀번호를 안전한 키로 변환) 
int derive_key(const char *password, unsigned char *salt, unsigned char *key, unsigned char *iv) {
    unsigned char key_iv[AES_KEYLEN + AES_IVLEN];
    
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LEN, ITERATIONS, 
                           EVP_sha256(), sizeof(key_iv), key_iv)) {
        handle_openssl_error();
        return -1;
    }
    
    memcpy(key, key_iv, AES_KEYLEN);
    memcpy(iv, key_iv + AES_KEYLEN, AES_IVLEN);
    return 0;
}

// 암호화 함수 (AES-256-CBC)
int aes_encrypt_file(const char *in_filename, const char *out_filename, const char *password) {
    FILE *ifp = fopen(in_filename, "rb");
    FILE *ofp = fopen(out_filename, "wb");
    if (!ifp || !ofp) return -1;

    unsigned char salt[SALT_LEN]; // 비밀번호에 붙일 salt 값
    unsigned char key[AES_KEYLEN]; // 키 값
    unsigned char iv[AES_IVLEN]; // iv값

    
    // 현재 시간을 시드로 난수 생성기 초기화
    srand((unsigned int)time(NULL)); 
    
    // rand()를 호출하여 salt 배열 채우기
    for (int i = 0; i < SALT_LEN; i++) {
        salt[i] = (unsigned char)(rand() % 256);
    }
    
    // 생성된 Salt를 기록 (복호화 시 필요)
    if (fwrite(salt, 1, SALT_LEN, ofp) != SALT_LEN) return -1;

    // 키 유도
    if (derive_key(password, salt, key, iv) < 0) return -1;

    // 암호화 컨텍스트 설정
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handle_openssl_error(); return -1;
    }

    // 파일 읽기 -> 암호화 -> 쓰기 과정루프
    unsigned char inbuf[1024];
    unsigned char outbuf[1024 + AES_IVLEN];
    int inlen, outlen;

    // 암호화 과정 수행
    while ((inlen = fread(inbuf, 1, sizeof(inbuf), ifp)) > 0) {
        if (1 != EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            handle_openssl_error(); return -1;
        }
        fwrite(outbuf, 1, outlen, ofp);
    }

    // 패딩 처리 및 마무리
    if (1 != EVP_EncryptFinal_ex(ctx, outbuf, &outlen)) {
        handle_openssl_error(); return -1;
    }
    fwrite(outbuf, 1, outlen, ofp);

    EVP_CIPHER_CTX_free(ctx);
    fclose(ifp);
    fclose(ofp);
    return 0;
}

// 복호화 함수 (AES-256-CBC)
int aes_decrypt_file(const char *in_filename, const char *out_filename, const char *password) {
    
    mkdir("./restored", 0777); // 복호화 폴더 자동 생성   
    FILE *ifp = fopen(in_filename, "rb");
    if (!ifp) return -1;

    unsigned char salt[SALT_LEN];
    unsigned char key[AES_KEYLEN];
    unsigned char iv[AES_IVLEN];

    // Salt 읽기
    if (fread(salt, 1, SALT_LEN, ifp) != SALT_LEN) { fclose(ifp); return -1; }

    // 키 유도
    if (derive_key(password, salt, key, iv) < 0) { fclose(ifp); return -1; }

    FILE *ofp = fopen(out_filename, "wb");
    if (!ofp) { fclose(ifp); return -1; }

    // 암호화 컨텍스트 설정
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        handle_openssl_error(); return -1;
    }

    unsigned char inbuf[1024 + AES_IVLEN];
    unsigned char outbuf[1024 + AES_IVLEN];
    int inlen, outlen;

    // 복호화 과정 수행
    while ((inlen = fread(inbuf, 1, sizeof(inbuf), ifp)) > 0) {
        if (1 != EVP_DecryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            handle_openssl_error(); return -1;
        }
        fwrite(outbuf, 1, outlen, ofp);
    }

    // 패딩 검증 및 마무리
    if (1 != EVP_DecryptFinal_ex(ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(ifp);
        fclose(ofp);
        unlink(out_filename); 
        return -2; 
    }
    fwrite(outbuf, 1, outlen, ofp);

    EVP_CIPHER_CTX_free(ctx);
    fclose(ifp);
    fclose(ofp);
    return 0;
}

// --- 결과 전송 함수 ---
void send_result(int client_pid, const char *status_msg) {
    char res_fifo_path[128];
    int fd_res;
    snprintf(res_fifo_path, sizeof(res_fifo_path), RES_FIFO_TEMPLATE, client_pid);
    
    fd_res = open(res_fifo_path, O_WRONLY);
    if (fd_res == -1) {
        perror("Worker: open response FIFO failed");
        return;
    }
    write(fd_res, status_msg, strlen(status_msg));
    close(fd_res);
}

// 메인 함수
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    char cmd[16];
    char arg1[256] = {0};
    char arg2[256] = {0};
    char arg3[16] = {0};
    

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    sscanf(argv[1], "%s %s %s %s", cmd, arg1, arg2, arg3);

    // 암호화 요청일 경우 암호화 수행 
    if (strcmp(cmd, "ENCRYPT") == 0) {
        printf("Worker: Encrypting %s (System Auto)...\n", arg1);
        char out_file[300];
        snprintf(out_file, sizeof(out_file), "%s.enc", arg1);

        // 암호화 수행 
        if (aes_encrypt_file(arg1, out_file, SYSTEM_PASS) == 0) { 
            unlink(arg1); // 원본 삭제
            printf("Worker: Encrypted to %s\n", out_file);
        } else {
            printf("Worker: Encryption Failed.\n");
        }
        return 0;
    } // 복호화 요청일 경우 복호화 수행 
    else if (strcmp(cmd, "DECRYPT") == 0) {
        int client_pid = atoi(arg3);
        printf("Worker: Decrypting %s (Client %d)...\n", arg1, client_pid);

        char *base_name = strrchr(arg1, '/');
        base_name = (base_name) ? base_name + 1 : arg1; 

        char clean_name[256];
        strncpy(clean_name, base_name, strlen(base_name) - 4);
        clean_name[strlen(base_name) - 4] = '\0'; // .enc 제거

        char out_file[512];
        snprintf(out_file, sizeof(out_file), "./restored/%s", clean_name);

        // 복호화 수행 
        int ret = aes_decrypt_file(arg1, out_file, arg2);
        char msg[MAX_BUF];

        // 성공 실패 여부 출력
        if (ret == 0) {  
            unlink(arg1); 
            snprintf(msg, MAX_BUF, "SUCCESS: Restored to %s", out_file);
        } else if (ret == -2) {
            snprintf(msg, MAX_BUF, "FAILED: Wrong Password");
            unlink(out_file); 
        } else {
            snprintf(msg, MAX_BUF, "FAILED: System Error");
        }
        
        // 최종 메시지 클라이언트에 전송
        send_result(client_pid, msg); 
        return 0;
    }

    EVP_cleanup();
    ERR_free_strings();
    return 1;
}
