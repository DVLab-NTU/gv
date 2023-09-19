/****************************************************************************
  FileName     [ reader.cpp ]
  PackageName  [ sat ]
  Synopsis     [ Proof reader for Minisat1.14_p ]
  Author       [ Jiunru Yang ]
  Copyright    [ Copyleft(c) 2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "reader.h"
#include <stdio.h>

void Reader::open( const char* file_name ){
	fd = ::open( file_name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );
	buf = new unsigned char [BUF_SIZE];
	pos = 0;
	overall_pos = 0;
	size = read( fd, buf, BUF_SIZE );
}

void Reader::open(int filedesc)
{
	fd = filedesc;
	buf = new unsigned char [BUF_SIZE];
	pos = 0;
	overall_pos = 0;
	size = read(fd, buf, BUF_SIZE);
}

void Reader::seek( const unsigned int to ){
	lseek( fd, to, SEEK_SET );
	size = read( fd, buf, BUF_SIZE );
	pos = 0;
	overall_pos = to;
}

unsigned int Reader::getChar(){
	if( pos < size ){
		overall_pos++;
		return static_cast<unsigned int>(buf[pos++]);
	}
	if( size < BUF_SIZE ) return RDR_EOF;
	pos = 1;
	overall_pos++;
	size = read( fd, buf, BUF_SIZE );
	if( size == 0 ) return RDR_EOF;
	return static_cast<unsigned int>(buf[0]);
}

unsigned long long Reader::get64(){
	unsigned int byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7;
	byte0 = getChar();
	if(byte0 == RDR_EOF) return RDR_EOF;
	if (!(byte0 & 0x80))
		return byte0;
	else{
		switch ((byte0 & 0x60) >> 5){
			case 0:
				byte1 = getChar();
				return ((byte0 & 0x1F) << 8) | byte1;
			case 1:
				byte1 = getChar();
				byte2 = getChar();
				return ((byte0 & 0x1F) << 16) | (byte1 << 8) | byte2;
			case 2:
				byte1 = getChar();
				byte2 = getChar();
				byte3 = getChar();
				return ((byte0 & 0x1F) << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
			default:
				byte0 = getChar();
				byte1 = getChar();
				byte2 = getChar();
				byte3 = getChar();
				byte4 = getChar();
				byte5 = getChar();
				byte6 = getChar();
				byte7 = getChar();
				return ((unsigned long long)((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3) << 32)
					|  (unsigned long long)((byte4 << 24) | (byte5 << 16) | (byte6 << 8) | byte7);
		}
	}
}

void Reader::close(){
	if( fd == -1 ) return;
	::close(fd);
	fd = -1;
	delete [] buf;
}

Reader::~Reader(){
	close();
}

bool Reader::eof() {
	if(pos < size) return false;
	if(size < BUF_SIZE) return true;
	pos = 0;
	size = read(fd, buf, BUF_SIZE);
	return size == 0;
}
