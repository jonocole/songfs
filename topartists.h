#ifndef TOP_ARTISTS_H_
#define TOP_ARTISTS_H_

struct ArtistList {
    char* data[50];
    int index;
};

#ifdef __cplusplus
    extern "C"
#endif
struct ArtistList* request( const char* url );

#endif
