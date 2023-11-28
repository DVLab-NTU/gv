/****************************************************************************
  FileName     [ reader.h ]
  PackageName  [ sat ]
  Synopsis     [ Proof reader for Minisat1.14_p ]
  Author       [ Jiunru Yang ]
  Copyright    [ Copyleft(c) 2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef _READER_H_
#define _READER_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 1024
#define RDR_EOF ((unsigned)~0)

class Reader{
	int 			fd;
	unsigned char* 	buf;
	unsigned int 	size;
	unsigned int	pos;
	unsigned int	getChar();
	unsigned int	overall_pos;
public:
	Reader():fd(-1)				{};
	void open	( const char* file_name );
	void open	( int file_descriptor );
	void close	();
	void seek	( const unsigned int to );
	unsigned long long get64	();
	~Reader		();
	bool null()					{ return fd == -1; }
	unsigned int Current_Pos()	{ return overall_pos; }
	//unsigned int Current_Pos()	{ return lseek(fd, 0, SEEK_CUR) - (size - pos); }
	bool eof();
};

#endif	// _READER_H_
