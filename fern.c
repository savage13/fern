
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <sacio/timespec.h>
#include <libmseed/libmseed.h>

#include <fern.h>

#include "defs.h"
#include "strip.h"
#include "slurp.h"

#define EVENT_URL   "https://earthquake.usgs.gov/fdsnws/event/1/query?"
#define STATION_URL "https://service.iris.edu/fdsnws/station/1/query?"
#define DATA_URL    "https://service.iris.edu/irisws/fedcatalog/1/query?"

Event **quake_xml_parse(char *data, size_t data_len, int verbose, char *cat);

typedef enum _action action;
#define ActionNone      0
#define ActionEvent     1<<0
#define ActionStation   1<<1
#define ActionAvailable 1<<2
#define ActionMiniseed  1<<3
#define ActionSac       1<<4
#define ActionRequest   (ActionAvailable | ActionMiniseed | ActionSac)


int
main(int argc, char *argv[]) {
    int ch = 0;
    int act = ActionNone;
    request *r = NULL;
    result *res = NULL;
    Event **ev = NULL;
    Event *e = NULL;
    duration dur = {0,0};
    duration d = {0,0};
    station **s = NULL;
    int verbose = 0;
    int epochs = FALSE;
    int show_times = FALSE;
    double v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;
    timespec64 t1, t2;
    data_request *fdr = NULL;
    char prefix[1024] = {0};
    char request_file[2048] = {0};
    char output[2048] = {0};
    char cat[16] = {0};
    size_t chunk_size = 200.0 * 1024.0 * 1024.0 ; // Request size in MB
    fern_strlcat(prefix, "fdsnws", sizeof(prefix));


    struct option longopts[] = {
        {"event-query",     no_argument, NULL, 'E'},
        {"station-query",   no_argument, NULL, 'S'},
        {"data-query",required_argument, NULL, 'D'},
        {"mag",       required_argument, NULL, 'm'},
        {"time",      required_argument, NULL, 't'},
        {"region",    required_argument, NULL, 'R'},
        {"radius",    required_argument, NULL, 'r'},
        {"depth",     required_argument, NULL, 'z'},
        {"network",   required_argument, NULL, 'n'},
        {"station",   required_argument, NULL, 's'},
        {"location",  required_argument, NULL, 'l'},
        {"channel",   required_argument, NULL, 'c'},
        {"epochs",    required_argument, NULL, 'y'},
        {"show-time", required_argument, NULL, 'w'},
        {"event",     required_argument, NULL, 'e'},
        {"duration",  required_argument, NULL, 'd'},
        {"max",       required_argument, NULL, 'M'},
        {"origin",    required_argument, NULL, 'O'},
        {"prefix",    required_argument, NULL, 'p'},
        {"input",     required_argument, NULL, 'i'},
        {"output",     required_argument, NULL, 'o'},
        {NULL, 0, NULL, 0},
    };
    r = request_new();

    while((ch = getopt_long(argc, argv, "ESD:m:t:R:r:z:vn:s:l:c:e:d:M:O:ywp:i:o:", longopts, NULL)) != -1) {
        switch(ch) {
        case 'v':
            request_set_verbose(r, 1);
            verbose = 1;
            break;
        case 'D':
            request_set_url(r, DATA_URL);
            request_set_arg(r, "format", arg_string_new("request"));
            request_set_arg(r, "nodata", arg_int_new(404));
            if(strcasecmp(optarg, "available") == 0) {
                act = ActionAvailable;
            } else if(strcasecmp(optarg, "miniseed") == 0) {
                act = ActionMiniseed;
            } else if(strcasecmp(optarg, "sac") == 0) {
                act = ActionSac;
            } else {
                printf("error: expected data-query request, miniseed, or sac, found %s\n", optarg);
                exit(-1);
            }
            break;
        case 'E':
            act = ActionEvent;
            request_set_url(r, EVENT_URL);
            request_set_arg(r, "nodata", arg_int_new(404));
            request_set_arg(r, "format", arg_string_new("xml"));
            fern_strlcpy(cat, "usgs", sizeof(cat));
            break;
        case 'S':
            act = ActionStation;
            request_set_url(r, STATION_URL);
            request_set_arg(r, "level", arg_string_new("station"));
            request_set_arg(r, "nodata", arg_int_new(404));
            request_set_arg(r, "format", arg_string_new("xml"));
            break;
        case 'e':
            if(!(e = event_from_id(optarg))) {
                printf("error: expected event id, got %s\n", optarg);
                exit(-1);
            }
            break;
        case 'r':
            sscanf(optarg, "%lf/%lf", &v1, &v2);
            request_set_arg(r, "minradius", arg_double_new(v1));
            request_set_arg(r, "maxradius", arg_double_new(v2));
            break;
        case 'R':
            sscanf(optarg, "%lf/%lf/%lf/%lf", &v1, &v2, &v3, &v4);
            request_set_arg(r, "minlon", arg_double_new(v1));
            request_set_arg(r, "maxlon", arg_double_new(v2));
            request_set_arg(r, "minlat", arg_double_new(v3));
            request_set_arg(r, "maxlat", arg_double_new(v4));
            break;
        case 'i':
            fern_strlcpy(request_file, optarg, sizeof(request_file));
            break;
        case 'o':
            fern_strlcpy(output, optarg, sizeof(output));
            break;
        case 'y':
            epochs = TRUE;
            break;
        case 'w':
            show_times = TRUE;
            break;
        case 'p':
            fern_strlcpy(prefix, optarg, sizeof(prefix));
            break;
        case 'M':
            chunk_size = atof(optarg) * 1024.0 * 1024.0;
            break;
        case 'n':
            request_set_arg(r, "net", arg_string_new(optarg));
            break;
        case 's':
            request_set_arg(r, "sta", arg_string_new(optarg));
            break;
        case 'l':
            request_set_arg(r, "loc", arg_string_new(optarg));
            break;
        case 'c':
            request_set_arg(r, "cha", arg_string_new(optarg));
            break;
        case 'm':
            if(sscanf(optarg, "%lf/%lf", &v1,&v2) != 2) {
                printf("error: expected magnitude min/max, found %s\n", optarg);
                exit(-1);
            }
            request_set_arg(r, "minmag", arg_double_new(v1));
            request_set_arg(r, "maxmag", arg_double_new(v2));
            break;
        case 't':
            if(!timespec64_parse(optarg, &t1)) {
                printf("error: expected time value, found %s\n", optarg);
                exit(-1);
            }
            if(optind < argc && !timespec64_parse(argv[optind], &t2)) {
                if(optind < argc && !duration_parse(argv[optind], &d)) {
                    printf("error: expected time or duration value, found %s\n", optarg);
                    exit(-1);
                }
                t2 = timespec64_add_duration(t1, &d);
            }
            optind++;
            request_set_arg(r, "start", arg_time_new(t1));
            request_set_arg(r, "end",   arg_time_new(t2));
            break;
        case 'O':
            if(sscanf(optarg, "%lf/%lf", &v1,&v2) != 2) {
                printf("error: expected lon/lat, found %s\n", optarg);
                exit(-1);
            }
            request_set_arg(r, "lon", arg_double_new(v1));
            request_set_arg(r, "lat", arg_double_new(v2));
            break;
        case 'd':
            if(!duration_parse(optarg, &dur)) {
                printf("error: expected duration found: %s\n", optarg);
            }
            break;
        case 'z':
            if(sscanf(optarg, "%lf/%lf", &v1,&v2) != 2) {
                printf("error: expected depth min/max, found %s\n", optarg);
                exit(-1);
            }
            request_set_arg(r, "mindepth", arg_double_new(v1));
            request_set_arg(r, "maxdepth", arg_double_new(v2));
            break;
        default:
            printf("error: Unknown option -%c\n", ch);
            exit(-1);
            break;
        }
    }
    argc -= optind;
    argv += optind;
    if(act == ActionNone) {
        printf("Error: Must specify a type of request\n");
        exit(-1);
    }
    if(act & ActionEvent && e) {
        duration d = {0,0};
        timespec64 t1 = {0,0}, t2 = {0,0};
        duration_parse("-1m", &d);
        t1 = timespec64_add_duration(event_time(e), &d);
        duration_parse("+1m", &d);
        t2 = timespec64_add_duration(event_time(e), &d);
        request_set_arg(r, "start", arg_time_new(t1));
        request_set_arg(r, "end", arg_time_new(t2));
    }
    if(act & ActionStation && e) {
        request_set_arg(r, "start", arg_time_new(event_time(e)));
        request_set_arg(r, "end", arg_time_new(event_time(e)));
        request_set_arg(r, "lon", arg_double_new(event_lon(e)));
        request_set_arg(r, "lat", arg_double_new(event_lat(e)));
    }
    if(act & ActionRequest && e) {
        request_set_arg(r, "start", arg_time_new(event_time(e)));
        request_set_arg(r, "end", arg_time_new(event_time(e)));
        request_set_arg(r, "lat", arg_double_new(event_lat(e)));
        request_set_arg(r, "lon", arg_double_new(event_lat(e)));
    }
    if(act & ActionRequest && dur.type != Duration_None) {
        data_avail_use_duration(r, &dur);
    }
    //
    // Request
    //
    if(strlen(request_file) == 0) {
        res = request_get(r);
        if(!result_is_ok(res)) {
            printf("%s\n", result_error_msg(res));
            exit(-1);
        }
    }

    //
    // Data Processing
    //
    if(act & ActionEvent) {
        if(!(ev = quake_xml_parse(result_data(res), result_len(res), verbose, cat))) {
            printf("Error parsing quake xml format\n");
            exit(-1);
        }
        events_write(ev, stdout);
    }
    if(act & ActionStation) {
        if(!(s = station_xml_parse(result_data(res), result_len(res), epochs, verbose))) {
            printf("error parsing station.xml data\n");
            exit(-1);
        }
        stations_write(s, show_times, stdout);
    }
    // Create Request using Data Request List
    if(act & ActionRequest) {
        if(strlen(request_file) == 0) {
            fdr = data_request_parse(result_data(res));
            data_request_chunks(fdr, (size_t) chunk_size);
            if(verbose) {
                data_request_write(fdr, stdout);
            }
        } else {
            size_t n = 0;
            char *data = NULL;
            if(!(data = slurp(request_file, &n))) {
                exit(-1);
            }
            fdr = data_request_parse(data);
            FREE(data);
        }
        if(strlen(output) > 0) {
            data_request_write_to_file(fdr, output);
        }
    }
    if(act & ActionMiniseed || act & ActionSac) {
        MS3TraceList *mst3k = NULL;
        char *filename = NULL;
        if(strlen(output) > 0) {
            filename = output;
        } else if(strlen(request_file) > 0) {
            filename = request_file;
        } else {
            filename = result_filename(res);
        }
        mst3k = data_request_download(fdr, filename, prefix,
                                           act == ActionMiniseed,
                                           act == ActionSac);
        if(act & ActionSac && mst3k) {
            int nerr = 0;
            char tmp[128] = {0};
            sac **out;
            out = miniseed_trace_list_to_sac(mst3k);
            sac_array_fill_meta_data(out, verbose);
            sac_array_fill_meta_data_from_event(out, e, verbose);
            for(size_t i = 0; i < xarray_length(out); i++) {
                update_distaz(out[i]);
                cprintf("green", "\tWriting data to %s [%s]\n",
                        out[i]->m->filename, data_size(sac_size(out[i]),tmp,sizeof(tmp)));
                sac_write(out[i], out[i]->m->filename, &nerr);
            }
        }
    }
    return 0;
}
