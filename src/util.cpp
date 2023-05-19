#include "util.h"

namespace trk
{

int64_t getDuration_ns(std::chrono::_V2::system_clock::time_point *start)
{
    auto finish = std::chrono::high_resolution_clock::now();

    int64 duration = std::chrono::duration_cast<std::chrono::nanoseconds>( finish - *start ).count();

    *start = finish;

    return duration;
}

/*
data: {
  "colorFilter": true,
  "colorFrom": "36,80,25",
  "colorTo": "149,160,255",
*/
bool setColorFromCSV(const std::string& csv, cv::Scalar& color)
{
    int h,s,v;
    int successfull_items_scaned = sscanf(csv.c_str(), "%d,%d,%d", &h, &s, &v );
    color[0] = h;
    color[1] = s;
    color[2] = v;
    return successfull_items_scaned == 3;
}

void set_color_from_values(int h, int s, int v, cv::Scalar& color)
{
    color[0] = h;
    color[1] = s;
    color[2] = v;
}

bool write_to_file(const std::string& filename, const std::string& content)
{
    bool rc = false;

    std::ofstream fp(filename, std::ios::out);
    if (fp.is_open())
    {
        fp << content;
        if ( !fp.fail() ) {
            rc = true;
        }
    }

    return rc;
}

bool load_file_to_string(const std::string& filename, std::string* content)
{
    bool rc = false;

    std::ifstream fp(filename, std::ios::in);

    if ( fp.is_open() ) {
        std::stringstream buffer;
        buffer << fp.rdbuf();
        if ( !fp.fail() ) {
            rc = true;
            content->assign(buffer.str());
        }
    }

    return rc;
}

} // namespace