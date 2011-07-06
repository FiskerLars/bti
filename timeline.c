#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <errno.h>

#include "bti.h"
#include "timeline.h"
#include "gpx.h"


extern int debug;
#define dbg(format, arg...)						\
	do {								\
		if (debug)						\
			fprintf(stdout, "bti: %s: " format , __func__ , \
				## arg);				\
	} while (0)


#define TL_BUFSIZE 256


static void tobedone(const char *f) {
      fprintf(stderr, "To Bo done %s\n", f);
      exit(EXIT_FAILURE);
}

struct status* new_status(const char * const text, 
			  const char * const user,
			  const char * const created,
			  const char * const id,
			  const char * const location,
			  const char * const geoloc) {
      struct status* status = (struct status*)malloc(sizeof(struct status));
      if (status) {
	    status->text = 0;
	    if (text)
		  status->text = strdup(text);
	    status->user = 0;
	    if (user)
		  status->user = strdup(user);
	    status->created = 0;
	    if (created)
		  status->created = strdup(created);
	    status->id = 0;
	    if (id)
		  status->id = strdup(id);
	    status->location = 0;
	    if (location)
		  status->location = strdup(location);
	    status->geoloc = 0;
	    if (geoloc)
		  status->geoloc = strdup(geoloc);
      }
      return status;
} 


/* created comes in .... format
 */
static int stat_timecmp(char* a, char* b) {
      tobedone(__func__);
}

void stat_free(struct status* stat) {
      if (stat) {
	    if (stat->text) free(stat->text);
	    if (stat->user) free(stat->user);
	    if (stat->created) free(stat->created);
	    if (stat->id) free(stat->id);
	    if (stat->location) free(stat->location);
	    if (stat->geoloc) free(stat->geoloc);
	    free(stat);
      }
}

static struct status* statdup(struct status* a) {
      struct status* stat = (struct status*)malloc(sizeof(struct status));
      int fail = 1;
      if (stat) {
	    fail = fail &&  (stat->text = strdup(a->text));  // status text
	    fail = fail &&  (stat->user = strdup(a->user));  // status text
	    fail = fail &&  (stat->created = strdup(a->created));  // status text
	    fail = fail &&  (stat->id = strdup(a->id));  // status text
	    fail = fail &&  (stat->location = strdup(a->location));  // status text
	    fail = fail &&  (stat->geoloc = strdup(a->geoloc));  // status text
	    if(fail) {
		  stat_free(stat);
		  stat = 0;
	    }
      }
      return stat;
}

static int statcmp(struct status* a, struct status* b) {
      tobedone(__func__);
}


/** todo: transfrm to GPX xsd:date-ime
 [-]CCYY-MM-DDThh:mm:ss[Z|(+|-)hh:mm]
The time zone may be specified as Z (UTC) or (+|-)hh:mm. Time zones that aren't specified are considered undetermined.

 */
static char* status_creat2gpx(struct status* stat) {
      struct tm time; 
      if (stat && stat->created) {
	    int res = getdate_r(stat->created, &time);
	    if (0 != res) {
		  size_t len;
		  char* str = (char*)malloc(80*sizeof(char));
		  len = strftime(str, 80, "%Y-%m-%dT%H:%M:%SZ", &time);
		  if (len > 0 && len < 80) {
			str = realloc(str, (len+1)*sizeof(char));
		  } else {
			free(str);
			str = 0;
		  }
		  return str;
	    } 
      }
      return 0;
}


#define COORDSTART "-0123456789"
#define COORDCHAR "-0123456789"
static char** status_geoloc2latlon(struct status* stat) {
      char* lat = 0;
      char* lon = 0;
      char** latlon = malloc(2*sizeof(char*));
      if(stat && stat->geoloc) {
	    char* s = stat->geoloc;
	    while(*s && (isspace(*s) || (0 == strchr(COORDSTART, *s)))) 
		  s++;
	    lat = s;
	    while(*s && (!isspace(*s) || (0 != strchr(COORDCHAR, *s)))) 
		  s++;
	    latlon[0] = strndup(lat, s-lat);
	    while(*s && (isspace(*s) || (0 == strchr(COORDSTART, *s)))) 
		  s++;
	    lon = s;
	    while(*s && (!isspace(*s) || (0 != strchr(COORDCHAR, *s)))) 
		  s++;
	    latlon[1] = strndup(lon, s-lon);
      }
      return latlon;
}



int stat_fprint(FILE* f, struct status* stat) {
      int n = 0;
      if (f != 0 && stat != 0) {
	    n += fprintf(f, "-----\n");
	    if (0 != stat->user) n += fprintf(f, "name: %s\n", stat->user);
	    if (stat->id) n += fprintf(f, "id: %s\n", stat->id);
	    if (stat->text) n += fprintf(f, "text: %s\n", stat->text);
	    if (stat->created) n += fprintf(f, "time: %s\n", stat->created);
	    if (stat->location) n += fprintf(f, "loca: %s\n", stat->location);
	    if (stat->geoloc) n += fprintf(f, "geol: %s\n", stat->geoloc);
      }
      return n;
}

struct timeline* new_timeline(){
      struct timeline* tl = (struct timeline*)malloc(sizeof(struct timeline));
      if (tl) {
	    tl->statuses = 0;
	    tl->statcnt = 0;
	    return tl;
      }
      return 0;
}

void free_timeline(struct timeline* tl) {
      if(tl) {
	    if(tl->statuses){
		  struct status** s = tl->statuses;
		  while(*s) 
			stat_free(*s);
		  free(tl->statuses);
	    }
	    free(tl);
      }
}

int tl_fprint(FILE* f, struct timeline* tl) {
      int n = 0;
      if(f && tl) {
	    int i = 0;
	    struct status** s = tl->statuses;
	    while(i < tl->statcnt && s) {
		  n += stat_fprint(f, *s);
		  i++;
		  s++;
	    }
      }
      return n;
}

void tl_append_status(struct timeline* tl, struct status* stat){
      tl->statuses = realloc(tl->statuses, (tl->statcnt + 2) * sizeof(struct status));
      if(tl->statuses){
	    tl->statuses[tl->statcnt] = stat;
	    tl->statcnt += 1;
	    tl->statuses[tl->statcnt] = 0;	    
      }
}

/*****************************************
 * status BASED FUNCTIONS
 *****************************************/

/* not thread safe */
static FILE* append_status_gpx(FILE* f, struct status* stat) {
      struct gpx_wpt wpt = {0};
      if(stat && stat->geoloc) {
	    char **latlon = status_geoloc2latlon(stat);
	    if (latlon) {
		  wpt.desc = stat->text; 
		  wpt.name = stat->id; 
		  wpt.time = status_creat2gpx(stat); // TODO transform to UTC and GPX format
		  wpt.lat = latlon[0];
		  wpt.lon = latlon[1];
		  free(latlon);
		  wpt.src = "microblog"; // TODO put in name of blog/source 
		  // wpt.link TODO have status link
		  // status->id
		  
		  f = gpx_append_trackpt(f, &wpt);
		  free(wpt.time);
		  free(wpt.lat);
		  free(wpt.lon);
	    } else {
		  // ERROR unparsable geoloc
	    }
      } else {
	    // ERROR, no trkpt w/o geolocation
	    // todo: derive from location??
      }
      return f;
}
static FILE* gpx_append_user_track(FILE* f , char* uid) {
      struct gpx_track trk = {0};
      trk.name = uid;
      gpx_start_track(f, &trk);
}



/* Write all fa at the current pos of f.*/
static FILE* file_append(FILE* f, FILE* fa) {
      char buf[256];
      size_t rb = 0;
      fpos_t pos;
      fgetpos(fa, &pos);
      rewind(fa);
      while(0 < (rb = fread(buf, sizeof(char), 256, fa) ) )
	    fwrite(buf, sizeof(char), rb, f);
      fsetpos(fa, &pos);
      fflush(f);
      return f;
}





void gpx_export_user_timeline(struct timeline* timeline, FILE *f, int append){
      //for each user in timeline create a separate track
      // if append_mode, search for users in track-file, read user timeline and merge
      if ( !f ) {
	    fprintf(stderr, "__func__ : ERROR no file descriptor given to write to\n");
	    return;
      }
      if(append)
	    tobedone( __func__);
      if (timeline) {
	    int b_done[timeline->statcnt]; // status i used
	    int b_alldone = 0; // all statuses done?
	    int i;
	    for( i=0; i<timeline->statcnt; i++) b_done[i] = 0;

	    tl_fprint(stdout, timeline);
	    fprintf(stdout, "************************\n");

	    gpx_write_header(f);

	    while(!b_alldone) {
		  b_alldone = 1;
		  char* uid = 0;
		  FILE* f_track = 0; // temp stream for merging
		  char* b_track;
		  size_t size_track;
		  // through whole list
		  for(i = 0; i < timeline->statcnt; i++) {
			// unless already done or unidentifiable
			
			if(!b_done[i] && timeline->statuses[i]->user) {
			      if(!uid) {
			      // if we have not fixed a user yet
				    if(timeline->statuses[i]->geoloc) {
					  uid = timeline->statuses[i]->user;
					  f_track = open_memstream(&b_track, &size_track);
					  gpx_append_user_track(f_track, uid);
					  gpx_start_trkseg(f_track, 0); 
					  
				    } else {
					  // drop this entry
					  b_done[i];
					  continue; // to next entry
				    }
			      } 
			      if (!strcmp(uid, timeline->statuses[i]->user)) {
				    if(timeline->statuses[i]->geoloc) {
					  stat_fprint(stdout, timeline->statuses[i]);
					  append_status_gpx(f_track, timeline->statuses[i]);
				    }
				    b_done[i] = 1;
			      }	 else {
				    b_alldone = 0;
			      }
			}
		  }
		  // end track
		  if (f_track) {
			gpx_end_trkseg(f_track);
			gpx_end_track(f_track);
			// TODO copy/merge track
			file_append(f,f_track);
			fclose(f_track);
			f_track = 0;
		  }
	    }
      
	    gpx_write_footer(f);
      }
      
}


/************************************************************************/
/* Reading                                                              */

/* TODO
 * Stream reading and parsing to safe memory and time
 */

static void parse_statuses(struct timeline* tl,
			   xmlDocPtr doc, xmlNodePtr current)
{
      struct status *status;
      xmlChar *text = NULL;
      xmlChar *user = NULL;
      xmlChar *created = NULL;
      xmlChar *id = NULL;
      xmlChar *location = NULL;
      xmlChar *geoloc = NULL;
      xmlNodePtr userinfo;
      
    
      current = current->xmlChildrenNode;
      while (current != NULL) {
		if (current->type == XML_ELEMENT_NODE) {
			if (!xmlStrcmp(current->name, (const xmlChar *)"retweeted_status"))
			      return parse_statuses(tl, doc, current);

			if (!xmlStrcmp(current->name, (const xmlChar *)"created_at"))
				created = xmlNodeListGetString(doc, current->xmlChildrenNode, 1);
			if (!xmlStrcmp(current->name, (const xmlChar *)"text"))
				text = xmlNodeListGetString(doc, current->xmlChildrenNode, 1);
			if (!xmlStrcmp(current->name, (const xmlChar *)"id"))
				id = xmlNodeListGetString(doc, current->xmlChildrenNode, 1);
			if (!xmlStrcmp(current->name, (const xmlChar *)"user")) {
				userinfo = current->xmlChildrenNode;
				while (userinfo != NULL) {
					if ((!xmlStrcmp(userinfo->name, (const xmlChar *)"screen_name"))) {
						if (user)
							xmlFree(user);
						user = xmlNodeListGetString(doc, userinfo->xmlChildrenNode, 1);
					} else if ((!xmlStrcmp(userinfo->name, (const xmlChar *)"location"))) {
					      if (location)
						    xmlFree(location);
					      location = xmlNodeListGetString(doc, userinfo->xmlChildrenNode, 1);
					} 
					userinfo = userinfo->next;
				}
			}
			if (!xmlStrcmp(current->name, (const xmlChar *)"geo"))
			      {
				    xmlNodePtr geop = current->xmlChildrenNode;
				    while (geop != NULL) {
					  if(!xmlStrcmp(geop->name, (const xmlChar *)"point")) {
						if(geoloc)
						      xmlFree(geoloc);
						geoloc = xmlNodeListGetString(doc, geop->xmlChildrenNode, 1);
					  }		
					  geop = geop->next;
				    }
			      }
			if (!xmlStrcmp(current->name, (const xmlChar *)"coordinates"))
			      {				    
				    dbg("coordinates: %s\n---coordinates---\n", xmlNodeListGetString(doc, current->xmlChildrenNode, 1));
			      }

		}
		current = current->next;
      } 
      // status parse finished
      // store status in timeline
      if(tl) {
	    struct status* status = new_status(text, user, created,
					       id, location, geoloc);
	    if(status) tl_append_status(tl, status);
      }
      // clean up
      if (user) xmlFree(user);
      if (text) xmlFree(text);
      if (created) xmlFree(created);
      if (id) xmlFree(id);
      if (location) xmlFree(location);
      if (geoloc) xmlFree(geoloc);

      return;
}



struct timeline* timeline_parse(char *document)
{
      xmlDocPtr doc;
	xmlNodePtr current;

	struct timeline* tl = new_timeline();

	doc = xmlReadMemory(document, strlen(document), "timeline.xml",
			    NULL, XML_PARSE_NOERROR);
	if (doc == NULL)
		return;

	current = xmlDocGetRootElement(doc);
	if (current == NULL) {
		fprintf(stderr, "empty document\n");
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(current->name, (const xmlChar *) "statuses")) {
	      fprintf(stderr, "unexpected document type, expected <statuses>, found %s\n", current->name);
		xmlFreeDoc(doc);
		return;
	}

	current = current->xmlChildrenNode;
	while (current != NULL) {
		if ((!xmlStrcmp(current->name, (const xmlChar *)"status")))
		      parse_statuses(tl, doc, current);
		current = current->next;
	}
	//	tl_fprint(stdout, session->tl);
	xmlFreeDoc(doc);

	return tl;
}


struct timeline*  timeline_fread(FILE* f) {
      struct timeline* tl = 0;
      char *buf = malloc((1+TL_BUFSIZE) * sizeof(char));
      char *p = buf;
      int bufsize = TL_BUFSIZE+1;
      int read = 0;
      int rb = 0; 
      
      while(TL_BUFSIZE <= (rb = fread(p, sizeof(char), TL_BUFSIZE, f) ) ) {
	    char *b = realloc(buf, sizeof(char) * (bufsize += TL_BUFSIZE)); 
	    p += rb;
	    if(!b) {
		  bufsize -= TL_BUFSIZE;
		  break;
	    }	  
	    p = b + (p - buf);
	    buf = b;
      }
      *p = '\0';
      buf = realloc(buf, sizeof(char) * (p-buf+1));
      tl = timeline_parse(buf);	          
      free(buf);
      return tl;
}

/** Writing a timeline in Twitter API XML, similar to parsed timeline above...
 */
int timeline_fwrite(FILE* f, struct timeline* tl) {
      tobedone(__func__);
      return 0;
}


/** Merges two timelines into a new timeline. Duplicates all status fields. Assumes ordered timelines by status->created.
 */
static struct timeline* merge_timelines(struct timeline* a, struct timeline* b) {
      int ai=0, bi=0, mi=0;
      struct timeline* mab = malloc(sizeof(struct timeline));
      int mabmemcnt = 0;
      if(a && a->statcnt > 0) mabmemcnt += a->statcnt;
      if(b && b->statcnt > 0) mabmemcnt += b->statcnt;
      mab->statuses = (struct status**)malloc(mabmemcnt * sizeof(struct status*));
      mab->statcnt = 0;
      
      while(mi < mabmemcnt) {
	    int cmp = 0;
	    if (ai < a->statcnt && bi < b->statcnt)
		  cmp = stat_timecmp(a->statuses[ai]->created, b->statuses[bi]->created);
	    else if ( ai < a->statcnt)
		  cmp = 1;
	    else if ( bi < b->statcnt) 
		  cmp = -1;
	    else
		  break;

	    if ( cmp > 0 ) {
		  mab->statuses[mi] = statdup(a->statuses[ai]);
		  mab->statcnt++;
	    } else if ( cmp < 0 ) { 
		  mab->statuses[mi] = statdup(b->statuses[bi]);
		  mab->statcnt++;
	    } else {
		  if( ! statcmp(a->statuses[ai], b->statuses[bi])) {
			mab->statuses[mi] = statdup(a->statuses[ai]);
			mab->statcnt++;
		  } else {
			mab->statuses[mi] = statdup(a->statuses[ai]);
			mab->statuses[mi+1] = statdup(a->statuses[ai]);
			mab->statcnt += 2;
		  }
	    }
      }
      mab->statuses = (struct status**)realloc(mab->statuses, mab->statcnt * sizeof(struct status*));
      return mab;
}

/** Split timeline based on user-id. Provides a fresh duplicated array of duplicated timelines.
 */
struct timeline** timeline_split(struct timeline* tl) {
      if(!tl || !tl->statuses) return 0;
      struct status** s = tl->statuses;
      struct timeline** tla = malloc(sizeof(struct timeline*));
      int tlacnt = 0;
      int alldone = 0;
      int done[tl->statcnt];
      int i;
      *tla = 0;
      for( i=0; i<tl->statcnt; i++) done[i] = 0;
      
      while(!alldone){
	    alldone = 1;
	    struct timeline* ctl = 0;
	    for(i = 0; i < tl->statcnt; i++) {
		  if(!done[i] && tl->statuses[i]) {
			if(!ctl) { // no current user to examine, create new timeline
			      struct timeline** tmptla = realloc(tla, (tlacnt+2) * sizeof(struct timeline*));
			      if(!tmptla) { // out of mem
				    errno = EAGAIN;
				    perror("Insufficient memory in __func__");
				    exit(errno);
			      }
			      tla = tmptla;
			      ctl = tla[tlacnt] = new_timeline();
			      tlacnt += 1;
			      tla[tlacnt] = 0;
			}
			// append to current timeline if it fits
			if(ctl->statcnt <= 0 || !strcmp(ctl->statuses[0]->user,tl->statuses[i]->user)) {
			      tl_append_status(ctl, statdup(tl->statuses[i]));
			      done[i] = 1;
			} else {
			      done[i] = 0;
			      alldone = 0;
			}
		  }
		  
	    } 
      }
      return tla;

}
