#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

class PDevice {
   private:
    volatile bool running_;
    int fd_;
    std::thread th_;

   public:
    PDevice() : running_(false), fd_(-1) {}

    void Start(const char* fname) {
        std::cout << "[PDevice] Start" << std::endl;
        fd_ = open(fname, O_RDWR);
        if (fd_ < 0) {
            std::cout << "device open error" << std::endl;
            return;
        }
        running_ = true;
        th_ = std::thread(&PDevice::Mainloop, this);
    }

    int Mainloop(void) {
        std::cout << "[PDevice] starts receiving..." << std::endl;
        while (running_) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd_, &rfds);
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 50000;
            if (select(fd_ + 1, &rfds, NULL, NULL, &tv) == -1) {
                break;
            }
            if (FD_ISSET(fd_, &rfds)) {
                std::cout << "[PDevice] fd is set" << std::endl;
                char buf[256];
                if(!running_) break;
                size_t siz = read(fd_, buf, 256);
                std::cout << "[PDevice] read(" << siz << ")" << std::endl;
                if (!running_) break;
                if (siz <= 0) break;
                if (siz >= 256) siz = 255;
                buf[siz] = 0;

                char buf2[256];
                CreateReply(buf2, buf, siz, 256);
                if (!running_) break;
                std::cout << "[PDevice] read[" << buf << "]" << std::endl;
                std::cout << "[PDevice] write[" << buf2 << "]" << std::endl;
                if(!running_) break;
                siz = write(fd_, buf2, siz);
                std::cout << "[PDevice] write(" << siz << ")" << std::endl;
                if (siz <= 0) break;
                sleep(1);
            }
        }
        std::cout << "[PDevice] end." << std::endl;
        return 0;
    }

    void Stop(void) {
        if (running_) {
            std::cout << "[PDevice] stop" << std::endl;
            running_ = false;
            close(fd_);
            if (th_.joinable()) {
                th_.join();
                std::cout << "[PDevice] stopped" << std::endl;
            }
        }
    }

   private:
    size_t CreateReply(char *buf2, const char *buf, size_t siz, size_t bufsiz) {
        if (bufsiz < siz) siz = bufsiz;
        for (size_t i = 0; i < siz; i++) {
            if (buf[i] == 0) {
                buf2[i] = 0;
                break;
            } else if ((buf[i] >= 0x20) && (buf[i] <= 0xFD)) {
                buf2[i] = (char)(unsigned char)(1 + (int)buf[i]);
            } else if (buf[i] == 0xFE) {
                buf2[i] = 0x20;
            } else {
                buf2[i] = buf[i];
            }
       }
        if (siz > 0) buf2[siz] = 0;
        return siz;
    }
};

int main() {
    PDevice pd;
    pd.Start("/dev/r2l");
    sleep(1);

    int fd = open("/dev/r2l", O_RDWR);
    if (fd < 0) {
        std::cout << "[Main] device open error" << std::endl;
        return 1;
    }

    std::string snd("12345ABCDEabcde Hello World!");
    size_t siz = write(fd, snd.c_str(), snd.size());
    if (siz <= 0)  {
        std::cout << "[Main] device write error" << std::endl;
        return 1;
    }

    char buf[256];
    siz = read(fd, buf, 256);
    if (siz <= 0)  {
        std::cout << "[Main] device read error" << std::endl;
        return 1;
    }
    std::cout << "[Main] receives " << buf << std::endl;
    close(fd);
    sleep(1);
    pd.Stop();
    return 0;
}

