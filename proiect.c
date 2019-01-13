#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

int countDir(char *path) {
	int i; int count = 0;
	for(i = 0; path[i] != '\0'; i++) {
		if( path[i] == '/' ) count++; 
	}
	return count;
}

static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	st->st_uid = getuid(); 
	st->st_gid = getgid(); 
	st->st_atime = time( NULL ); 
	st->st_mtime = time( NULL ); 
	
	int count = countDir(path);
	if ( count == 1 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
	else
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024 * 3;
	}
	
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Getting The List of Files of %s\n", path );
	
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 )
	{
			
		char users[50];
		FILE *fp = popen("users", "r");
		fscanf(fp, "%s", users);
		
		char *user = strtok(users, " ");

		while( user != NULL ) {
			filler( buffer, user, NULL, 0 );
			user = strtok(NULL, " ");
		}

	} else {
		filler( buffer, "procs", NULL, 0 );
	} 
	
	return 0;
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Trying to read %s, %u, %u\n", path, offset, size );
	
	char result[size];
	size_t newLen;
	
	if ( countDir(path) == 2 ) {
		char user[50];
		char command[50];
		int i;
		for( i = 1; path[i] != '/'; i++ ) {
			user[i-1] = path[i];
		}
		user[i-1] = '\0';
		strcpy(command, "ps -u ");
		strcat(command, user);
		FILE *fp = popen(command, "r");
		newLen = fread(buffer, sizeof(char), size, fp);
	} else {
	 	return -1;
	}
	return newLen;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
};

int main( int argc, char *argv[] )
{
	fuse_main( argc, argv, &operations, NULL );

	return 0;
}

//Compile gcc proiect.c -o proiect `pkg-config fuse --cflags --libs`
//Run ./proiect [MOUNT_POINT] -f

