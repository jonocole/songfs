#include <expat.h>
#include <curl/curl.h>
#include <string.h>

size_t onData( void *ptr, size_t size, size_t nmemb, void *stream );

struct ArtistList {
    char* data[50];
    int index;
};

size_t onData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    XML_Parse( (XML_Parser)stream, (const char*)ptr, size * nmemb, 0 );
    return size * nmemb;
}

static bool aname = false;
void startElement(void *userData, const XML_Char *name, const XML_Char **atts) 
{
    if( strcmp( name, "name" ) == 0 )
    {
        aname = true;
        ArtistList* artistList = (ArtistList*)userData;
        artistList->data[artistList->index] = NULL;
    }
}


void endElement( void *userData, const XML_Char *name)
{
    if( strcmp( name, "name" ) == 0 )
    {
        aname = false;
        ((ArtistList*)userData)->index++;
    }
}

void charHandler(void *userData, const XML_Char *s, int len)
{
    if( !aname )
        return;

    ArtistList* artistlist = (ArtistList*)userData;
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


XML_Parser initExpat()
{
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetElementHandler( parser, startElement, endElement );
    XML_SetCharacterDataHandler( parser, charHandler );
   
    return parser;
}

extern "C"
ArtistList* request( const char* url )
{
    XML_Parser parser = initExpat();
    
    ArtistList* artistlist = (ArtistList*)malloc( sizeof( ArtistList ));
    artistlist->index = 0;
 
    XML_SetUserData( parser, artistlist );

    CURL* curl = curl_easy_init();
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
    curl_easy_setopt( curl, CURLOPT_URL, url );

    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, onData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, parser );
    curl_easy_perform( curl );

    XML_Parse( parser, "", 0, 1 );
    return (ArtistList*)XML_GetUserData( parser );
}
//int main( int argc, char* argv[] )
//{
//    ArtistList* list = request( "http://ws.audioscrobbler.com/2.0/?method=user.gettopartists&user=jonocole&api_key=b25b959554ed76058ac220b7b2e0a026" );
//
//    for( int i = 0; i < 50 && list->data[i]; ++i )
//        printf( "%s\n", list->data[i] );
//
//    return 0;
//}
