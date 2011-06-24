


/** GPX waypoint, basic type for routes and tracks and... waypoints.
<time> xsd:dateTime </time> [0..1] ?
<magvar> degreesType </magvar> [0..1] ?
<geoidheight> xsd:decimal </geoidheight> [0..1] ?
<name> xsd:string </name> [0..1] ?
<cmt> xsd:string </cmt> [0..1] ?
<desc> xsd:string </desc> [0..1] ?
<src> xsd:string </src> [0..1] ?
<link> linkType </link> [0..*] ?
<sym> xsd:string </sym> [0..1] ?
<type> xsd:string </type> [0..1] ?
<fix> fixType </fix> [0..1] ?
<sat> xsd:nonNegativeInteger </sat> [0..1] ?
<hdop> xsd:decimal </hdop> [0..1] ?
<vdop> xsd:decimal </vdop> [0..1] ?
<pdop> xsd:decimal </pdop> [0..1] ?
<ageofdgpsdata> xsd:decimal </ageofdgpsdata> [0..1] ?
<dgpsid> dgpsStationType </dgpsid> [0..1] ?
<extensions> extensionsType </extensions> [0..1] ?
</...>

http://www.topografix.com/GPX/1/1/gpx.xsd
*/
struct gpx_wpt {
      // wptType required fields
      // lat="latitudeType [1] ?"
      // lon="longitudeType [1] ?">
      char *lat;
      char *lon;

      // wptType optional fields:
      // <ele> xsd:decimal </ele> [0..1] ?
      char *ele;
      char *time;
      char *magvar;
      char *geoidheight;
      char *name;
      char *cmt;
      char *desc;
      char *src;
      char **link;
      char *sym;
      char *type;
      char *fix;
      char *sat;
      char *hdop;
      char *vdop;
      char *pdop;
      char *ageofdgpsdata;
      char *dgpsid;
      char *extensions;
};

// TODO
struct gpx_trkseg {};

struct gpx_track {
      // <name> xsd:string </name> [0..1] ?
      char* name;
    // <cmt> xsd:string </cmt> [0..1] ?
      char* cmt;
      // <desc> xsd:string </desc> [0..1] ?
      char* desc;
      // <src> xsd:string </src> [0..1] ?
      char* src;
      // <link> linkType </link> [0..*] ?
      char **link;
      // <number> xsd:nonNegativeInteger </number> [0..1] ?
      char *number;
      // <type> xsd:string </type> [0..1] ?
      char *type;
      // <extensions> extensionsType </extensions> [0..1] ?
      // <trkseg> trksegType </trkseg> [0..*] ? 
      struct gpx_trkseg* trkseg; // NULL-terminated
};

/** 
    
 */
void gpx_write_header( FILE *f );
FILE* gpx_end_trkseg(FILE* f);
FILE* gpx_start_trkseg(FILE* f, struct gpx_trkseg* tseg);
FILE* gpx_end_track(FILE* f);
FILE* gpx_start_track(FILE* f, struct gpx_track* trk);
FILE* gpx_append_trackpt(FILE* f, struct gpx_wpt* wpt);
