# OpenSSL 라이브러리 경로 설정
# -lssl, -lcrypto는 OpenSSL을 사용할 때 필수로 링크 
CFLAGS = -Wall -std=c99
LDFLAGS = -lssl -lcrypto

# 프로젝트 실행 파일 목록
TARGETS = daemon worker client

all: $(TARGETS)

daemon: daemon.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS)

worker: worker.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS)

client: client.c
	gcc $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)
	rm -f /tmp/request_fifo
	# 응답 FIFO는 클라이언트가 정리하므로 데몬 정리 목록에서 제외
