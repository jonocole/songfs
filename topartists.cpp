#include <expat.h>
#include <curl/curl.h>
#include <string.h>
#include "topartists.h"

static size_t 
onData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    XML_Parse( (XML_Parser)stream, (const char*)ptr, size * nmemb, 0 );
    return size * nmemb;
}

static int aname = 0;
void startElement(void *userData, const XML_Char *name, const XML_Char **atts) 
{
    if( strcmp( name, "name" ) == 0 )
    {
        aname = 1;
        struct ArtistList* artistList = (struct ArtistList*)userData;
        artistList->data[artistList->index] = NULL;
    }
}


static void 
endElement( void *userData, const XML_Char *name)
{
    if( strcmp( name, "name" ) == 0 )
    {
        aname = 0;
        ((struct ArtistList*)userData)->index++;
    }
}

static void 
charHandler(void *userData, const XML_Char *s, int len)
{
    if( !aname )
        return;

    struct ArtistList* artistlist = (struct ArtistList*)userData;
    char ** const data = artistlist->data;
    const int index = artistlist->index;
    
    if( !data[index] )
    {
        data[index] = (char*)malloc( len + 1 );
        strncpy( data[index], s, len );
        data[index][len] = 0;
        return;
    }
    
    data[index] = (char*)realloc( data[index], strlen( data[index] ) + len + 1 );

    int oldlen = strlen( data[index] );
    strncpy( &data[index][ oldlen ],
             s, len );

    data[index][ oldlen + len  ] = 0; 
}


static XML_Parser 
initExpat()
{
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetElementHandler( parser, startElement, endElement );
    XML_SetCharacterDataHandler( parser, charHandler );
   
    return parser;
}

extern "C" 
{
struct ArtistList* 
request( const char* url )
{
    XML_Parser parser = initExpat();
    
    struct ArtistList* artistlist = (struct ArtistList*)malloc( sizeof( struct ArtistList ));
    artistlist->index = 0;
 
    XML_SetUserData( parser, artistlist );

    CURL* curl = curl_easy_init();
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
    curl_easy_setopt( curl, CURLOPT_URL, url );

    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, onData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, parser );
    curl_easy_perform( curl );

    XML_Parse( parser, "", 0, 1 );
    return (struct ArtistList*)XML_GetUserData( parser );
}
}

