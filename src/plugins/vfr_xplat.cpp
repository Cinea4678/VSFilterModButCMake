// Cross-platform VFR (Variable Frame Rate) translator
// Based on src/vsfilter/vfr.cpp with Windows-specific code replaced

#include "../vsfilter/vfr.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <filesystem>

#ifdef _WIN32
#include <atlconv.h>
#include <atlpath.h>
#else
#include <strings.h> // strncasecmp
#define _strnicmp strncasecmp
#endif

class TimecodesV1 : public VFRTranslator {
    double default_spf;
    double first_non_section_timestamp;
    int first_non_section_frame;

    struct FrameRateSection {
        double start_time;
        double spf;
        int start_frame;
        int end_frame;
    };
    std::vector<FrameRateSection> sections;

public:
    double TimeStampFromFrameNumber(int n) override {
        for (size_t i = 0; i < sections.size(); i++) {
            auto& sect = sections[i];
            if (n >= sect.start_frame && n <= sect.end_frame)
                return sect.start_time + (n - sect.start_frame) * sect.spf;
        }
        if (n < 0) return 0.0;
        return first_non_section_timestamp + (n - first_non_section_frame) * default_spf;
    }

    TimecodesV1(FILE* f) {
        char buf[100];
        default_spf = -1;
        double cur_time = 0.0;

        FrameRateSection temp;
        temp.start_time = 0.0;
        temp.spf = -1;
        temp.start_frame = 0;
        temp.end_frame = 0;

        while (fgets(buf, 100, f)) {
            if (buf[0] == '#') continue;
            if (_strnicmp(buf, "Assume ", 7) == 0 && default_spf < 0) {
                default_spf = atof(buf + 7);
                if (default_spf > 0) default_spf = 1.0 / default_spf;
                else default_spf = -1;
                temp.spf = default_spf;
                continue;
            }

            int sf, ef; float fps;
            if (sscanf(buf, "%d,%d,%f", &sf, &ef, &fps) == 3) {
                temp.end_frame = sf - 1;
                if (temp.end_frame >= temp.start_frame) {
                    cur_time += (temp.end_frame - temp.start_frame + 1) * temp.spf;
                    sections.push_back(temp);
                }
                temp.spf = 1.0 / fps;
                temp.start_frame = sf;
                temp.end_frame = ef;
                temp.start_time = cur_time;
                cur_time += (ef - sf + 1) / (double)fps;
                sections.push_back(temp);
                temp.spf = default_spf;
                temp.start_frame = ef + 1;
                temp.end_frame = ef;
                temp.start_time = cur_time;
            }
        }
        first_non_section_timestamp = cur_time;
        first_non_section_frame = temp.start_frame;
    }
};

class TimecodesV2 : public VFRTranslator {
    std::vector<double> timestamps;
    int last_known_frame;
    double last_known_timestamp;
    double assumed_spf;
public:
    double TimeStampFromFrameNumber(int n) override {
        if (n >= 0 && n < (int)timestamps.size()) return timestamps[n];
        if (n < 0) return 0.0;
        return last_known_timestamp + (n - last_known_frame) * assumed_spf;
    }

    TimecodesV2(FILE* f) {
        char buf[50];
        timestamps.reserve(8192);
        while (fgets(buf, 50, f)) {
            if (buf[0] == '#') continue;
            timestamps.push_back(atof(buf) / 1000.0);
        }
        last_known_frame = (int)timestamps.size() - 1;
        last_known_timestamp = timestamps[last_known_frame];
        assumed_spf = last_known_timestamp - timestamps[last_known_frame - 1];
    }
};

VFRTranslator* GetVFRTranslator(const char* vfrfile) {
    if (!vfrfile || !*vfrfile) return nullptr;

    FILE* f = nullptr;

#ifdef _WIN32
    int size = MultiByteToWideChar(CP_UTF8, 0, vfrfile, -1, nullptr, 0);
    wchar_t* wfile = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, vfrfile, -1, wfile, size);
    if (!PathFileExistsW(wfile)) {
        delete[] wfile;
        size = MultiByteToWideChar(CP_ACP, 0, vfrfile, -1, nullptr, 0);
        wfile = new wchar_t[size];
        MultiByteToWideChar(CP_ACP, 0, vfrfile, -1, wfile, size);
    }
    f = _wfopen(wfile, L"r");
    delete[] wfile;
#else
    // On non-Windows, vfrfile is already UTF-8 path
    f = fopen(vfrfile, "r");
#endif

    if (!f) return nullptr;

    char buf[32];
    buf[19] = 0;
    VFRTranslator* res = nullptr;
    if (fgets(buf, 32, f) && buf[0] == '#') {
        if (buf[19] == '1') res = new TimecodesV1(f);
        else if (buf[19] == '2') res = new TimecodesV2(f);
    }
    fclose(f);
    return res;
}
