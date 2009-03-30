#include <expat.h>
#include <curl/curl.h>
#include <string.h>

size_t onData( void *ptr, size_t size, size_t nmemb, void *stream );


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
    }
}


void endElement( void *userData, const XML_Char *name)
{
    if( strcmp( name, "name" ) == 0 )
    {
        aname = false;
    }
}

void charHandler(void *userData, const XML_Char *s, int len)
{
    if( aname == true )
        printf( "%.*s\n" , len, s );
}


XML_Parser initExpat()
{
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetElementHandler( parser, startElement, endElement );
    XML_SetCharacterDataHandler( parser, charHandler );
    return parser;
}


void request( const char* url )
{
    XML_Parser parser = initExpat();

    CURL* curl = curl_easy_init();
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 );
    curl_easy_setopt( curl, CURLOPT_URL, url );

    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, onData );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, parser );
    curl_easy_perform( curl );

    XML_Parse( parser, "", 0, 1 );
}

int main( int argc, char* argv[] )
{
    request( "http://ws.audioscrobbler.com/2.0/?method=user.gettopartists&user=jonocole&api_key=b25b959554ed76058ac220b7b2e0a026" );

    return 0;
}
