#include "../include/head.h"

int main() {

    char snd[4096];
    char rcv[4096];
    map<pid_t, string> pidToName;
    map<pid_t, int> pidToFd;

    //创建并打开公有管道
    int ret;
    if ((ret = mkfifo("pubpipe", 0664)) == -1) {
        perror("mkfifo public pipe failed");
        exit(0);
    }
    printf("创建公有管道成功\n");
    int fd_pub;
    if ((fd_pub = open("pubpipe", O_RDONLY)) == -1) {
        perror("open public pipe failed");
        exit(0);
    }
    printf("打开公有管道成功\n");

    //循环处理客户端请求
    while (1) {
        bzero(rcv, sizeof(rcv));
        read(fd_pub, rcv, sizeof(rcv));
        char* src;
        pid_t srcpid;
        char* tmp;
        if (strlen(rcv) != 0) {
            src = strtok(rcv, ":");
            srcpid = atoi(src);
            tmp = strtok(NULL, ":");
        } else {
            continue;
        }

        //登录请求
        if(strncmp(tmp, "Login", 5) == 0) {

            //为客户端创建私有管道
            printf("<登录请求>\n");
            char* name = strtok(NULL, rcv);
            if ((ret = mkfifo(name, 0664)) == -1) {
                perror("mkfifo private pipe failed");
                exit(0);
            }
            pidToName[srcpid] = name;
            printf("创建私有管道成功\n");
            //只写方式打开私有管道
            int fd_pri;
            if ((fd_pri = open(name, O_WRONLY)) == -1) {
                perror("open private pipe failed");
                exit(0);
            }
            pidToFd[srcpid] = fd_pri;
            printf("打开私有管道[%s]成功\n", name);
            printf("\n");
            continue;
        }

        //有效用户请求
        if (strncmp(tmp, "all", 3) == 0) {

            printf("<有效用户请求>\n");
            //找到有效用户
            bzero(rcv, sizeof(rcv));
            strcat(rcv, "有效用户:\0");
            for (auto ite : pidToName) {
                if (ite.first != srcpid) {
                    strcat(rcv, ite.second.c_str());
                    strcat(rcv, " ");
                }
            }
            //根据客户端PID找到其私有管道文件描述符，并反馈给它
            int fd_pri;
            auto ite = pidToFd.find(srcpid);
            if (ite != pidToFd.end()) {
                fd_pri = ite->second;
            }
            write(fd_pri, rcv, sizeof(rcv));
            printf("反馈有效用户成功\n");
            printf("\n");
            continue;
        }

        //退出请求
        if (strncmp(tmp, "quit", 4) == 0) {
            printf("<退出请求>\n");
            //关闭对应私有管道
            map<pid_t, string>::iterator ite;
            ite = pidToName.find(srcpid);
            if (ite != pidToName.end()) {
                unlink(ite->second.c_str());
                ite = pidToName.erase(ite);
                printf("删除私有管道成功\n");
            }
            printf("\n");
            continue;
        }

        //通信请求
        printf("<通信请求>\n");
        char* data = strtok(NULL, ":");
        string dstName = tmp;
        int dstpid;
        string srcName;
        //根据发送端PID找到其名字
        auto it = pidToName.find(srcpid);
        if (it != pidToName.end()) {
            srcName = it->second;
        }
        bzero(snd, sizeof(snd));
        sprintf(snd, "%s:%s", srcName.c_str(), data);
        //根据接收端名字找到其PID
        for (auto ite = pidToName.begin(); ite != pidToName.end(); ite++) {
            if (ite->second == dstName) {
                dstpid = ite->first;
            }
        }
        //根据接收端PID找到其私有管道文件描述符，并转发消息
        int fd;
        auto ite = pidToFd.find(dstpid);
        if (ite != pidToFd.end()) {
            fd = ite->second;
        }
        write(fd, snd, sizeof(snd));
        printf("转发消息[%s]\n", snd);
        printf("\n");
    }

    return 0;
}