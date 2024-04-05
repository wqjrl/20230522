#include "../include/head.h"

//设置非阻塞
void setNonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

int main() {

    //打开公有管道
    int fd_pub;
    if ((fd_pub = open("pubpipe", O_WRONLY)) == -1) {
        perror("open public pipe failed");
        exit(0);
    }
    //设置非阻塞写公有管道
    setNonblock(fd_pub);

    int fds[2];
    pipe(fds);

    int fd_pri = -1;
    int ret;
    char buf[1024];
    char snd[4096];
    char rcv[4096];
    bzero(buf, sizeof(buf));
    printf("启动成功,请使用以下命令:\n");
    printf("    [Login:name] 进行登录, 自定义name, 如aa, bb\n");

    pid_t pid = fork();
    if (pid > 0) {
        close(fds[1]);
        char name[20];
        read(fds[0], name, sizeof(name));
        close(fds[0]);
        while (1) {
            if (strlen(name) != 0) {
                usleep(10);
                break;
            }
        }

        //打开私有管道
        if ((fd_pri = open(name, O_RDONLY)) == -1) {
            perror("open private pipe failed");
            exit(0);
        }

        //设置非阻塞读私有管道
        setNonblock(fd_pri);

        printf("登录成功,请使用以下命令:\n");
        printf("    [all] 获取有效用户\n");
        printf("    [dst:data] 指定接收方dst, 向其发送数据data\n");
        printf("    [quit] 退出\n");
        //读取私有管道
        while (1) {
            if (fd_pri != -1) {
                bzero(rcv, sizeof(rcv));
                if ((ret = read(fd_pri, rcv, sizeof(rcv))) > 0) {
                    printf("%s\n", rcv);
                }
            }
        }


    } else if (0 == pid) {
        close(fds[0]);
        while (1) {
            bzero(buf, sizeof(buf));
            scanf("%s", buf);

            if (strncmp(buf, "Login", 5) == 0) { //发送 Login:name
                bzero(snd, sizeof(snd));
                sprintf(snd, "%d:%s", getpid(), buf);

                write(fd_pub, snd, sizeof(snd));
                char* tmp = strtok(buf, ":");
                tmp = strtok(NULL, ":");
                char name[20];
                strcpy(name, tmp);
                write(fds[1], name, sizeof(name));
                close(fds[1]);

            } else if (strncmp(buf, "all", 3) == 0) { //发送 all
                bzero(snd, sizeof(snd));
                sprintf(snd, "%d:%s", getpid(), buf);
                write(fd_pub, snd, sizeof(snd));

            } else if (strncmp(buf, "quit", 4) == 0) { //发送 quit
                bzero(snd, sizeof(snd));
                sprintf(snd, "%d:%s", getpid(), buf);
                write(fd_pub, snd, sizeof(snd));
                close(fd_pub);
                close(fd_pri);
                kill(getppid(), SIGKILL);
                exit(0);

            } else { //发送dest:data
                bzero(snd, sizeof(snd));
                sprintf(snd, "%d:%s", getpid(), buf);
                write(fd_pub, snd, sizeof(snd));
            }
        }

    } else {
        perror("fork failed");
        exit(0);
    }
    return 0;
}