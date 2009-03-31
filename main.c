#define FUSE_USE_VERSION  27

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "topartists.h"

static const char* username = "mxcl";

static int getAttr( char* name, const char* path, struct stat *stbuf )
{
    char testpath[ strlen(name) + 3 ];
    strcpy( testpath, "/" );
    strcat( testpath, name );

    if(strcmp(path, testpath) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 1;
        return 1;
    }
    
    strcat( testpath, "/" );

    if(strncmp(path, testpath, strlen( testpath )) == 0) {
        if( strstr(&path[10], "/" )) {
            stbuf->st_mode = S_IFREG | 0755;
            stbuf->st_nlink = 1;
        } else {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 1;
        }
        return 1;
    }
    return 0;
}

static int lfmfs_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));

    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return res;
    }
    else if( getAttr( "Artists", path, stbuf )) {
        return res;
    }
    else if( getAttr( "Tags", path, stbuf )) {
        return res;
    }
    
    return -ENOENT;
}

static void filler_toplevel( void* buf, fuse_fill_dir_t filler )
{
    filler( buf, "Artists", NULL, 0 );
    filler( buf, "Tags", NULL, 0 );
}

static void filler_tags( void* buf, fuse_fill_dir_t filler )
{
    struct ArtistList* list = request( "http://ws.audioscrobbler.com/2.0/?method=tag.getTopTags&api_key=b25b959554ed76058ac220b7b2e0a026" );

    int i;
    for( i = 0; i < 50 && list->data[i]; ++i )
        filler( buf, list->data[i], NULL, 0 );

}

static void filler_artists( void* buf, fuse_fill_dir_t filler )
{
    char url[ 105 + strlen( username ) ];
    
    strcpy( url, "http://ws.audioscrobbler.com/2.0/?method=user.gettopartists&user=" );
    strcat( url, username );
    strcat( url, "&api_key=b25b959554ed76058ac220b7b2e0a026" );
    struct ArtistList* list = request( url );

    int i;
    for( i = 0; i < 50 && list->data[i]; ++i )
        filler( buf, list->data[i], NULL, 0 );

}

static void filler_tracks( char* artist, void* buf, fuse_fill_dir_t filler )
{
    char* pos = 0;
    while( pos = strstr( artist, " " )){
        *pos = '+';
    }

    char url[ 100 + strlen( artist ) ];
    strcpy( url, "http://ws.audioscrobbler.com/2.0/?method=artist.getTopTracks&artist=" );
    strcat( url, artist );
    strcat( url, "&api_key=b25b959554ed76058ac220b7b2e0a026" );
    
    struct ArtistList* list = request( url );

    int i;
    for( i = 0; i < 50 && list->data[i]; ++i )
    {
        char filename[ strlen(list->data[i]) + 5 ];
        strcpy( filename, list->data[i] );
        strcat( filename, ".mp3" );
        filler( buf, filename, NULL, 0 );
    }

}

static void filler_tag_tracks( char* tag, void* buf, fuse_fill_dir_t filler )
{
    char* pos = 0;
    while( pos = strstr( tag, " " )){
        *pos = '+';
    }

    char url[ 100 + strlen( tag ) ];
    strcpy( url, "http://ws.audioscrobbler.com/2.0/?method=tag.getTopTracks&tag=" );
    strcat( url, tag );
    strcat( url, "&api_key=b25b959554ed76058ac220b7b2e0a026" );
    
    printf( "Requesting: %s\n", url );
    struct ArtistList* list = request( url );

    int i;
    for( i = 0; i < 50 && list->data[i]; ++i )
    {
        char filename[ strlen(list->data[i]) + 5 ];
        strcpy( filename, list->data[i] );
        strcat( filename, ".mp3" );
        filler( buf, filename, NULL, 0 );
    }
   
}

static int lfmfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;
    
    if( strcmp(path, "/") == 0 )
    {
        filler_toplevel( buf, filler );
    }
    else if( strcmp( path, "/Artists" ) == 0 )
    {
        filler_artists( buf, filler );
    }
    else if( strncmp( path, "/Artists/", 9) == 0 )
    {
        filler_tracks( &path[9], buf, filler );
    }
    else if( strcmp( path, "/Tags" ) == 0 )
    {
        filler_tags( buf, filler );
    }
    else if( strncmp( path, "/Tags/", 6 ) == 0)
    {
        filler_tag_tracks( &path[6], buf, filler );
    }
    else
    {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    return 0;
}

static int lfmfs_open(const char *path, struct fuse_file_info *fi)
{
    if(strcmp(path, path) != 0)
        return -ENOENT;

    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int lfmfs_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, path) != 0)
        return -ENOENT;

//    len = strlen(str);
//    if (offset < len) {
//        if (offset + size > len)
//            size = len - offset;
//        memcpy(buf, str + offset, size);
//    } else
//        size = 0;
//
    return size;
}

static struct fuse_operations oper = {
    .getattr = lfmfs_getattr,
    .readdir = lfmfs_readdir,
    .open   = lfmfs_open,
    .read   = lfmfs_read,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &oper, NULL);
}
