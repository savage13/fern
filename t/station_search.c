
#include <fern.h>

int
multiple_centers() {
    // Construct a request
    // Time:     1994/160 - 1994/161
    // Network:  XE
    // Channel:  BH?
    // Region:   -180/180/-90/0
    request *r = station_req_new();
    station_req_set_network(r, "9A");
    request_set_verbose(r, 1);
    // Get request and check result
    result *res = request_get(r);
    if(!result_is_ok(res)) {
        return -1;
    }
    // Search PH5
    request_set_url(r, "http://service.iris.edu/ph5ws/station/1/query?");
    result *res2 = request_get(r);
    if(!result_is_ok(res2)) {
        return -1;
    }

    xml *x = xml_merge_results(res, res2, "//s:Network");
    if(!x) {
        return -1;
    }
    // Parse station xml output
    int verbose   = 1;
    int epochs    = 1;
    int show_time = 1;
    station **st = station_xml_parse(x, epochs, verbose);
    if(st == NULL) {
        return -1;
    }

    // Print out the stations
    stations_write(st, show_time, stdout);
    return 0;
}

int
main() {
    // Construct a request
    // Time:     1994/160 - 1994/161
    // Network:  XE
    // Channel:  BH?
    // Region:   -180/180/-90/0
    request *r = station_req_new();
    station_req_set_time_range(r,
                               timespec64_from_yjhmsf(1994, 160, 0, 0, 0, 0),
                               timespec64_from_yjhmsf(1994, 161, 0, 0, 0, 0));
    station_req_set_network(r, "XE");
    station_req_set_channel(r, "BH?");
    station_req_set_region(r, -180.0, 180.0, -90.0, 0.0);
    request_set_verbose(r, 1);
    // Get request and check result
    result *res = request_get(r);
    if(!result_is_ok(res)) {
        return -1;
    }
    // Parse station xml output
    int verbose   = 1;
    int epochs    = 1;
    int show_time = 1;
    station **st = station_xml_parse_from_raw(result_data(res), result_len(res), epochs, verbose);
    if(st == NULL) {
        return -1;
    }
    // Print out the stations
    stations_write(st, show_time, stdout);

    // Request across multiple urls
    if(multiple_centers() != 0) {
        return -1;
    }
    return 0;
}
