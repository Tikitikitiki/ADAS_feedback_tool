#include <bits/stdc++.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Very small CSV parser that handles quoted values
static std::vector<std::string> parse_csv_line(const std::string &line) {
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
    }
    out.push_back(cur);
    return out;
}

static std::string quote_csv(const std::string &s){
    std::string out = s;
    // escape double quotes
    size_t pos = 0;
    while ((pos = out.find('"', pos)) != std::string::npos) { out.replace(pos,1,"\"\""); pos += 2; }
    return std::string("\"") + out + std::string("\"");
}

std::string query_overpass(const std::string &lat, const std::string &lon, int radius_meters, const std::string &outMode = "tags", int timeout_s=25){
    std::string endpoint = "https://overpass-api.de/api/interpreter";
    // Query: look for ways with highway tag near point
    std::ostringstream q;
    q << "[out:json][timeout:" << timeout_s << "];";
    q << "way(around:" << radius_meters << "," << lat << "," << lon << ")[highway];";
    if(outMode == "geom") q << "out geom;"; else q << "out tags;";
    std::string payload = q.str();

    CURL *curl = curl_easy_init();
    if(!curl) return "";

    std::string readbuf;
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readbuf);
    // set a user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ADAS-Overpass-Client/1.0");
    // small timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) return "";
    return readbuf;
}

// https://wiki.openstreetmap.org/wiki/Key%3Ahighway
std::string find_road_type_for_point(const std::string &lat_s, const std::string &lon_s){
    const std::vector<int> radii = {20, 50};
    for(int radius: radii){
        std::string resp = query_overpass(lat_s, lon_s, radius, "tags");
        if(resp.empty()) continue; // network error -> try next radius
        try{
            auto j = json::parse(resp);
            if(j.contains("elements") && j["elements"].is_array()){
                for(auto &el: j["elements"]){
                    if(!el.contains("tags") || !el["tags"].contains("highway")) continue;
                    return el["tags"]["highway"].get<std::string>();
                }
            }
        }catch(...){
            // parse error -> try next radius
        }
    }
    return std::string("NA");
}

int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " input.csv [output.csv]\n";
        return 1;
    }
    std::string inpath = argv[1];
    std::string outpath;
    if(argc >=3) outpath = argv[2];
    else outpath = inpath + ".with_roads.csv";

    std::ifstream inf(inpath);
    if(!inf){ std::cerr << "Failed to open input: " << inpath << "\n"; return 2; }
    std::ofstream outf(outpath);
    if(!outf){ std::cerr << "Failed to open output: " << outpath << "\n"; return 3; }

    std::string header;
    if(!std::getline(inf, header)){ std::cerr << "Empty input" << std::endl; return 4; }
    auto cols = parse_csv_line(header);
    int latIdx = -1, lonIdx = -1;
    for(size_t i=0;i<cols.size();++i){
        std::string c = cols[i];
        std::transform(c.begin(), c.end(), c.begin(), [](unsigned char ch){ return std::tolower(ch); });
        if(c == "latitude" || c=="lat") latIdx = i;
        if(c == "longitude" || c=="lon" || c=="lng") lonIdx = i;
    }
    // write header with new column
    for(size_t i=0;i<cols.size();++i){
        if(i) outf << ",";
        outf << quote_csv(cols[i]);
    }
    outf << "," << quote_csv("RoadType") << "\n";

    if(latIdx < 0 || lonIdx < 0){
        std::cerr << "Warning: Latitude/Longitude columns not found. Appending empty RoadType.\n";
        std::string line;
        while(std::getline(inf, line)){
            outf << line << "," << quote_csv("") << "\n";
        }
        return 0;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string line;
    size_t row = 0;
    while(std::getline(inf, line)){
        ++row;
        auto fields = parse_csv_line(line);
        std::string roadType = "";
        if(latIdx < (int)fields.size() && lonIdx < (int)fields.size()){
            std::string lat = fields[latIdx];
            std::string lon = fields[lonIdx];
            // trim
            auto trim = [](std::string &s){
                size_t a = 0; while(a<s.size() && isspace((unsigned char)s[a])) ++a;
                size_t b = s.size(); while(b> a && isspace((unsigned char)s[b-1])) --b;
                if(a||b!=s.size()) s = s.substr(a,b-a);
            };
            trim(lat); trim(lon);
            if(!lat.empty() && !lon.empty()){
                roadType = find_road_type_for_point(lat, lon);
            }
        }

        // write fields back
        for(size_t i=0;i<fields.size();++i){
            if(i) outf << ',';
            outf << quote_csv(fields[i]);
        }
        outf << ',' << quote_csv(roadType) << '\n';
        // be kind to Overpass (default 200ms as requested)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    curl_global_cleanup();
    std::cout << "Wrote: " << outpath << std::endl;
    return 0;
}
