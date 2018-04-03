#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const std::string RUNNING_TIME_NAME_ = "running_time.csv";
std::chrono::milliseconds running_duration_;
std::string start_DateTime_;
std::chrono::system_clock::time_point start_time_point_;

void RecordStartTime(void) {
    start_time_point_ = std::chrono::system_clock::now();
    start_DateTime_ = "Start";

    running_duration_.zero();
    std::string file_path = RUNNING_TIME_NAME_;
    std::ifstream ifs;
    ifs.open(file_path, std::ios::in);
    if (ifs.is_open()) {
        ifs.seekg(-1, std::ios_base::end);
        bool found_line = false;
        while (!found_line) {
            bool found_lf = false;
            std::string buf;
            while (!found_lf) {
                char ch;
                ifs.get(ch);
                if (static_cast<int>(ifs.tellg()) <= 1) {
                    buf = ch + buf;
                    ifs.seekg(0);
                    found_line = true;
                    found_lf = true;
                } else if (ch == '\n') {
                    ifs.seekg(-2, std::ios_base::cur);
                    found_lf = true;
                } else {
                    ifs.seekg(-2, std::ios_base::cur);
                    buf = ch + buf;
                }
            }

            if (!buf.empty()) {
                std::vector<std::string> lst;
                boost::split(lst, buf, boost::is_any_of(","));
                if (lst.size() == 3) {
                    std::string dur = lst[2];
                    std::vector<std::string> timlst;
                    boost::split(timlst, dur, boost::is_any_of(":."));
                    if (timlst.size() == 4) {
                        std::chrono::hours h =
                            std::chrono::hours(std::stoul(timlst[0]));
                        std::chrono::minutes m =
                            std::chrono::minutes(std::stoul(timlst[1]));
                        std::chrono::seconds s =
                            std::chrono::seconds(std::stoul(timlst[2]));
                        std::chrono::milliseconds i =
                            std::chrono::milliseconds(std::stoul(timlst[3]));
                        running_duration_ = h + m + s + i;
                        found_line = true;
                    }
                }
            }
        }
        ifs.close();
    }
}

void RecordEndTime(void) {
    // 終了時刻取得
    auto end_time_point = std::chrono::system_clock::now();
    auto end_DateTime = "End";

    // 経過時間の算出
    auto running_thistime = end_time_point - start_time_point_;
    running_duration_ +=
        std::chrono::duration_cast<std::chrono::milliseconds>(running_thistime);

    auto h = std::chrono::duration_cast<std::chrono::hours>(running_duration_);
    running_duration_ -= h;
    auto m =
        std::chrono::duration_cast<std::chrono::minutes>(running_duration_);
    running_duration_ -= m;
    auto s =
        std::chrono::duration_cast<std::chrono::seconds>(running_duration_);
    running_duration_ -= s;
    auto i = std::chrono::duration_cast<std::chrono::milliseconds>(
        running_duration_);

    std::stringstream dur;
    dur << h.count() << ":" << std::setw(2) << std::setfill('0') << m.count()
        << ":" << std::setw(2) << std::setfill('0') << s.count() << "."
        << std::setw(3) << std::setfill('0') << i.count();
    std::string file_path = RUNNING_TIME_NAME_;
    std::ofstream ofs;
    ofs.open(file_path, std::ios::out | std::ios::app);
    ofs << start_DateTime_ << "," << end_DateTime << "," << dur.str()
        << std::endl;
    ofs.close();
}

int main() {
    RecordStartTime();
    sleep(5);
    RecordEndTime();
    return 0;
}
