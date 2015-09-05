all: edp.exe
 
CC=gcc
CFLAGS=-I. -g -D_LINUX -lm -lpthread
CLIENT_OBJ = cJSON.o EdpKit.o Main.o

# 如果需要加密功能且系统内已经安装openssl，
# 取消以下两行注释
#CFLAGS+=-D_ENCRYPT -lcrypto
#CLIENT_OBJ += Openssl.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

edp.exe: $(CLIENT_OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f edp.exe $(CLIENT_OBJ)
