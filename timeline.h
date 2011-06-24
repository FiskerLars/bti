

/** Status Message with adjoined information, based on twitter API
 */
struct status {
      char *text;  // status text
      char *user;  // screen name
      char *created; // creation time
      char *id; // user id
      char *location; // Descriptive Location
      char *geoloc; //geographic coordinates
};

struct status* new_status(const char * const text, 
			  const char * const user,
			  const char * const created,
			  const char * const id,
			  const char * const location,
			  const char * const geoloc);
int stat_fprint(FILE* f, struct status* stat);

struct timeline { 
      struct status** statuses;
      int statcnt;
};


struct timeline* new_timeline();
void tl_append_status(struct timeline* tl, struct status* stat);
int tl_fprint(FILE* f, struct timeline* tl);

void gpx_export_user_timeline(struct timeline* timeline, FILE *f, int append);
