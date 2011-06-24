#include <stdio.h>

#include "gpx.h"


/*****************************************
 * GPX BACKEND FUNCTIONS
 * (following is the part to copy and paste for other projects)
 *****************************************/

FILE* gpx_append_links(FILE* f, char** links){
      fprintf(stderr, __func__);
}


/** 
    
 */
FILE* gpx_append_trackpt(FILE* f, struct gpx_wpt* wpt) {
      if(!wpt || !(wpt->lat && wpt->lon))
	    return 0;
      fprintf ( f, "  <trkpt lat=\"%s\" lon=\"%s\">\n", wpt->lat, wpt->lon );
      if(wpt->ele)  
	    fprintf ( f, "    <ele>%s</ele>\n", wpt->ele );
      if(wpt->time)  
	    fprintf ( f, "    <time>%s</time>\n", wpt->time);
      if(wpt->magvar)  
	    fprintf ( f, "    <magvar>%s</magvar>\n", wpt->magvar );
      if(wpt->geoidheight)  
	    fprintf ( f, "    <geoidheight>%s</geoidheight>\n", wpt->geoidheight);
      if(wpt->name)  
	    fprintf ( f, "    <name>%s</name>\n", wpt->name);
      if(wpt->cmt)  
	    fprintf ( f, "    <cmt>%s</cmt>\n", wpt->cmt);
      if(wpt->desc)  
	    fprintf ( f, "    <desc>%s</desc>\n", wpt->desc);
      if(wpt->src)  
	    fprintf ( f, "    <src>%s</src>\n", wpt->src);
      if(wpt->link) {  
	    char* l;
	    for(l = *wpt->link; l; l++)
	    fprintf ( f, "    <link>%s</link>\n", l);
      }
      if(wpt->sym)  
	    fprintf ( f, "    <sym>%s</sym>\n", wpt->sym);
      if(wpt->type)  
	    fprintf ( f, "    <type>%s</type>\n", wpt->type);
      if(wpt->fix)  
	    fprintf ( f, "    <fix>%s</fix>\n", wpt->fix);
      if(wpt->sat)  
	    fprintf ( f, "    <sat>%s</sat>\n", wpt->sat);
      if(wpt->hdop)  
	    fprintf ( f, "    <hdop>%s</hdop>\n", wpt->hdop);
      if(wpt->vdop)  
	    fprintf ( f, "    <vdop>%s</vdop>\n", wpt->vdop);
      if(wpt->pdop)  
	    fprintf ( f, "    <pdop>%s</pdop>\n", wpt->pdop);
      if(wpt->ageofdgpsdata)  
	    fprintf ( f, "    <ageofdgpsdata>%s</ageofdgpsdata>\n", wpt->ageofdgpsdata);
      if(wpt->dgpsid)  
	    fprintf ( f, "    <dgpsid>%s</dgpsid>\n", wpt->dgpsid);
      fprintf ( f, "  </trkpt>\n" );
      return f;
}


FILE* gpx_end_trkseg(FILE* f) {
      fprintf( f, "  </trkseg>\n");
}

FILE* gpx_start_trkseg(FILE* f, struct gpx_trkseg* tseg) {
      fprintf( f, "  <trkseg>\n");
      if(tseg) {      
	    fprintf(stderr, __func__);
      }
}


FILE* gpx_append_trksegs(FILE* f, struct gpx_trkseg* trksegs){
      //      gpx_start_trkseg(f);
      fprintf(stderr, __func__);
}


FILE* gpx_end_track(FILE* f) {
      fprintf( f, "  </trk>\n");
}


void gpx_write_footer( FILE *f )
{
  fprintf(f, "</gpx>\n");
}

void gpx_write_header( FILE *f )
{
  fprintf(f, "<?xml version=\"1.0\"?>\n"
          "<gpx version=\"1.0\" creator=\"Viking -- http://viking.sf.net/\"\n"
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
          "xmlns=\"http://www.topografix.com/GPX/1/0\"\n"
          "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
}


FILE* gpx_start_track(FILE* f, struct gpx_track* trk) {
      fprintf( f, "  <trk>\n");
      if(trk) {
	    // <name> xsd:string </name> [0..1] ?
	    if(trk->name)  
		  fprintf ( f, "   <name>%s</name>\n", trk->name);
	    // <cmt> xsd:string </cmt> [0..1] ?
	    if(trk->cmt)  
		  fprintf ( f, "   <cmt>%s</cmt>\n", trk->cmt);
	    // <desc> xsd:string </desc> [0..1] ?
	    if(trk->desc)  
		  fprintf ( f, "   <desc>%s</desc>\n", trk->desc);
	    // <src> xsd:string </src> [0..1] ?
	    if(trk->src)  
		  fprintf ( f, "   <src>%s</src>\n", trk->src);
	    // <link> linkType </link> [0..*] ?
	    if(trk->link) 
		  gpx_append_links(f, trk->link);
	    // <number> xsd:nonNegativeInteger </number> [0..1] ?
	    if(trk->number)  
		  fprintf ( f, "   <number>%s</number>\n", trk->number);
	    // <type> xsd:string </type> [0..1] ?
	    if(trk->type)  
		  fprintf ( f, "    <type>%s</type>\n", trk->type);
	    // <extensions> extensionsType </extensions> [0..1] ?
	    // <trkseg> trksegType </trkseg> [0..*] ? 
	    if(trk->trkseg)
		  gpx_append_trksegs(f, trk->trkseg);
      }
}









